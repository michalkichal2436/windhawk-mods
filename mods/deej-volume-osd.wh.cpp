// ==WindhawkMod==
// @id              deej-volume-osd
// @name            Deej Volume OSD (Windows 11 Style)
// @description     Displays a smooth Windows 11 style volume flyout OSD when moving physical sliders from the deej hardware mixer project
// @version         1.0
// @author          michalkichal2436
// @github          https://github.com/michalkichal2436/deej-osd
// @include         explorer.exe
// @compilerOptions -lole32 -loleaut32 -luuid -lgdi32 -ldwmapi -luser32 -lshell32 -lgdiplus -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- WindowPreset: compact_app_icon
  $name: Window Layout Preset
  $description: Choose one of the 4 window presets (select Custom to unlock all layout toggles below)
  $options:
  - compact_app_icon: Compact 1-Line with App Icon (App Icon + Bar + Number)
  - win11_compact: Windows 11 Style Compact 1-Line (App Icon + Bar + Thumb, no Number)
  - two_line: 2-Line Detailed (App Icon & Name + Speaker + Bar + Number)
  - custom: Custom (Configure all layout options manually below)
- ConfigYamlPath: ""
  $name: Path to deej config.yaml (or deej.exe folder)
  $description: Paste the path to config.yaml. If left empty, the mod automatically locates the running deej.exe process and syncs with its config.yaml.
- ShowThresholdPercent: 3
  $name: Wake Threshold (%)
  $description: Minimum volume change (%) required from rest before the OSD window appears (1 to 50%). Once visible, the bar updates smoothly.
- MinPercentStep: 1
  $name: Active Update Step (%)
  $description: Minimum volume change (%) required to refresh the OSD while it is already visible (default 1%)
- WindowPosition: bottom_center
  $name: Screen Position
  $description: Select where the OSD window appears on the screen
  $options:
  - bottom_center: Bottom Center (Windows 11 default)
  - bottom_left: Bottom Left
  - bottom_right: Bottom Right
  - top_center: Top Center
  - top_left: Top Left
  - top_right: Top Right
  - center: Screen Center
  - custom: Custom Coordinates (X and Y below)
- OffsetX: 24
  $name: Horizontal Offset X (px)
  $description: Distance from the left/right screen edge (or exact X coordinate when Screen Position is set to Custom Coordinates)
- OffsetY: 56
  $name: Vertical Offset Y (px)
  $description: Distance from the bottom/top screen edge (or exact Y coordinate when Screen Position is set to Custom Coordinates)
- BarLengthPx: 140
  $name: Volume Bar Length (px)
  $description: Length of the volume progress bar in pixels (window width adjusts automatically, default 140)
- WindowOpacityPercent: 98
  $name: Window Opacity (%)
  $description: Opacity of the OSD window from 10 (very transparent) to 100 (fully opaque)
- DisplayDurationMs: 1500
  $name: Display Duration (ms)
  $description: How long the OSD stays visible after slider movement stops (in milliseconds)
- UseSystemAccentColor: true
  $name: Use Windows 11 System Accent Color
  $description: Automatically matches the volume bar fill color to the active Windows 11 dark-mode accent color
- FontName: Segoe UI
  $name: Font Family
  $description: Font family used for labels and volume numbers (e.g. Segoe UI, Arial, Calibri, Consolas)
- LabelFontSize: 11
  $name: Application Label Font Size (pt)
  $description: Font size for the application or channel name in 2-line and Custom modes
- NumberFontSize: 13
  $name: Volume Number Font Size (pt)
  $description: Font size for the 0-100 volume percentage number on the right side of the bar
- IconSizePx: 20
  $name: Application Icon Size (px)
  $description: Size of the captured or custom application icon in pixels (from 12 to 48)
- CustomHeaderMode: two_line
  $name: "[Custom Preset] Base Row Layout"
  $description: Used when Window Layout Preset is set to Custom
  $options:
  - two_line: 2-Line (Header Row + Volume Bar Row)
  - single_speaker: 1-Line with Speaker Icon on the left
  - single_app_icon: 1-Line with Application Icon on the left
- TextHorizontalAlign: center
  $name: "[Custom Preset] Header Text Horizontal Alignment"
  $description: Horizontal alignment of the application icon and label in 2-Line / Custom mode
  $options:
  - left: Left
  - center: Center
  - right: Right
- TextVerticalPos: top
  $name: "[Custom Preset] Header Text Vertical Position"
  $description: Position of the application label relative to the volume bar in 2-Line / Custom mode
  $options:
  - top: Top (Above the volume bar)
  - bottom: Bottom (Below the volume bar)
- ShowVolumeNumber: true
  $name: "[Custom Preset] Show Volume Number (0-100)"
  $description: Displays the numeric volume percentage on the right side of the bar in Custom mode
- ShowAppIcon: true
  $name: "[Custom Preset] Show Application Icon"
  $description: Automatically captures the active process icon or uses a custom icon path below
- HideSliderNumber: true
  $name: Hide Slider Index Prefix (Slider X)
  $description: When enabled, displays only the application/channel name (e.g. Brave instead of Slider 1 - Brave)
- ShowAppNameInUnmapped: true
  $name: Show Active App Name for deej.unmapped
  $description: Displays the specific process name (e.g. Spotify) instead of a generic Unmapped label
- ShowBarThumb: false
  $name: Show Circular Thumb on Volume Bar
  $description: Draws a circular indicator on the progress bar (disabled by default to match the flat Windows 11 Fn+F2 bar)
- CustomIconMaster: ""
  $name: Custom Icon Path - Slider 0
  $description: Optional path to a .png, .ico, .bmp, or .exe file for Slider 0
- CustomIconSlider1: ""
  $name: Custom Icon Path - Slider 1
  $description: Optional path to a .png, .ico, .bmp, or .exe file for Slider 1
- CustomIconSlider2: ""
  $name: Custom Icon Path - Slider 2
  $description: Optional path to a .png, .ico, .bmp, or .exe file for Slider 2
- CustomIconSlider3: ""
  $name: Custom Icon Path - Slider 3
  $description: Optional path to a .png, .ico, .bmp, or .exe file for Slider 3
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <gdiplus.h>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cwctype>

#define WM_OSD_SHOW_VOLUME     (WM_APP + 1)
#define WM_OSD_SESSION_CREATED (WM_APP + 2)
#define WM_OSD_DEVICE_CHANGED  (WM_APP + 3)
#define WM_OSD_RELOAD_SETTINGS (WM_APP + 4)

#define TIMER_HIDE_OSD         1
#define TIMER_FADE_OSD         2
#define TIMER_CONFIG_WATCH     3

enum class PresetMode {
    CompactAppIcon,
    Win11Compact,
    TwoLine,
    Custom
};

enum class CustomRowLayout {
    TwoLine,
    SingleSpeaker,
    SingleAppIcon
};

enum class HAlign { Left, Center, Right };
enum class VPos { Top, Bottom };
enum class WinPos {
    BottomCenter, BottomLeft, BottomRight,
    TopCenter, TopLeft, TopRight,
    Center, Custom
};

struct ModSettings {
    PresetMode windowPreset = PresetMode::CompactAppIcon;
    std::wstring configYamlPath;
    int showThresholdPercent = 3;
    int minPercentStep = 1;
    WinPos windowPosition = WinPos::BottomCenter;
    int offsetX = 24;
    int offsetY = 56;
    int barLengthPx = 140;
    int windowOpacityPercent = 98;
    int displayDurationMs = 1500;
    bool useSystemAccentColor = true;
    std::wstring fontName = L"Segoe UI";
    int labelFontSize = 11;
    int numberFontSize = 13;
    int iconSizePx = 20;
    CustomRowLayout customHeaderMode = CustomRowLayout::TwoLine;
    HAlign textHAlign = HAlign::Center;
    VPos textVPos = VPos::Top;
    bool showVolumeNumber = true;
    bool showAppIcon = true;
    bool hideSliderNumber = true;
    bool showAppNameInUnmapped = true;
    bool showBarThumb = false;
    std::wstring customIconMaster;
    std::wstring customIconSlider1;
    std::wstring customIconSlider2;
    std::wstring customIconSlider3;
} g_settings;

// Effective layout state resolved from the active WindowPreset
struct EffectiveLayout {
    bool isTwoLine = false;
    bool leftUsesAppIcon = true;
    bool showHeaderAppIcon = true;
    bool showVolumeNumber = true;
    bool hideSliderNumber = true;
    bool showAppNameInUnmapped = true;
    bool showBarThumb = false;
    HAlign headerAlign = HAlign::Center;
    VPos headerVPos = VPos::Top;
};

static EffectiveLayout GetEffectiveLayout() {
    EffectiveLayout el;

    // Settings shared by presets unless a preset explicitly overrides them.
    el.hideSliderNumber = g_settings.hideSliderNumber;
    el.showAppNameInUnmapped = g_settings.showAppNameInUnmapped;
    el.showBarThumb = g_settings.showBarThumb;

    switch (g_settings.windowPreset) {
        case PresetMode::CompactAppIcon:
            el.isTwoLine = false;
            el.leftUsesAppIcon = true;
            el.showHeaderAppIcon = false;
            el.showVolumeNumber = true;
            break;

        case PresetMode::Win11Compact:
            // Fixed Windows 11 preset, matching the requested options:
            // 1-line + application icon, left/top alignment, no numeric value,
            // slider prefix visible, active app name for deej.unmapped, thumb enabled.
            el.isTwoLine = false;
            el.leftUsesAppIcon = true;
            el.showHeaderAppIcon = false;
            el.showVolumeNumber = false;
            el.hideSliderNumber = false;
            el.showAppNameInUnmapped = true;
            el.showBarThumb = true;
            el.headerAlign = HAlign::Left;
            el.headerVPos = VPos::Top;
            break;

        case PresetMode::TwoLine:
            el.isTwoLine = true;
            el.leftUsesAppIcon = false;
            el.showHeaderAppIcon = true;
            el.showVolumeNumber = true;
            el.headerAlign = g_settings.textHAlign;
            el.headerVPos = g_settings.textVPos;
            break;

        case PresetMode::Custom:
            el.isTwoLine = (g_settings.customHeaderMode == CustomRowLayout::TwoLine);
            el.leftUsesAppIcon = (g_settings.customHeaderMode == CustomRowLayout::SingleAppIcon);
            el.showHeaderAppIcon = g_settings.showAppIcon;
            el.showVolumeNumber = g_settings.showVolumeNumber;
            el.headerAlign = g_settings.textHAlign;
            el.headerVPos = g_settings.textVPos;
            break;
    }
    return el;
}

// Synchronized deej config.yaml state
struct DeejConfig {
    std::map<int, std::vector<std::wstring>> sliderMapping;
    std::wstring resolvedPath;
    FILETIME lastWriteTime = {};
    bool loaded = false;
} g_deejConfig;

struct VolumeEventPayload {
    int sliderIndex;
    std::wstring targetKeyLower;
    std::wstring displayAppName;
    std::wstring processFullPath;
    int volumePercent;
    bool muted;
};

static HANDLE g_hThread = nullptr;
static HWND g_hOsdWnd = nullptr;
static HANDLE g_hInitEvent = nullptr;
static volatile bool g_bShuttingDown = false;
static volatile bool g_bOsdActive = false;
static volatile int g_activeSliderIndex = -1;
static ULONG_PTR g_gdiplusToken = 0;

static std::wstring g_currentLabel = L"Master";
static int g_currentVolume = 50;
static bool g_currentMuted = false;
static BYTE g_currentAlpha = 0;
static Gdiplus::Bitmap* g_currentAppBitmap = nullptr;

static std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](wchar_t c) { return std::towlower(c); });
    return s;
}

static std::string TrimAscii(std::string s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r' || s.front() == '\n' || s.front() == '\"' || s.front() == '\'')) {
        s.erase(s.begin());
    }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n' || s.back() == '\"' || s.back() == '\'')) {
        s.pop_back();
    }
    return s;
}

static std::wstring TrimQuotesW(std::wstring s) {
    while (!s.empty() && (s.front() == L' ' || s.front() == L'\t' || s.front() == L'\"' || s.front() == L'\'')) {
        s.erase(s.begin());
    }
    while (!s.empty() && (s.back() == L' ' || s.back() == L'\t' || s.back() == L'\"' || s.back() == L'\'')) {
        s.pop_back();
    }
    return s;
}

static std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    if (size <= 0) return L"";
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size);
    return result;
}

static bool GetProcessPathsByPid(DWORD pid, std::wstring& outFileNameLower, std::wstring& outFullPath) {
    outFileNameLower.clear();
    outFullPath.clear();
    if (pid == 0) return false;

    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) return false;

    wchar_t buf[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    bool ok = false;
    if (QueryFullProcessImageNameW(hProc, 0, buf, &size)) {
        outFullPath = buf;
        size_t pos = outFullPath.find_last_of(L"\\/");
        std::wstring name = (pos != std::wstring::npos) ? outFullPath.substr(pos + 1) : outFullPath;
        outFileNameLower = ToLower(name);
        ok = true;
    }
    CloseHandle(hProc);
    return ok;
}

static std::wstring ResolveConfigYamlPath() {
    std::wstring path = TrimQuotesW(g_settings.configYamlPath);

    if (!path.empty()) {
        DWORD attr = GetFileAttributesW(path.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
            if (path.back() != L'\\' && path.back() != L'/') path += L'\\';
            path += L"config.yaml";
            return path;
        }
        std::wstring lower = ToLower(path);
        if (lower.size() > 8 && lower.substr(lower.size() - 8) == L"deej.exe") {
            size_t pos = path.find_last_of(L"\\/");
            if (pos != std::wstring::npos) {
                return path.substr(0, pos + 1) + L"config.yaml";
            }
        }
        return path;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"deej.exe") == 0) {
                    std::wstring exeName, fullPath;
                    if (GetProcessPathsByPid(pe.th32ProcessID, exeName, fullPath)) {
                        size_t pos = fullPath.find_last_of(L"\\/");
                        if (pos != std::wstring::npos) {
                            CloseHandle(hSnap);
                            return fullPath.substr(0, pos + 1) + L"config.yaml";
                        }
                    }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return L"";
}

static void SetDefaultDeejMapping() {
    g_deejConfig.sliderMapping.clear();
    g_deejConfig.sliderMapping[0] = { L"master" };
    g_deejConfig.sliderMapping[1] = { L"brave.exe" };
    g_deejConfig.sliderMapping[2] = { L"deej.unmapped" };
    g_deejConfig.sliderMapping[3] = { L"discord.exe" };
}

static bool SyncWithDeejConfigYaml(bool forceReload = false) {
    std::wstring yamlPath = ResolveConfigYamlPath();
    if (yamlPath.empty()) {
        if (!g_deejConfig.loaded) {
            SetDefaultDeejMapping();
            g_deejConfig.loaded = true;
        }
        return false;
    }

    WIN32_FILE_ATTRIBUTE_DATA fad = {};
    if (!GetFileAttributesExW(yamlPath.c_str(), GetFileExInfoStandard, &fad)) {
        if (!g_deejConfig.loaded) {
            SetDefaultDeejMapping();
            g_deejConfig.loaded = true;
        }
        return false;
    }

    if (!forceReload && g_deejConfig.loaded && g_deejConfig.resolvedPath == yamlPath &&
        CompareFileTime(&fad.ftLastWriteTime, &g_deejConfig.lastWriteTime) == 0) {
        return false;
    }

    std::ifstream file(yamlPath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::map<int, std::vector<std::wstring>> newMapping;
    bool inSliderMapping = false;
    int currentSliderIdx = -1;

    std::string line;
    while (std::getline(file, line)) {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        std::string trimmed = TrimAscii(line);
        if (trimmed.empty()) continue;

        size_t indent = line.find_first_not_of(" \t");
        if (indent == std::string::npos) indent = 0;

        if (indent == 0) {
            if (trimmed.rfind("slider_mapping:", 0) == 0) {
                inSliderMapping = true;
                currentSliderIdx = -1;
            } else {
                inSliderMapping = false;
                currentSliderIdx = -1;
            }
            continue;
        }

        if (inSliderMapping) {
            if (trimmed[0] == '-') {
                if (currentSliderIdx >= 0) {
                    std::string item = TrimAscii(trimmed.substr(1));
                    if (!item.empty()) {
                        newMapping[currentSliderIdx].push_back(ToLower(Utf8ToWide(item)));
                    }
                }
                continue;
            }

            size_t colonPos = trimmed.find(':');
            if (colonPos != std::string::npos) {
                std::string keyStr = TrimAscii(trimmed.substr(0, colonPos));
                std::string valStr = TrimAscii(trimmed.substr(colonPos + 1));

                try {
                    currentSliderIdx = std::stoi(keyStr);
                } catch (...) {
                    currentSliderIdx = -1;
                    continue;
                }

                if (!valStr.empty()) {
                    if (valStr.front() == '[' && valStr.back() == ']') {
                        std::string inner = valStr.substr(1, valStr.size() - 2);
                        std::stringstream ss(inner);
                        std::string token;
                        while (std::getline(ss, token, ',')) {
                            std::string cleanTok = TrimAscii(token);
                            if (!cleanTok.empty()) {
                                newMapping[currentSliderIdx].push_back(ToLower(Utf8ToWide(cleanTok)));
                            }
                        }
                    } else {
                        newMapping[currentSliderIdx].push_back(ToLower(Utf8ToWide(valStr)));
                    }
                }
            }
        }
    }

    if (!newMapping.empty()) {
        g_deejConfig.sliderMapping = std::move(newMapping);
        g_deejConfig.resolvedPath = yamlPath;
        g_deejConfig.lastWriteTime = fad.ftLastWriteTime;
        g_deejConfig.loaded = true;
        return true;
    }

    return false;
}

static int FindSliderForTarget(const std::wstring& procNameLower, DWORD pid) {
    for (const auto& kv : g_deejConfig.sliderMapping) {
        for (const auto& target : kv.second) {
            if (target == procNameLower) {
                return kv.first;
            }
        }
    }

    if (pid != 0) {
        HWND hFg = GetForegroundWindow();
        if (hFg) {
            DWORD fgPid = 0;
            GetWindowThreadProcessId(hFg, &fgPid);
            if (fgPid == pid) {
                for (const auto& kv : g_deejConfig.sliderMapping) {
                    for (const auto& target : kv.second) {
                        if (target == L"deej.current") {
                            return kv.first;
                        }
                    }
                }
            }
        }
    }

    if (procNameLower != L"master" && procNameLower != L"mic" && procNameLower != L"system") {
        for (const auto& kv : g_deejConfig.sliderMapping) {
            for (const auto& target : kv.second) {
                if (target == L"deej.unmapped") {
                    return kv.first;
                }
            }
        }
    }

    return -1;
}

static std::wstring PrettifyAppName(const std::wstring& rawLower) {
    if (rawLower == L"master") return L"Master";
    if (rawLower == L"mic") return L"Microphone";
    if (rawLower == L"system") return L"System Sounds";
    if (rawLower.empty()) return L"Application";

    std::wstring clean = rawLower;
    if (clean.size() > 4 && clean.substr(clean.size() - 4) == L".exe") {
        clean = clean.substr(0, clean.size() - 4);
    }
    if (!clean.empty()) {
        clean[0] = std::towupper(clean[0]);
    }
    return clean;
}

static std::wstring FormatSliderLabel(int sliderIndex, const std::wstring& procNameLower) {
    EffectiveLayout el = GetEffectiveLayout();

    std::wstring prefix;
    if (!el.hideSliderNumber && sliderIndex >= 0) {
        prefix = L"Slider " + std::to_wstring(sliderIndex) + L" \u2022 ";
    }

    bool isUnmappedSlider = false;
    auto it = g_deejConfig.sliderMapping.find(sliderIndex);
    if (it != g_deejConfig.sliderMapping.end()) {
        for (const auto& t : it->second) {
            if (t == L"deej.unmapped") {
                isUnmappedSlider = true;
                break;
            }
        }
    }

    if (isUnmappedSlider && !el.showAppNameInUnmapped) {
        return prefix + L"Unmapped";
    }

    return prefix + PrettifyAppName(procNameLower);
}

static Gdiplus::Bitmap* CreateGdiplusBitmapFromHIcon(HICON hIcon) {
    if (!hIcon) return nullptr;

    ICONINFO ii = {};
    if (!GetIconInfo(hIcon, &ii)) return nullptr;

    BITMAP bm = {};
    if (!GetObjectW(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(bm), &bm)) {
        if (ii.hbmColor) DeleteObject(ii.hbmColor);
        if (ii.hbmMask) DeleteObject(ii.hbmMask);
        return nullptr;
    }

    int width = bm.bmWidth;
    int height = ii.hbmColor ? bm.bmHeight : (bm.bmHeight / 2);
    if (width <= 0 || height <= 0) {
        if (ii.hbmColor) DeleteObject(ii.hbmColor);
        if (ii.hbmMask) DeleteObject(ii.hbmMask);
        return nullptr;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<BYTE> pixels(width * height * 4, 0);
    HDC hdc = GetDC(nullptr);
    bool hasAlpha = false;

    if (ii.hbmColor) {
        GetDIBits(hdc, ii.hbmColor, 0, height, pixels.data(), &bmi, DIB_RGB_COLORS);
        for (int i = 0; i < width * height; ++i) {
            if (pixels[i * 4 + 3] != 0) {
                hasAlpha = true;
                break;
            }
        }
        if (!hasAlpha && ii.hbmMask) {
            std::vector<BYTE> maskPixels(width * height * 4, 0);
            GetDIBits(hdc, ii.hbmMask, 0, height, maskPixels.data(), &bmi, DIB_RGB_COLORS);
            for (int i = 0; i < width * height; ++i) {
                pixels[i * 4 + 3] = (maskPixels[i * 4] == 0) ? 255 : 0;
            }
        }
    }
    ReleaseDC(nullptr, hdc);
    if (ii.hbmColor) DeleteObject(ii.hbmColor);
    if (ii.hbmMask) DeleteObject(ii.hbmMask);

    auto* bmp = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
    Gdiplus::BitmapData bd = {};
    Gdiplus::Rect rect(0, 0, width, height);
    if (bmp->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bd) == Gdiplus::Ok) {
        for (int y = 0; y < height; ++y) {
            memcpy(static_cast<BYTE*>(bd.Scan0) + y * bd.Stride, pixels.data() + y * width * 4, width * 4);
        }
        bmp->UnlockBits(&bd);
    }
    return bmp;
}

static Gdiplus::Bitmap* LoadHighResBitmapForEvent(int sliderIndex, const std::wstring& processFullPath) {
    EffectiveLayout el = GetEffectiveLayout();
    if (!el.leftUsesAppIcon && !el.showHeaderAppIcon) {
        return nullptr;
    }

    std::wstring customPath;
    switch (sliderIndex) {
        case 0: customPath = g_settings.customIconMaster; break;
        case 1: customPath = g_settings.customIconSlider1; break;
        case 2: customPath = g_settings.customIconSlider2; break;
        case 3: customPath = g_settings.customIconSlider3; break;
    }
    customPath = TrimQuotesW(customPath);

    if (!customPath.empty()) {
        auto* directBmp = Gdiplus::Bitmap::FromFile(customPath.c_str());
        if (directBmp && directBmp->GetLastStatus() == Gdiplus::Ok && directBmp->GetWidth() > 0) {
            return directBmp;
        }
        delete directBmp;

        HICON hCustomIcon = nullptr;
        if (SUCCEEDED(SHDefExtractIconW(customPath.c_str(), 0, 0, &hCustomIcon, nullptr, MAKELONG(64, 16))) && hCustomIcon) {
            auto* iconBmp = CreateGdiplusBitmapFromHIcon(hCustomIcon);
            DestroyIcon(hCustomIcon);
            if (iconBmp) return iconBmp;
        }
    }

    if (!processFullPath.empty()) {
        HICON hExeIcon = nullptr;
        if (SUCCEEDED(SHDefExtractIconW(processFullPath.c_str(), 0, 0, &hExeIcon, nullptr, MAKELONG(64, 16))) && hExeIcon) {
            auto* iconBmp = CreateGdiplusBitmapFromHIcon(hExeIcon);
            DestroyIcon(hExeIcon);
            if (iconBmp) return iconBmp;
        }

        SHFILEINFOW sfi = {};
        if (SHGetFileInfoW(processFullPath.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON) && sfi.hIcon) {
            auto* iconBmp = CreateGdiplusBitmapFromHIcon(sfi.hIcon);
            DestroyIcon(sfi.hIcon);
            if (iconBmp) return iconBmp;
        }
    }

    return nullptr;
}

// Reliable GDI+ Font builder with automatic fallback when variable font names are not recognized by GDI+
static Gdiplus::Font* CreateFontWithFallback(const std::wstring& requestedFamily, float sizePt, INT style = Gdiplus::FontStyleRegular) {
    Gdiplus::FontFamily primary(requestedFamily.c_str());
    if (primary.IsAvailable()) {
        return new Gdiplus::Font(&primary, sizePt, style, Gdiplus::UnitPoint);
    }

    Gdiplus::FontFamily segoe(L"Segoe UI");
    if (segoe.IsAvailable()) {
        return new Gdiplus::Font(&segoe, sizePt, style, Gdiplus::UnitPoint);
    }

    return new Gdiplus::Font(Gdiplus::FontFamily::GenericSansSerif(), sizePt, style, Gdiplus::UnitPoint);
}

static Gdiplus::Color GetWindows11AccentColor() {
    if (g_settings.useSystemAccentColor) {
        BYTE palette[32] = {};
        DWORD size = sizeof(palette);
        if (RegGetValueW(HKEY_CURRENT_USER,
                         L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent",
                         L"AccentPalette", RRF_RT_REG_BINARY, nullptr, palette, &size) == ERROR_SUCCESS && size >= 16) {
            return Gdiplus::Color(255, palette[4], palette[5], palette[6]);
        }
    }
    return Gdiplus::Color(255, 96, 205, 255);
}

static bool IsHardwareVolumeKeyPressed() {
    return (GetAsyncKeyState(VK_VOLUME_DOWN) & 0x8000) ||
           (GetAsyncKeyState(VK_VOLUME_UP) & 0x8000) ||
           (GetAsyncKeyState(VK_VOLUME_MUTE) & 0x8000);
}

static bool ShouldTriggerOsd(int sliderIdx, int newPct, bool newMuted, int& ioAnchorPct, bool& ioLastMuted) {
    if (sliderIdx < 0) return false;

    if (ioAnchorPct == -1) {
        ioAnchorPct = newPct;
        ioLastMuted = newMuted;
        return false;
    }

    bool muteChanged = (newMuted != ioLastMuted);
    int diff = std::abs(newPct - ioAnchorPct);

    int requiredDelta = (g_bOsdActive && g_activeSliderIndex == sliderIdx)
                            ? g_settings.minPercentStep
                            : g_settings.showThresholdPercent;

    if (!muteChanged && diff < requiredDelta) {
        return false;
    }

    ioAnchorPct = newPct;
    ioLastMuted = newMuted;
    return true;
}

class CEndpointVolumeCallback : public IAudioEndpointVolumeCallback {
    LONG m_refCount = 1;
    std::wstring m_channelKey;
    int m_anchorPercent = -1;
    bool m_lastMuted = false;

public:
    CEndpointVolumeCallback(const std::wstring& channelKey, int initialPercent, bool initialMuted)
        : m_channelKey(channelKey), m_anchorPercent(initialPercent), m_lastMuted(initialMuted) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == __uuidof(IAudioEndpointVolumeCallback)) {
            *ppv = static_cast<IAudioEndpointVolumeCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override {
        if (!pNotify || g_bShuttingDown || !g_hOsdWnd) return S_OK;

        int newPct = static_cast<int>(std::lround(pNotify->fMasterVolume * 100.0f));
        bool newMuted = (pNotify->bMuted != FALSE);

        if (m_channelKey == L"master" && IsHardwareVolumeKeyPressed()) {
            m_anchorPercent = newPct;
            m_lastMuted = newMuted;
            return S_OK;
        }

        int sliderIdx = FindSliderForTarget(m_channelKey, 0);
        if (!ShouldTriggerOsd(sliderIdx, newPct, newMuted, m_anchorPercent, m_lastMuted)) {
            return S_OK;
        }

        auto* payload = new VolumeEventPayload{
            sliderIdx,
            m_channelKey,
            PrettifyAppName(m_channelKey),
            L"",
            newPct,
            newMuted
        };
        if (!PostMessageW(g_hOsdWnd, WM_OSD_SHOW_VOLUME, 0, reinterpret_cast<LPARAM>(payload))) {
            delete payload;
        }
        return S_OK;
    }
};

class CSessionEvents : public IAudioSessionEvents {
    LONG m_refCount = 1;
    DWORD m_pid = 0;
    std::wstring m_procName;
    std::wstring m_procFullPath;
    int m_anchorPercent = -1;
    bool m_lastMuted = false;

public:
    CSessionEvents(DWORD pid, const std::wstring& procName, const std::wstring& procFullPath, int initialPct, bool initialMuted)
        : m_pid(pid), m_procName(procName), m_procFullPath(procFullPath), m_anchorPercent(initialPct), m_lastMuted(initialMuted) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == __uuidof(IAudioSessionEvents)) {
            *ppv = static_cast<IAudioSessionEvents*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    STDMETHODIMP OnDisplayNameChanged(LPCWSTR, LPCGUID) override { return S_OK; }
    STDMETHODIMP OnIconPathChanged(LPCWSTR, LPCGUID) override { return S_OK; }

    STDMETHODIMP OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID) override {
        if (g_bShuttingDown || !g_hOsdWnd) return S_OK;
        if (m_procName.empty()) return S_OK;

        int newPct = static_cast<int>(std::lround(NewVolume * 100.0f));
        bool newMuted = (NewMute != FALSE);
        int sliderIdx = FindSliderForTarget(m_procName, m_pid);

        if (!ShouldTriggerOsd(sliderIdx, newPct, newMuted, m_anchorPercent, m_lastMuted)) {
            return S_OK;
        }

        auto* payload = new VolumeEventPayload{
            sliderIdx,
            m_procName,
            PrettifyAppName(m_procName),
            m_procFullPath,
            newPct,
            newMuted
        };
        if (!PostMessageW(g_hOsdWnd, WM_OSD_SHOW_VOLUME, 0, reinterpret_cast<LPARAM>(payload))) {
            delete payload;
        }
        return S_OK;
    }

    STDMETHODIMP OnChannelVolumeChanged(DWORD, float[], DWORD, LPCGUID) override { return S_OK; }
    STDMETHODIMP OnGroupingParamChanged(LPCGUID, LPCGUID) override { return S_OK; }
    STDMETHODIMP OnStateChanged(AudioSessionState) override { return S_OK; }
    STDMETHODIMP OnSessionDisconnected(AudioSessionDisconnectReason) override { return S_OK; }
};

class CSessionNotification : public IAudioSessionNotification {
    LONG m_refCount = 1;
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == __uuidof(IAudioSessionNotification)) {
            *ppv = static_cast<IAudioSessionNotification*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    STDMETHODIMP OnSessionCreated(IAudioSessionControl* pNewSession) override {
        if (!pNewSession || g_bShuttingDown || !g_hOsdWnd) return S_OK;
        pNewSession->AddRef();
        if (!PostMessageW(g_hOsdWnd, WM_OSD_SESSION_CREATED, 0, reinterpret_cast<LPARAM>(pNewSession))) {
            pNewSession->Release();
        }
        return S_OK;
    }
};

class CDeviceNotificationClient : public IMMNotificationClient {
    LONG m_refCount = 1;
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == __uuidof(IMMNotificationClient)) {
            *ppv = static_cast<IMMNotificationClient*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    STDMETHODIMP OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
    STDMETHODIMP OnDeviceAdded(LPCWSTR) override { return S_OK; }
    STDMETHODIMP OnDeviceRemoved(LPCWSTR) override { return S_OK; }
    STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override {
        if ((flow == eRender || flow == eCapture) && role == eConsole && g_hOsdWnd && !g_bShuttingDown) {
            PostMessageW(g_hOsdWnd, WM_OSD_DEVICE_CHANGED, 0, 0);
        }
        return S_OK;
    }
    STDMETHODIMP OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }
};

struct RegisteredSession {
    IAudioSessionControl* control = nullptr;
    CSessionEvents* events = nullptr;
};

static IMMDeviceEnumerator* g_pEnumerator = nullptr;
static CDeviceNotificationClient* g_pDeviceClient = nullptr;
static IMMDevice* g_pRenderDevice = nullptr;
static IAudioEndpointVolume* g_pMasterVolume = nullptr;
static CEndpointVolumeCallback* g_pMasterCallback = nullptr;
static IMMDevice* g_pCaptureDevice = nullptr;
static IAudioEndpointVolume* g_pMicVolume = nullptr;
static CEndpointVolumeCallback* g_pMicCallback = nullptr;
static IAudioSessionManager2* g_pSessionManager = nullptr;
static CSessionNotification* g_pSessionNotification = nullptr;
static std::vector<RegisteredSession> g_sessions;

static void AttachSessionControl(IAudioSessionControl* pControl) {
    if (!pControl) return;

    IAudioSessionControl2* pControl2 = nullptr;
    if (FAILED(pControl->QueryInterface(__uuidof(IAudioSessionControl2), reinterpret_cast<void**>(&pControl2)))) {
        return;
    }

    DWORD pid = 0;
    pControl2->GetProcessId(&pid);
    bool isSystemSounds = (pControl2->IsSystemSoundsSession() == S_OK);
    pControl2->Release();

    for (const auto& s : g_sessions) {
        if (s.control == pControl) return;
    }

    int initPct = -1;
    bool initMuted = false;
    ISimpleAudioVolume* pSimpleVol = nullptr;
    if (SUCCEEDED(pControl->QueryInterface(__uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&pSimpleVol)))) {
        float vol = 0.0f;
        BOOL muted = FALSE;
        if (SUCCEEDED(pSimpleVol->GetMasterVolume(&vol))) {
            initPct = static_cast<int>(std::lround(vol * 100.0f));
        }
        if (SUCCEEDED(pSimpleVol->GetMute(&muted))) {
            initMuted = (muted != FALSE);
        }
        pSimpleVol->Release();
    }

    std::wstring procName, procFullPath;
    if (isSystemSounds) {
        procName = L"system";
    } else if (!GetProcessPathsByPid(pid, procName, procFullPath) || procName.empty()) {
        return;
    }

    auto* pEvents = new CSessionEvents(pid, procName, procFullPath, initPct, initMuted);
    if (SUCCEEDED(pControl->RegisterAudioSessionNotification(pEvents))) {
        pControl->AddRef();
        g_sessions.push_back({pControl, pEvents});
    } else {
        pEvents->Release();
    }
}

static void CleanupAudioBindings() {
    for (auto& s : g_sessions) {
        if (s.control && s.events) {
            s.control->UnregisterAudioSessionNotification(s.events);
        }
        if (s.events) s.events->Release();
        if (s.control) s.control->Release();
    }
    g_sessions.clear();

    if (g_pSessionManager && g_pSessionNotification) {
        g_pSessionManager->UnregisterSessionNotification(g_pSessionNotification);
    }
    if (g_pSessionNotification) {
        g_pSessionNotification->Release();
        g_pSessionNotification = nullptr;
    }
    if (g_pSessionManager) {
        g_pSessionManager->Release();
        g_pSessionManager = nullptr;
    }

    if (g_pMasterVolume && g_pMasterCallback) {
        g_pMasterVolume->UnregisterControlChangeNotify(g_pMasterCallback);
    }
    if (g_pMasterCallback) {
        g_pMasterCallback->Release();
        g_pMasterCallback = nullptr;
    }
    if (g_pMasterVolume) {
        g_pMasterVolume->Release();
        g_pMasterVolume = nullptr;
    }
    if (g_pRenderDevice) {
        g_pRenderDevice->Release();
        g_pRenderDevice = nullptr;
    }

    if (g_pMicVolume && g_pMicCallback) {
        g_pMicVolume->UnregisterControlChangeNotify(g_pMicCallback);
    }
    if (g_pMicCallback) {
        g_pMicCallback->Release();
        g_pMicCallback = nullptr;
    }
    if (g_pMicVolume) {
        g_pMicVolume->Release();
        g_pMicVolume = nullptr;
    }
    if (g_pCaptureDevice) {
        g_pCaptureDevice->Release();
        g_pCaptureDevice = nullptr;
    }
}

static void InitAudioBindings() {
    CleanupAudioBindings();
    if (!g_pEnumerator) return;

    if (SUCCEEDED(g_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &g_pRenderDevice))) {
        if (SUCCEEDED(g_pRenderDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                                reinterpret_cast<void**>(&g_pMasterVolume)))) {
            float initVol = 0.0f;
            BOOL initMute = FALSE;
            g_pMasterVolume->GetMasterVolumeLevelScalar(&initVol);
            g_pMasterVolume->GetMute(&initMute);
            int initPct = static_cast<int>(std::lround(initVol * 100.0f));

            g_pMasterCallback = new CEndpointVolumeCallback(L"master", initPct, initMute != FALSE);
            g_pMasterVolume->RegisterControlChangeNotify(g_pMasterCallback);
        }

        if (SUCCEEDED(g_pRenderDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                                                reinterpret_cast<void**>(&g_pSessionManager)))) {
            g_pSessionNotification = new CSessionNotification();
            g_pSessionManager->RegisterSessionNotification(g_pSessionNotification);

            IAudioSessionEnumerator* pEnum = nullptr;
            if (SUCCEEDED(g_pSessionManager->GetSessionEnumerator(&pEnum))) {
                int count = 0;
                pEnum->GetCount(&count);
                for (int i = 0; i < count; ++i) {
                    IAudioSessionControl* pControl = nullptr;
                    if (SUCCEEDED(pEnum->GetSession(i, &pControl))) {
                        AttachSessionControl(pControl);
                        pControl->Release();
                    }
                }
                pEnum->Release();
            }
        }
    }

    if (SUCCEEDED(g_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &g_pCaptureDevice))) {
        if (SUCCEEDED(g_pCaptureDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                                 reinterpret_cast<void**>(&g_pMicVolume)))) {
            float initVol = 0.0f;
            BOOL initMute = FALSE;
            g_pMicVolume->GetMasterVolumeLevelScalar(&initVol);
            g_pMicVolume->GetMute(&initMute);
            int initPct = static_cast<int>(std::lround(initVol * 100.0f));

            g_pMicCallback = new CEndpointVolumeCallback(L"mic", initPct, initMute != FALSE);
            g_pMicVolume->RegisterControlChangeNotify(g_pMicCallback);
        }
    }
}

static void FillSmoothPill(Gdiplus::Graphics& g, float x, float y, float w, float h, const Gdiplus::Color& color) {
    if (w <= 0.5f || h <= 0.5f) return;
    Gdiplus::SolidBrush brush(color);
    if (w <= h) {
        g.FillEllipse(&brush, x, y, h, h);
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddArc(x, y, h, h, 90.0f, 180.0f);
    path.AddArc(x + w - h, y, h, h, 270.0f, 180.0f);
    path.CloseFigure();
    g.FillPath(&brush, &path);
}

static void CalculateWindowSize(UINT dpi, int& outWidth, int& outHeight) {
    EffectiveLayout el = GetEffectiveLayout();
    float scale = dpi / 96.0f;
    auto S = [scale](int px) -> int { return static_cast<int>(std::lround(px * scale)); };

    int leftIconSpan = el.leftUsesAppIcon ? std::max(20, g_settings.iconSizePx) : 20;
    int leftMargin = S(16 + leftIconSpan + 12);
    int rightMargin = el.showVolumeNumber
                          ? S(52 + std::max(0, (g_settings.numberFontSize - 13) * 3))
                          : S(18);
    int barLen = S(std::clamp(g_settings.barLengthPx, 60, 1200));

    outWidth = leftMargin + barLen + rightMargin;

    if (el.isTwoLine) {
        int labelRowH = S(std::max(g_settings.labelFontSize + 10, el.showHeaderAppIcon ? g_settings.iconSizePx + 4 : 18));
        int barRowH = S(std::max(28, g_settings.numberFontSize + 14));
        outHeight = S(16) + labelRowH + barRowH;
    } else {
        int minRowH = std::max(46, g_settings.numberFontSize + 32);
        if (el.leftUsesAppIcon) {
            minRowH = std::max(minRowH, g_settings.iconSizePx + 24);
        }
        outHeight = S(minRowH);
    }
}

static void PaintOsdWindow(HWND hwnd, HDC hdc) {
    EffectiveLayout el = GetEffectiveLayout();

    RECT rc;
    GetClientRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    UINT dpi = GetDpiForWindow(hwnd);
    float scale = dpi / 96.0f;
    auto S = [scale](float px) -> float { return px * scale; };

    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

    // Windows 11 Dark Flyout background (#2C2C2C)
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 44, 44, 44));
    g.FillRectangle(&bgBrush, 0, 0, w, h);

    float padX = S(16.0f);
    float padY = S(7.0f);

    Gdiplus::RectF rcLabelRow(0, 0, 0, 0);
    Gdiplus::RectF rcBarRow(padX, 0, static_cast<float>(w) - 2.0f * padX, static_cast<float>(h));

    if (el.isTwoLine) {
        float labelRowH = S(static_cast<float>(std::max(g_settings.labelFontSize + 10, el.showHeaderAppIcon ? g_settings.iconSizePx + 4 : 18)));
        float barRowH = static_cast<float>(h) - 2.0f * padY - labelRowH;

        if (el.headerVPos == VPos::Top) {
            rcLabelRow = Gdiplus::RectF(padX, padY, static_cast<float>(w) - 2.0f * padX, labelRowH);
            rcBarRow   = Gdiplus::RectF(padX, padY + labelRowH, static_cast<float>(w) - 2.0f * padX, barRowH);
        } else {
            rcBarRow   = Gdiplus::RectF(padX, padY, static_cast<float>(w) - 2.0f * padX, barRowH);
            rcLabelRow = Gdiplus::RectF(padX, padY + barRowH, static_cast<float>(w) - 2.0f * padX, labelRowH);
        }

        // 1. Draw Header Row (App Icon + Label)
        Gdiplus::Font* labelFont = CreateFontWithFallback(
            g_settings.fontName,
            static_cast<float>(g_settings.labelFontSize),
            Gdiplus::FontStyleRegular
        );
        Gdiplus::SolidBrush labelBrush(Gdiplus::Color(255, 238, 238, 238));

        float iconPx = (el.showHeaderAppIcon && g_currentAppBitmap) ? S(static_cast<float>(g_settings.iconSizePx)) : 0.0f;
        float iconGap = (iconPx > 0.0f) ? S(6.0f) : 0.0f;

        Gdiplus::RectF measuredBox;
        Gdiplus::StringFormat measureFmt;
        measureFmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
        g.MeasureString(g_currentLabel.c_str(), -1, labelFont, Gdiplus::PointF(0, 0), &measureFmt, &measuredBox);

        float maxTextW = std::max(0.0f, rcLabelRow.Width - iconPx - iconGap);
        float textW = std::min(measuredBox.Width + S(4.0f), maxTextW);
        float totalW = iconPx + iconGap + textW;

        float startX = rcLabelRow.X;
        if (el.headerAlign == HAlign::Center) {
            startX = rcLabelRow.X + (rcLabelRow.Width - totalW) * 0.5f;
        } else if (el.headerAlign == HAlign::Right) {
            startX = rcLabelRow.GetRight() - totalW;
        }

        if (iconPx > 0.0f && g_currentAppBitmap) {
            float iconY = rcLabelRow.Y + (rcLabelRow.Height - iconPx) * 0.5f;
            g.DrawImage(g_currentAppBitmap, Gdiplus::RectF(startX, iconY, iconPx, iconPx));
        }

        Gdiplus::StringFormat drawFmt;
        drawFmt.SetAlignment(Gdiplus::StringAlignmentNear);
        drawFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        drawFmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
        drawFmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

        Gdiplus::RectF rcText(startX + iconPx + iconGap, rcLabelRow.Y, textW + S(4.0f), rcLabelRow.Height);
        g.DrawString(g_currentLabel.c_str(), -1, labelFont, rcText, &drawFmt, &labelBrush);
        delete labelFont;
    }

    // 2. Draw Left Icon on the Volume Bar Row (App Icon in CompactAppIcon mode, or Speaker Glyph)
    float barCenterY = rcBarRow.Y + rcBarRow.Height * 0.5f;
    float leftIconSpanPx = S(static_cast<float>(el.leftUsesAppIcon ? std::max(20, g_settings.iconSizePx) : 20));

    if (el.leftUsesAppIcon && g_currentAppBitmap) {
        float iconPx = S(static_cast<float>(g_settings.iconSizePx));
        float iconX = padX + (leftIconSpanPx - iconPx) * 0.5f;
        float iconY = barCenterY - iconPx * 0.5f;
        g.DrawImage(g_currentAppBitmap, Gdiplus::RectF(iconX, iconY, iconPx, iconPx));
    } else {
        wchar_t iconGlyph = L'\uE767';
        if (g_currentMuted || g_currentVolume == 0) {
            iconGlyph = L'\uE74F';
        } else if (g_currentVolume < 33) {
            iconGlyph = L'\uE992';
        } else if (g_currentVolume < 66) {
            iconGlyph = L'\uE993';
        } else {
            iconGlyph = L'\uE994';
        }

        Gdiplus::FontFamily iconFamily(L"Segoe Fluent Icons");
        Gdiplus::FontFamily fallbackFamily(L"Segoe MDL2 Assets");
        const Gdiplus::FontFamily* pFamily = iconFamily.IsAvailable() ? &iconFamily : &fallbackFamily;

        Gdiplus::Font speakerFont(pFamily, S(16.0f), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
        Gdiplus::StringFormat iconFmt;
        iconFmt.SetAlignment(Gdiplus::StringAlignmentNear);
        iconFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        wchar_t glyphStr[2] = { iconGlyph, 0 };
        Gdiplus::RectF rcSpeaker(padX, rcBarRow.Y, leftIconSpanPx + S(8.0f), rcBarRow.Height);
        g.DrawString(glyphStr, 1, &speakerFont, rcSpeaker, &iconFmt, &whiteBrush);
    }

    // 3. Draw Anti-Aliased Progress Bar
    float barLeft = padX + leftIconSpanPx + S(12.0f);
    float rightSpace = el.showVolumeNumber
                           ? S(static_cast<float>(52 + std::max(0, (g_settings.numberFontSize - 13) * 3)))
                           : S(18.0f);
    float barRight = static_cast<float>(w) - rightSpace;
    float barWidth = std::max(10.0f, barRight - barLeft);
    float barThickness = S(4.5f);
    float barTop = barCenterY - barThickness * 0.5f;

    FillSmoothPill(g, barLeft, barTop, barWidth, barThickness, Gdiplus::Color(255, 158, 158, 158));

    int clampedVol = std::clamp(g_currentVolume, 0, 100);
    float fillW = (barWidth * clampedVol) / 100.0f;
    Gdiplus::Color accentColor = g_currentMuted
                                     ? Gdiplus::Color(255, 130, 130, 130)
                                     : GetWindows11AccentColor();

    if (fillW > 0.5f) {
        FillSmoothPill(g, barLeft, barTop, fillW, barThickness, accentColor);
    }

    if (el.showBarThumb) {
        float thumbX = barLeft + fillW;
        float outerR = S(6.5f);
        float innerR = S(3.8f);
        Gdiplus::SolidBrush outerBrush(Gdiplus::Color(255, 68, 68, 68));
        Gdiplus::SolidBrush innerBrush(accentColor);
        g.FillEllipse(&outerBrush, thumbX - outerR, barCenterY - outerR, outerR * 2.0f, outerR * 2.0f);
        g.FillEllipse(&innerBrush, thumbX - innerR, barCenterY - innerR, innerR * 2.0f, innerR * 2.0f);
    }

    // 4. Draw Volume Percentage Number (0-100) on the right side
    if (el.showVolumeNumber) {
        Gdiplus::Font* numFont = CreateFontWithFallback(
            g_settings.fontName,
            static_cast<float>(g_settings.numberFontSize),
            Gdiplus::FontStyleRegular
        );
        Gdiplus::SolidBrush numBrush(Gdiplus::Color(255, 255, 255, 255));

        Gdiplus::StringFormat numFmt;
        numFmt.SetAlignment(Gdiplus::StringAlignmentFar);
        numFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        std::wstring volText = std::to_wstring(clampedVol);
        Gdiplus::RectF rcNum(barRight, rcBarRow.Y, static_cast<float>(w) - barRight - padX, rcBarRow.Height);
        g.DrawString(volText.c_str(), -1, numFont, rcNum, &numFmt, &numBrush);
        delete numFont;
    }
}

static BYTE GetTargetMaxAlpha() {
    int pct = std::clamp(g_settings.windowOpacityPercent, 10, 100);
    return static_cast<BYTE>((pct * 255) / 100);
}

static void PositionAndShowOsd(HWND hwnd) {
    UINT dpi = GetDpiForWindow(hwnd);
    float scale = dpi / 96.0f;
    auto S = [scale](int px) -> int { return static_cast<int>(std::lround(px * scale)); };

    int width = 240, height = 48;
    CalculateWindowSize(dpi, width, height);

    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMon = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfoW(hMon, &mi);

    int workW = mi.rcWork.right - mi.rcWork.left;
    int workH = mi.rcWork.bottom - mi.rcWork.top;
    int offX = S(g_settings.offsetX);
    int offY = S(g_settings.offsetY);

    int x = 0, y = 0;
    switch (g_settings.windowPosition) {
        case WinPos::BottomCenter:
            x = mi.rcWork.left + (workW - width) / 2;
            y = mi.rcWork.bottom - height - offY;
            break;
        case WinPos::BottomLeft:
            x = mi.rcWork.left + offX;
            y = mi.rcWork.bottom - height - offY;
            break;
        case WinPos::BottomRight:
            x = mi.rcWork.right - width - offX;
            y = mi.rcWork.bottom - height - offY;
            break;
        case WinPos::TopCenter:
            x = mi.rcWork.left + (workW - width) / 2;
            y = mi.rcWork.top + offY;
            break;
        case WinPos::TopLeft:
            x = mi.rcWork.left + offX;
            y = mi.rcWork.top + offY;
            break;
        case WinPos::TopRight:
            x = mi.rcWork.right - width - offX;
            y = mi.rcWork.top + offY;
            break;
        case WinPos::Center:
            x = mi.rcWork.left + (workW - width) / 2;
            y = mi.rcWork.top + (workH - height) / 2;
            break;
        case WinPos::Custom:
            x = mi.rcMonitor.left + offX;
            y = mi.rcMonitor.top + offY;
            break;
    }

    KillTimer(hwnd, TIMER_FADE_OSD);
    g_bOsdActive = true;
    g_currentAlpha = GetTargetMaxAlpha();
    SetLayeredWindowAttributes(hwnd, 0, g_currentAlpha, LWA_ALPHA);

    SetWindowPos(hwnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(hwnd, nullptr, FALSE);
    SetTimer(hwnd, TIMER_HIDE_OSD, g_settings.displayDurationMs, nullptr);
}

static LRESULT CALLBACK OsdWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            DWORD cornerPref = 2; // DWMWCP_ROUND
            DwmSetWindowAttribute(hwnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &cornerPref, sizeof(cornerPref));
            BOOL darkMode = TRUE;
            DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));
            SetTimer(hwnd, TIMER_CONFIG_WATCH, 2000, nullptr);
            return 0;
        }

        case WM_OSD_SHOW_VOLUME: {
            auto* payload = reinterpret_cast<VolumeEventPayload*>(lParam);
            if (payload) {
                g_activeSliderIndex = payload->sliderIndex;
                g_currentLabel = FormatSliderLabel(payload->sliderIndex, payload->targetKeyLower);
                g_currentVolume = payload->volumePercent;
                g_currentMuted = payload->muted;

                delete g_currentAppBitmap;
                g_currentAppBitmap = LoadHighResBitmapForEvent(payload->sliderIndex, payload->processFullPath);

                delete payload;
                PositionAndShowOsd(hwnd);
            }
            return 0;
        }

        case WM_OSD_SESSION_CREATED: {
            auto* pNewSession = reinterpret_cast<IAudioSessionControl*>(lParam);
            if (pNewSession) {
                AttachSessionControl(pNewSession);
                pNewSession->Release();
            }
            return 0;
        }

        case WM_OSD_DEVICE_CHANGED: {
            InitAudioBindings();
            return 0;
        }

        case WM_OSD_RELOAD_SETTINGS: {
            SyncWithDeejConfigYaml(true);
            if (IsWindowVisible(hwnd)) {
                PositionAndShowOsd(hwnd);
            }
            return 0;
        }

        case WM_TIMER: {
            if (wParam == TIMER_HIDE_OSD) {
                KillTimer(hwnd, TIMER_HIDE_OSD);
                SetTimer(hwnd, TIMER_FADE_OSD, 16, nullptr);
            } else if (wParam == TIMER_FADE_OSD) {
                if (g_currentAlpha > 25) {
                    g_currentAlpha -= 25;
                    SetLayeredWindowAttributes(hwnd, 0, g_currentAlpha, LWA_ALPHA);
                } else {
                    g_currentAlpha = 0;
                    g_bOsdActive = false;
                    g_activeSliderIndex = -1;
                    KillTimer(hwnd, TIMER_FADE_OSD);
                    ShowWindow(hwnd, SW_HIDE);
                }
            } else if (wParam == TIMER_CONFIG_WATCH) {
                SyncWithDeejConfigYaml(false);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = w;
            bmi.bmiHeader.biHeight = -h;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            void* pBits = nullptr;
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP dibBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
            HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, dibBmp));

            PaintOsdWindow(hwnd, memDC);

            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(dibBmp);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_HIDE_OSD);
            KillTimer(hwnd, TIMER_FADE_OSD);
            KillTimer(hwnd, TIMER_CONFIG_WATCH);
            g_bOsdActive = false;
            g_activeSliderIndex = -1;
            delete g_currentAppBitmap;
            g_currentAppBitmap = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static DWORD WINAPI OsdThreadProc(LPVOID) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);

    SyncWithDeejConfigYaml(true);

    const wchar_t* className = L"WindhawkDeejOsdClass";
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = OsdWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    g_hOsdWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        className,
        L"Deej Volume OSD",
        WS_POPUP,
        0, 0, 240, 48,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                   __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&g_pEnumerator)))) {
        g_pDeviceClient = new CDeviceNotificationClient();
        g_pEnumerator->RegisterEndpointNotificationCallback(g_pDeviceClient);
        InitAudioBindings();
    }

    SetEvent(g_hInitEvent);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CleanupAudioBindings();
    if (g_pEnumerator && g_pDeviceClient) {
        g_pEnumerator->UnregisterEndpointNotificationCallback(g_pDeviceClient);
    }
    if (g_pDeviceClient) {
        g_pDeviceClient->Release();
        g_pDeviceClient = nullptr;
    }
    if (g_pEnumerator) {
        g_pEnumerator->Release();
        g_pEnumerator = nullptr;
    }

    UnregisterClassW(className, wc.hInstance);
    if (g_gdiplusToken) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
    CoUninitialize();
    return 0;
}

static std::wstring ReadStringSetting(PCWSTR name, PCWSTR defaultVal = L"") {
    PCWSTR val = Wh_GetStringSetting(name);
    std::wstring res = val ? val : defaultVal;
    if (val) Wh_FreeStringSetting(val);
    return res;
}

static void LoadSettings() {
    std::wstring presetStr = ReadStringSetting(L"WindowPreset", L"compact_app_icon");
    if (presetStr == L"win11_compact") g_settings.windowPreset = PresetMode::Win11Compact;
    else if (presetStr == L"two_line") g_settings.windowPreset = PresetMode::TwoLine;
    else if (presetStr == L"custom") g_settings.windowPreset = PresetMode::Custom;
    else g_settings.windowPreset = PresetMode::CompactAppIcon;

    g_settings.configYamlPath = ReadStringSetting(L"ConfigYamlPath", L"");
    g_settings.showThresholdPercent = std::clamp(Wh_GetIntSetting(L"ShowThresholdPercent"), 1, 50);
    g_settings.minPercentStep = std::clamp(Wh_GetIntSetting(L"MinPercentStep"), 1, 10);

    std::wstring winPosStr = ReadStringSetting(L"WindowPosition", L"bottom_center");
    if (winPosStr == L"bottom_left") g_settings.windowPosition = WinPos::BottomLeft;
    else if (winPosStr == L"bottom_right") g_settings.windowPosition = WinPos::BottomRight;
    else if (winPosStr == L"top_center") g_settings.windowPosition = WinPos::TopCenter;
    else if (winPosStr == L"top_left") g_settings.windowPosition = WinPos::TopLeft;
    else if (winPosStr == L"top_right") g_settings.windowPosition = WinPos::TopRight;
    else if (winPosStr == L"center") g_settings.windowPosition = WinPos::Center;
    else if (winPosStr == L"custom") g_settings.windowPosition = WinPos::Custom;
    else g_settings.windowPosition = WinPos::BottomCenter;

    g_settings.offsetX = Wh_GetIntSetting(L"OffsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"OffsetY");
    g_settings.barLengthPx = std::clamp(Wh_GetIntSetting(L"BarLengthPx"), 60, 1200);
    g_settings.windowOpacityPercent = std::clamp(Wh_GetIntSetting(L"WindowOpacityPercent"), 10, 100);
    g_settings.displayDurationMs = std::max(300, Wh_GetIntSetting(L"DisplayDurationMs"));
    g_settings.useSystemAccentColor = Wh_GetIntSetting(L"UseSystemAccentColor") != 0;

    g_settings.fontName = ReadStringSetting(L"FontName", L"Segoe UI");
    if (g_settings.fontName.empty()) {
        g_settings.fontName = L"Segoe UI";
    }
    g_settings.labelFontSize = std::clamp(Wh_GetIntSetting(L"LabelFontSize"), 8, 36);
    g_settings.numberFontSize = std::clamp(Wh_GetIntSetting(L"NumberFontSize"), 8, 36);
    g_settings.iconSizePx = std::clamp(Wh_GetIntSetting(L"IconSizePx"), 12, 48);

    std::wstring customHeaderStr = ReadStringSetting(L"CustomHeaderMode", L"two_line");
    if (customHeaderStr == L"single_speaker") g_settings.customHeaderMode = CustomRowLayout::SingleSpeaker;
    else if (customHeaderStr == L"single_app_icon") g_settings.customHeaderMode = CustomRowLayout::SingleAppIcon;
    else g_settings.customHeaderMode = CustomRowLayout::TwoLine;

    std::wstring hAlignStr = ReadStringSetting(L"TextHorizontalAlign", L"center");
    if (hAlignStr == L"center") g_settings.textHAlign = HAlign::Center;
    else if (hAlignStr == L"right") g_settings.textHAlign = HAlign::Right;
    else g_settings.textHAlign = HAlign::Left;

    std::wstring vPosStr = ReadStringSetting(L"TextVerticalPos", L"top");
    if (vPosStr == L"bottom") g_settings.textVPos = VPos::Bottom;
    else g_settings.textVPos = VPos::Top;

    g_settings.showVolumeNumber = Wh_GetIntSetting(L"ShowVolumeNumber") != 0;
    g_settings.showAppIcon = Wh_GetIntSetting(L"ShowAppIcon") != 0;
    g_settings.hideSliderNumber = Wh_GetIntSetting(L"HideSliderNumber") != 0;
    g_settings.showAppNameInUnmapped = Wh_GetIntSetting(L"ShowAppNameInUnmapped") != 0;
    g_settings.showBarThumb = Wh_GetIntSetting(L"ShowBarThumb") != 0;

    g_settings.customIconMaster = ReadStringSetting(L"CustomIconMaster");
    g_settings.customIconSlider1 = ReadStringSetting(L"CustomIconSlider1");
    g_settings.customIconSlider2 = ReadStringSetting(L"CustomIconSlider2");
    g_settings.customIconSlider3 = ReadStringSetting(L"CustomIconSlider3");
}

BOOL Wh_ModInit() {
    g_bShuttingDown = false;
    LoadSettings();

    g_hInitEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_hThread = CreateThread(nullptr, 0, OsdThreadProc, nullptr, 0, nullptr);
    if (g_hThread && g_hInitEvent) {
        WaitForSingleObject(g_hInitEvent, 3000);
    }
    if (g_hInitEvent) {
        CloseHandle(g_hInitEvent);
        g_hInitEvent = nullptr;
    }
    return g_hThread != nullptr;
}

void Wh_ModUninit() {
    g_bShuttingDown = true;
    if (g_hOsdWnd) {
        SendMessageW(g_hOsdWnd, WM_CLOSE, 0, 0);
        g_hOsdWnd = nullptr;
    }
    if (g_hThread) {
        WaitForSingleObject(g_hThread, 5000);
        CloseHandle(g_hThread);
        g_hThread = nullptr;
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    if (g_hOsdWnd) {
        PostMessageW(g_hOsdWnd, WM_OSD_RELOAD_SETTINGS, 0, 0);
    }
}
