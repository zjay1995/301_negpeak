// wpeak64_gui.cpp -- native 64-bit Windows GUI for the WPEAK port.
//
// Single main window, stacked top to bottom (legacy windows merged into one
// frame instead of separate popups):
//   menu bar        File / Acquire / Options / Help
//   icon toolbar    Open/Save/Export, Run/Cal/Stop, Settings, Manual
//                   Control, About (legacy toolbar style, GDI icons)
//   chromatogram    trace + baseline + numbered/NEG peaks and peak table,
//                   or the live trace while an acquisition runs
//   acquisition     phase, backend, zone temperatures (always visible)
//   element table   components: RT windows, response factors, alarm
//                   limits, calibration points, latest concentrations
//   status bar      baseline/noise/alarm/run state/loaded files
//
// Two floating dialogs: Options > Detector & Integration (data rate,
// noise/baseline, peak algorithm, ...) and Options > Manual Valve/Relay
// Control (legacy manual S/E/I/AZ/HEAT buttons -- toggles each configured
// hardware line directly, disabled while a run is in progress).
//
// Command line (optional): [run.csv] [method.ini] [/autorun] in any order.
//
// Build (MinGW-w64): see Makefile target build/wpeak64.exe

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "peak64.h"
#include "method64.h"
#include "acquire64.h"

using namespace wpeak64;

enum {
    IDM_OPEN_DATA = 101, IDM_OPEN_METHOD = 102, IDM_EXPORT = 103,
    IDM_EXIT = 104, IDM_OPEN_CAL = 105, IDM_SAVE_METHOD = 106,
    IDM_PRINT_REPORT = 107,
    IDM_RUN_START = 201, IDM_RUN_CAL = 202, IDM_RUN_ABORT = 203,
    IDM_ABOUT = 401, IDM_SETTINGS = 501, IDM_MANUAL = 502, IDM_ELEMENTS = 503,
    IDM_RELAYS = 504,
    IDC_SET_OK = 601, IDC_SET_CANCEL = 602,
    IDC_MANUAL_BASE = 700,   // + line index, up to ~64 controllable lines
    IDC_EL_LIST = 751, IDC_EL_NAME = 752, IDC_EL_RT = 753, IDC_EL_WINDOW = 754,
    IDC_EL_RESPONSE = 755, IDC_EL_AHIGH = 756, IDC_EL_ALOW = 757, IDC_EL_ACTIVE = 758,
    IDC_EL_ADD = 759, IDC_EL_UPDATE = 760, IDC_EL_DELETE = 761,
    IDC_EL_APPLY = 762, IDC_EL_CANCEL = 763,
    IDC_EL_DACCH = 764, IDC_EL_DACRANGE = 765, IDC_EL_CALIB = 766,
    IDC_EL_DETA = 767, IDC_EL_DETB = 768,
    IDC_CAL_LIST = 770,
    IDC_CAL_STD1 = 771, IDC_CAL_STD2 = 772, IDC_CAL_STD3 = 773, IDC_CAL_STD4 = 774,
    IDC_CAL_STD5 = 775, IDC_CAL_STD6 = 776, IDC_CAL_STD7 = 777, IDC_CAL_STD8 = 778,
    IDC_CAL_UPDATE = 779, IDC_CAL_APPLY = 780, IDC_CAL_CANCEL = 781,
    IDC_RL_LIST = 790, IDC_RL_NAME = 791, IDC_RL_LINE = 792,
    IDC_RL_ONTIME = 793, IDC_RL_OFFTIME = 794,
    IDC_RL_ADD = 795, IDC_RL_UPDATE = 796, IDC_RL_DELETE = 797,
    IDC_RL_APPLY = 798, IDC_RL_CANCEL = 799,
    // toolbar icons reused from the original Peak Works software (wpeak64.rc)
    IDB_OPEN = 901, IDB_SAVE = 902, IDB_HELP = 903,
};
static const UINT WM_ACQ_DONE = WM_APP + 1;

static Method                 g_method;
static std::vector<long>      g_trace;
static std::vector<ReportRow> g_rows;
static long                   g_noise = 0, g_baseline = 0;
// ---- Detector B (only meaningful when g_has_b) -----------------------------
static bool                   g_has_b = false;
static std::vector<long>      g_trace_b;
static std::vector<ReportRow> g_rows_b;
static long                   g_noise_b = 0, g_baseline_b = 0;
static std::string            g_data_path;    // empty = synthetic demo
static std::string            g_method_path;  // empty = defaults
static std::string            g_cal_path;     // calibration file (optional)

static HWND g_main = nullptr;
static HWND g_manualwnd = nullptr;   // Manual Valve/Relay Control (holds a Hardware conn.)
static bool g_autorun = false;   // /autorun: start a run at launch

// ---- modern industrial style -------------------------------------------------
// Segoe UI for labels/headers (falls back to Tahoma/Arial where missing),
// Consolas for tabular numeric data; charcoal header strip per window.
static HFONT g_font_ui = nullptr, g_font_ui_bold = nullptr, g_font_mono = nullptr;
// toolbar icons reused from the original Peak Works software (wpeak64.rc);
// null if the resource failed to load, in which case DrawToolIcon falls
// back to its GDI-drawn equivalent.
static HBITMAP g_bmp_open = nullptr, g_bmp_save = nullptr, g_bmp_help = nullptr;
static const COLORREF kHeaderBg   = RGB(38, 42, 48);    // charcoal
static const COLORREF kHeaderFg   = RGB(235, 238, 240);
static const COLORREF kAccent     = RGB(0, 120, 155);   // industrial teal
static const COLORREF kAlarmRed   = RGB(198, 40, 40);
static const COLORREF kNegOrange  = RGB(198, 110, 0);
static const COLORREF kTraceBlue  = RGB(20, 90, 180);
static const COLORREF kBaseGreen  = RGB(0, 140, 70);
static const COLORREF kGridGray   = RGB(120, 126, 132);

static void CreateFonts()
{
    g_font_ui = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Segoe UI");
    g_font_ui_bold = CreateFontA(-16, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Segoe UI");
    g_font_mono = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
}

static void LoadToolbarBitmaps(HINSTANCE inst)
{
    g_bmp_open = LoadBitmapA(inst, MAKEINTRESOURCEA(IDB_OPEN));
    g_bmp_save = LoadBitmapA(inst, MAKEINTRESOURCEA(IDB_SAVE));
    g_bmp_help = LoadBitmapA(inst, MAKEINTRESOURCEA(IDB_HELP));
}

// blits a small legacy bitmap centered in a toolbar button rect
static void DrawBmpIcon(HDC dc, HBITMAP bmp, const RECT &r)
{
    BITMAP bm;
    GetObjectA(bmp, sizeof bm, &bm);
    HDC mem = CreateCompatibleDC(dc);
    HGDIOBJ old = SelectObject(mem, bmp);
    int w = r.right - r.left - 4, h = r.bottom - r.top - 4;
    if(w > bm.bmWidth)  w = bm.bmWidth;    // never upscale these -- keep the retro look crisp
    if(h > bm.bmHeight) h = bm.bmHeight;
    int x = r.left + ((r.right - r.left) - w) / 2;
    int y = r.top  + ((r.bottom - r.top)  - h) / 2;
    BitBlt(dc, x, y, w, h, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
}

// bottom status bar: charcoal strip with teal-separated info segments;
// returns the y coordinate content must stay above
static int DrawStatusBar(HDC dc, const RECT &rc,
                         const std::vector<std::string> &segments)
{
    const int h = 28;
    RECT bar = rc; bar.top = bar.bottom - h;
    HBRUSH bg = CreateSolidBrush(kHeaderBg);
    FillRect(dc, &bar, bg);
    DeleteObject(bg);
    RECT accent = bar; accent.bottom = accent.top + 2;
    HBRUSH ab = CreateSolidBrush(kAccent);
    FillRect(dc, &accent, ab);
    DeleteObject(ab);

    HGDIOBJ old = SelectObject(dc, g_font_ui);
    int x = rc.left + 16, y = bar.top + 6;
    for(size_t i = 0; i < segments.size(); i++) {
        if(i) {
            SetTextColor(dc, kAccent);
            TextOutA(dc, x, y, "\xb7", 1);
            x += 14;
        }
        SetTextColor(dc, kHeaderFg);
        const std::string &s = segments[i];
        TextOutA(dc, x, y, s.c_str(), (int)s.size());
        SIZE sz;
        GetTextExtentPoint32A(dc, s.c_str(), (int)s.size(), &sz);
        x += sz.cx + 14;
        if(x > rc.right - 40) break;
    }
    SelectObject(dc, old);
    SetTextColor(dc, RGB(0,0,0));
    return bar.top;
}

static std::string BaseName(const std::string &path)
{
    size_t p = path.find_last_of("/\\");
    return p == std::string::npos ? path : path.substr(p + 1);
}

// ---- toolbar strip (second menu row with icon shortcuts) ----------------------
// Drawn under the native menu bar in the main window, legacy toolbar style:
// Open Data | Open Method | Save Method | Export || Run | Cal | Stop ||
// Settings | Manual Control | About. Icons are GDI-drawn.
struct ToolButton { int id; bool sep_before; };
static const ToolButton kToolButtons[] = {
    { IDM_OPEN_DATA,   false }, { IDM_OPEN_METHOD, false },
    { IDM_SAVE_METHOD, false }, { IDM_EXPORT,      false },
    { IDM_RUN_START,   true  }, { IDM_RUN_CAL,     false }, { IDM_RUN_ABORT, false },
    { IDM_SETTINGS,    true  }, { IDM_MANUAL,      false }, { IDM_ELEMENTS, false },
    { IDM_ABOUT,       true  },
};
static const int kToolH = 46, kToolBtn = 34, kToolPad = 6, kToolSep = 12;

static RECT ToolButtonRect(int index)
{
    int x = 12;
    for(int i = 0; i < index; i++) {
        if(kToolButtons[i + 1].sep_before) x += kToolSep;   // group gap
        x += kToolBtn + kToolPad;
    }
    RECT r = { x, kToolPad, x + kToolBtn, kToolPad + kToolBtn };
    return r;
}

static int ToolbarButtonAt(int x, int y)
{
    const int n = (int)(sizeof kToolButtons / sizeof kToolButtons[0]);
    if(y < 0 || y > kToolH) return 0;
    for(int i = 0; i < n; i++) {
        RECT r = ToolButtonRect(i);
        if(x >= r.left && x < r.right && y >= r.top && y < r.bottom)
            return kToolButtons[i].id;
    }
    return 0;
}

static void DrawToolIcon(HDC dc, int id, const RECT &r, bool enabled)
{
    COLORREF cMain = enabled ? RGB(210, 215, 220) : RGB(105, 110, 116);
    COLORREF cRun  = enabled ? RGB(60, 190, 90)   : RGB(70, 95, 78);
    COLORREF cStop = enabled ? RGB(225, 70, 60)   : RGB(105, 72, 70);
    COLORREF cTeal = enabled ? RGB(60, 180, 215)  : RGB(70, 105, 118);
    int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;

    HPEN pen = CreatePen(PS_SOLID, 2, cMain);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));

    switch(id) {
        case IDM_OPEN_DATA: {                        // folder (legacy icon, GDI fallback)
            if(g_bmp_open) { DrawBmpIcon(dc, g_bmp_open, r); break; }
            POINT p[] = { {r.left+7,r.top+12}, {r.left+13,r.top+12}, {r.left+15,r.top+15},
                          {r.right-7,r.top+15}, {r.right-7,r.bottom-9}, {r.left+7,r.bottom-9} };
            Polygon(dc, p, 6);
            break;
        }
        case IDM_OPEN_METHOD: {                      // document with lines
            Rectangle(dc, r.left+9, r.top+7, r.right-9, r.bottom-7);
            MoveToEx(dc, r.left+12, cy-4, nullptr); LineTo(dc, r.right-12, cy-4);
            MoveToEx(dc, r.left+12, cy,   nullptr); LineTo(dc, r.right-12, cy);
            MoveToEx(dc, r.left+12, cy+4, nullptr); LineTo(dc, r.right-12, cy+4);
            break;
        }
        case IDM_SAVE_METHOD: {                      // floppy disk (legacy icon, GDI fallback)
            if(g_bmp_save) { DrawBmpIcon(dc, g_bmp_save, r); break; }
            Rectangle(dc, r.left+8, r.top+8, r.right-8, r.bottom-8);
            Rectangle(dc, r.left+13, r.top+8, r.right-13, r.top+15);
            Rectangle(dc, r.left+12, cy+2, r.right-12, r.bottom-8);
            break;
        }
        case IDM_EXPORT: {                           // arrow out of tray
            MoveToEx(dc, r.left+8, r.bottom-11, nullptr); LineTo(dc, r.left+8, r.bottom-8);
            LineTo(dc, r.right-8, r.bottom-8); LineTo(dc, r.right-8, r.bottom-11);
            MoveToEx(dc, cx, r.bottom-12, nullptr); LineTo(dc, cx, r.top+8);
            MoveToEx(dc, cx-5, r.top+13, nullptr); LineTo(dc, cx, r.top+8);
            LineTo(dc, cx+5, r.top+13);
            break;
        }
        case IDM_RUN_START: {                        // green play triangle
            HBRUSH b = CreateSolidBrush(cRun);
            HPEN p2 = CreatePen(PS_SOLID, 1, cRun);
            SelectObject(dc, b); SelectObject(dc, p2);
            POINT p[] = { {r.left+11,r.top+8}, {r.left+11,r.bottom-8}, {r.right-9,cy} };
            Polygon(dc, p, 3);
            SelectObject(dc, pen); SelectObject(dc, GetStockObject(NULL_BRUSH));
            DeleteObject(b); DeleteObject(p2);
            break;
        }
        case IDM_RUN_CAL: {                          // teal triangle + C
            HBRUSH b = CreateSolidBrush(cTeal);
            HPEN p2 = CreatePen(PS_SOLID, 1, cTeal);
            SelectObject(dc, b); SelectObject(dc, p2);
            POINT p[] = { {r.left+9,r.top+8}, {r.left+9,r.bottom-8}, {r.right-13,cy} };
            Polygon(dc, p, 3);
            SelectObject(dc, pen); SelectObject(dc, GetStockObject(NULL_BRUSH));
            DeleteObject(b); DeleteObject(p2);
            SetTextColor(dc, cMain);
            TextOutA(dc, r.right-12, cy-8, "C", 1);
            break;
        }
        case IDM_RUN_ABORT: {                        // red stop square
            HBRUSH b = CreateSolidBrush(cStop);
            HPEN p2 = CreatePen(PS_SOLID, 1, cStop);
            SelectObject(dc, b); SelectObject(dc, p2);
            Rectangle(dc, r.left+10, r.top+10, r.right-10, r.bottom-10);
            SelectObject(dc, pen); SelectObject(dc, GetStockObject(NULL_BRUSH));
            DeleteObject(b); DeleteObject(p2);
            break;
        }
        case IDM_SETTINGS: {                         // gear: circle + spokes
            Ellipse(dc, cx-7, cy-7, cx+7, cy+7);
            for(int a = 0; a < 8; a++) {
                static const int dx[] = { 0, 7, 10, 7, 0, -7, -10, -7 };
                static const int dy[] = { -10, -7, 0, 7, 10, 7, 0, -7 };
                MoveToEx(dc, cx + dx[a] * 7 / 10, cy + dy[a] * 7 / 10, nullptr);
                LineTo(dc, cx + dx[a], cy + dy[a]);
            }
            break;
        }
        case IDM_MANUAL: {                           // valve symbol: bowtie on a pipe
            MoveToEx(dc, r.left+4, cy, nullptr); LineTo(dc, r.right-4, cy);
            HBRUSH b = CreateSolidBrush(cTeal);
            HPEN p2 = CreatePen(PS_SOLID, 1, cTeal);
            SelectObject(dc, b); SelectObject(dc, p2);
            POINT pl[] = { {cx-9,cy-7}, {cx-9,cy+7}, {cx,cy} };
            POINT pr[] = { {cx+9,cy-7}, {cx+9,cy+7}, {cx,cy} };
            Polygon(dc, pl, 3);
            Polygon(dc, pr, 3);
            SelectObject(dc, pen); SelectObject(dc, GetStockObject(NULL_BRUSH));
            DeleteObject(b); DeleteObject(p2);
            break;
        }
        case IDM_ELEMENTS: {                         // table grid with a pencil
            Rectangle(dc, r.left+6, r.top+8, r.right-12, r.bottom-10);
            MoveToEx(dc, r.left+6, cy-1, nullptr);   LineTo(dc, r.right-12, cy-1);
            MoveToEx(dc, (r.left+r.right)/2-5, r.top+8, nullptr);
            LineTo(dc, (r.left+r.right)/2-5, r.bottom-10);
            {
                HPEN p2 = CreatePen(PS_SOLID, 2, cTeal);
                SelectObject(dc, p2);
                MoveToEx(dc, r.right-13, r.bottom-6, nullptr); LineTo(dc, r.right-4, r.bottom-15);
                SelectObject(dc, pen);
                DeleteObject(p2);
            }
            break;
        }
        case IDM_ABOUT: {                            // question mark (legacy icon, GDI fallback)
            if(g_bmp_help) { DrawBmpIcon(dc, g_bmp_help, r); break; }
            SetTextColor(dc, cMain);
            TextOutA(dc, cx-4, cy-9, "?", 1);
            break;
        }
    }
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
}

// second menu row: charcoal strip of icon buttons + right-aligned title;
// returns the content top
static int DrawToolbar(HDC dc, const RECT &rc, bool running)
{
    RECT strip = rc; strip.bottom = strip.top + kToolH;
    HBRUSH bg = CreateSolidBrush(kHeaderBg);
    FillRect(dc, &strip, bg);
    DeleteObject(bg);
    RECT accent = strip; accent.top = strip.bottom; accent.bottom = strip.bottom + 3;
    HBRUSH ab = CreateSolidBrush(kAccent);
    FillRect(dc, &accent, ab);
    DeleteObject(ab);

    const int n = (int)(sizeof kToolButtons / sizeof kToolButtons[0]);
    HGDIOBJ oldFont = SelectObject(dc, g_font_ui);
    for(int i = 0; i < n; i++) {
        RECT r = ToolButtonRect(i);
        int id = kToolButtons[i].id;
        // button face -- the reused legacy bitmaps (folder/disk/help) are
        // opaque with a light Windows-3.x background, so give just those
        // buttons a matching light face instead of the usual dark one
        bool legacyBmp = (id == IDM_OPEN_DATA && g_bmp_open) ||
                         (id == IDM_SAVE_METHOD && g_bmp_save) ||
                         (id == IDM_ABOUT && g_bmp_help);
        HBRUSH face = CreateSolidBrush(legacyBmp ? RGB(192, 192, 192) : RGB(55, 60, 66));
        RECT br = r;
        FillRect(dc, &br, face);
        DeleteObject(face);
        bool enabled = true;
        if(id == IDM_RUN_START || id == IDM_RUN_CAL || id == IDM_MANUAL) enabled = !running;
        if(id == IDM_RUN_ABORT) enabled = running;
        SetBkMode(dc, TRANSPARENT);
        DrawToolIcon(dc, id, r, enabled);
    }
    // right-aligned title
    SelectObject(dc, g_font_ui_bold);
    SetTextColor(dc, kHeaderFg);
    const char *title = "GC301c GAS CHROMATOGRAPH  \xb7  WPEAK64";
    SIZE sz;
    GetTextExtentPoint32A(dc, title, (int)strlen(title), &sz);
    if(ToolButtonRect(n - 1).right + 24 + sz.cx < rc.right)
        TextOutA(dc, rc.right - sz.cx - 16, strip.top + 12, title, (int)strlen(title));
    SelectObject(dc, oldFont);
    SetTextColor(dc, RGB(0,0,0));
    return strip.bottom + 3;
}

// ---- shared state between the acquisition thread and the windows ------------
struct AcqShared {
    CRITICAL_SECTION cs;
    bool running = false, abort_req = false;
    bool is_cal = false;
    int  standard = 1;
    std::string phase;
    std::vector<std::pair<std::string,double>> zones;   // name, temp C
    std::vector<long> live;                             // growing trace
    std::vector<Peak> live_peaks;                        // identified as they finish
    long live_baseline = 0;
    AcquireResult result;
    std::string err;
    bool ok = false;
};
static AcqShared g_acq;

class GuiProgress : public AcquireProgress {
public:
    void OnPhase(const char *phase) override {
        EnterCriticalSection(&g_acq.cs);
        g_acq.phase = phase;
        LeaveCriticalSection(&g_acq.cs);
    }
    void OnPoint(long counts) override {
        EnterCriticalSection(&g_acq.cs);
        g_acq.live.push_back(counts);
        LeaveCriticalSection(&g_acq.cs);
        Sleep(2);   // pace simulated runs so the live view is watchable;
                    // negligible against real ADC sampling intervals
    }
    void OnLivePeaks(const std::vector<Peak> &peaks, long baseline) override {
        EnterCriticalSection(&g_acq.cs);
        g_acq.live_peaks = peaks;
        g_acq.live_baseline = baseline;
        LeaveCriticalSection(&g_acq.cs);
    }
    void OnZone(const char *name, double temp_c) override {
        EnterCriticalSection(&g_acq.cs);
        bool found = false;
        for(auto &z : g_acq.zones)
            if(z.first == name) { z.second = temp_c; found = true; }
        if(!found) g_acq.zones.push_back({ name, temp_c });
        LeaveCriticalSection(&g_acq.cs);
    }
    bool Aborted() override {
        EnterCriticalSection(&g_acq.cs);
        bool a = g_acq.abort_req;
        LeaveCriticalSection(&g_acq.cs);
        return a;
    }
};

static DWORD WINAPI AcqThread(LPVOID)
{
    Method m = g_method;                     // snapshot (incl. calibration)
    Hardware h;
    std::string err;
    bool ok = false;
    AcquireResult res;

    if(!OpenHardware(m.hw, m.zones, 1.0, h, err)) {
        // fall back to simulation so the GUI works everywhere
        m.hw.backend = "sim";
        if(!OpenHardware(m.hw, m.zones, 1.0, h, err)) goto done;
    }
    {
        GuiProgress prog;
        AcquireRun run(m, h, g_acq.is_cal ? g_acq.standard : 0, 1);
        ok = run.Run(res, false, err, &prog);
        if(ok && g_acq.is_cal) {
            run.UpdateCalibration(m, res);
            std::string cal = g_cal_path.empty() ? "cal.ini" : g_cal_path;
            std::string serr;
            if(SaveCalibration(cal, m, serr))
                g_method = m;                // keep the new calibration live
        }
        CloseHardware(h);
    }
done:
    EnterCriticalSection(&g_acq.cs);
    g_acq.result = res;
    g_acq.err = err;
    g_acq.ok = ok;
    g_acq.running = false;
    // the phase left by the sequencer only reaches "DONE" on a clean finish;
    // on abort/error it would otherwise stick at whatever phase was active
    // (e.g. "ANALYZE") forever, contradicting the IDLE status shown once
    // running goes false. Force it to a state that matches the outcome.
    if(!ok) g_acq.phase = (err == "aborted") ? "STOPPED" : "ERROR";
    LeaveCriticalSection(&g_acq.cs);
    PostMessageA(g_main, WM_ACQ_DONE, 0, 0);
    return 0;
}

static bool StartAcquisition(HWND hwnd, bool is_cal, int standard)
{
    if(g_manualwnd) {
        MessageBoxA(hwnd, "Close Manual Control before starting a run.", "WPEAK64",
                    MB_OK | MB_ICONWARNING);
        return false;
    }
    EnterCriticalSection(&g_acq.cs);
    bool busy = g_acq.running;
    if(!busy) {
        g_acq.running = true;
        g_acq.abort_req = false;
        g_acq.is_cal = is_cal;
        g_acq.standard = standard;
        g_acq.phase = "starting";
        g_acq.zones.clear();
        g_acq.live.clear();
        g_acq.live_peaks.clear();
        g_acq.live_baseline = 0;
    }
    LeaveCriticalSection(&g_acq.cs);
    if(busy) {
        MessageBoxA(hwnd, "An acquisition is already running.", "WPEAK64",
                    MB_OK | MB_ICONINFORMATION);
        return false;
    }
    HANDLE th = CreateThread(nullptr, 0, AcqThread, nullptr, 0, nullptr);
    if(th) CloseHandle(th);
    return th != nullptr;
}

// ---- analysis of files / synthetic demo --------------------------------------
static bool RunAnalysis(HWND hwnd, std::string &err)
{
    Method m;
    m.det.MinHeight = 20;                     // demo default
    if(!g_method_path.empty() && !LoadMethod(g_method_path, m, err))
        return false;
    if(!g_cal_path.empty()) {
        std::string cerr;
        if(!LoadCalibration(g_cal_path, m, cerr)) { err = cerr; return false; }
    }

    // start empty: no demo data -- the plot fills from File > Open Data or a run
    std::vector<long> y;
    if(!g_data_path.empty()) {
        if(!LoadChromatogram(g_data_path, y, err))
            return false;
    }

    g_method = m;
    if(y.empty()) {
        g_trace.clear();
        g_rows.clear();
        g_noise = g_baseline = 0;
    }
    else {
        Integrator integ(m.det, m.data_rate, m.analysis_time);
        for(long v : y)
            integ.ProcessPoint(v);
        g_trace    = y;
        g_rows     = BuildReport(integ.peaks, m);
        EvaluateAlarms(g_rows, m);
        g_noise    = integ.noise;
        g_baseline = integ.act_thresh;
    }

    char title[512];
    std::snprintf(title, sizeof title, "GC301c - WPEAK64 (64-bit Windows port) - %s%s%s",
                  g_data_path.empty() ? "no data" : g_data_path.c_str(),
                  g_method_path.empty() ? "" : " / ",
                  g_method_path.c_str());
    if(hwnd) SetWindowTextA(hwnd, title);
    return true;
}

// ---- shared chromatogram painter ---------------------------------------------
static void PaintTrace(HDC dc, RECT plot, const std::vector<long> &trace,
                       long baseline, const std::vector<ReportRow> *rows,
                       int data_rate)
{
    if(trace.size() < 2) return;
    long ymin = trace[0], ymax = trace[0];
    for(long v : trace) { if(v < ymin) ymin = v; if(v > ymax) ymax = v; }
    if(baseline) { if(baseline < ymin) ymin = baseline; if(baseline > ymax) ymax = baseline; }
    long yspan = ymax - ymin; if(yspan < 1) yspan = 1;
    ymin -= yspan / 10; ymax += yspan / 10; yspan = ymax - ymin;
    const long n = (long)trace.size();

    auto X = [&](double i) { return plot.left + (int)((double)(plot.right - plot.left) * i / (n - 1)); };
    auto Y = [&](double v) { return plot.bottom - (int)((double)(plot.bottom - plot.top) * (v - ymin) / yspan); };

    HPEN frame = CreatePen(PS_SOLID, 1, kGridGray);
    HGDIOBJ oldPen = SelectObject(dc, frame);
    HGDIOBJ oldFont = SelectObject(dc, g_font_mono);
    Rectangle(dc, plot.left, plot.top, plot.right, plot.bottom);

    long total_s = n / data_rate;
    long step = total_s > 0 ? (total_s + 5) / 6 : 1; if(step < 1) step = 1;
    for(long s = 0; s <= total_s; s += step) {
        int x = X((double)s * data_rate);
        MoveToEx(dc, x, plot.bottom, nullptr); LineTo(dc, x, plot.bottom + 5);
        char lbl[16]; int len = std::snprintf(lbl, sizeof lbl, "%lds", s);
        SetTextColor(dc, kGridGray);
        TextOutA(dc, x - 8, plot.bottom + 8, lbl, len);
    }
    // x-axis title, centered under the tick labels
    {
        const char *xt = "Retention Time";
        SIZE sz; GetTextExtentPoint32A(dc, xt, (int)strlen(xt), &sz);
        SetTextColor(dc, kGridGray);
        TextOutA(dc, (plot.left + plot.right) / 2 - sz.cx / 2, plot.bottom + 24, xt, (int)strlen(xt));
    }

    // y-axis tick labels (detector signal, counts) + light gridlines
    {
        const int nticks = 5;
        for(int t = 0; t <= nticks; t++) {
            double v = ymin + yspan * t / nticks;
            int yy = Y(v);
            HPEN gridPen = CreatePen(PS_DOT, 1, RGB(225,227,229));
            SelectObject(dc, gridPen);
            MoveToEx(dc, plot.left, yy, nullptr); LineTo(dc, plot.right, yy);
            SelectObject(dc, frame);
            DeleteObject(gridPen);
            MoveToEx(dc, plot.left - 5, yy, nullptr); LineTo(dc, plot.left, yy);
            char lbl[16]; int len = std::snprintf(lbl, sizeof lbl, "%ld", (long)v);
            SIZE sz; GetTextExtentPoint32A(dc, lbl, len, &sz);
            SetTextColor(dc, kGridGray);
            TextOutA(dc, plot.left - 9 - sz.cx, yy - 7, lbl, len);
        }
        // y-axis title, vertical, along the left margin
        HFONT vfont = CreateFontA(-13, 0, 900, 900, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  VARIABLE_PITCH | FF_SWISS, "Segoe UI");
        HGDIOBJ oldVFont = SelectObject(dc, vfont);
        const char *yt = "Detector Signal (counts)";
        SetTextColor(dc, kGridGray);
        TextOutA(dc, plot.left - 60, (plot.top + plot.bottom) / 2 + (int)strlen(yt) * 3,
                yt, (int)strlen(yt));
        SelectObject(dc, oldVFont);
        DeleteObject(vfont);
    }
    SetTextColor(dc, RGB(0,0,0));

    if(baseline) {
        HPEN basePen = CreatePen(PS_DOT, 1, kBaseGreen);
        SelectObject(dc, basePen);
        MoveToEx(dc, plot.left, Y((double)baseline), nullptr);
        LineTo(dc, plot.right, Y((double)baseline));
        SelectObject(dc, frame);
        DeleteObject(basePen);
    }

    HPEN tracePen = CreatePen(PS_SOLID, 1, kTraceBlue);
    SelectObject(dc, tracePen);
    MoveToEx(dc, X(0), Y((double)trace[0]), nullptr);
    for(long i = 1; i < n; i++)
        LineTo(dc, X((double)i), Y((double)trace[(size_t)i]));

    if(rows) {
        HPEN posPen = CreatePen(PS_SOLID, 2, kAlarmRed);
        HPEN negPen = CreatePen(PS_SOLID, 2, kNegOrange);
        SelectObject(dc, g_font_ui);
        for(const ReportRow &r : *rows) {
            const Peak &p = r.peak;
            bool neg = p.Height < 0;
            SelectObject(dc, neg ? negPen : posPen);
            int xa = X((double)p.From), xb = X((double)p.To), xm = X((double)p.Time);
            int yb = Y((double)baseline);
            MoveToEx(dc, xa, yb - 6, nullptr); LineTo(dc, xa, yb + 6);
            MoveToEx(dc, xb, yb - 6, nullptr); LineTo(dc, xb, yb + 6);
            MoveToEx(dc, xa, yb, nullptr);     LineTo(dc, xb, yb);

            int yap = Y((double)(baseline + p.Height));
            char lbl[64]; int len;
            SetTextColor(dc, neg ? kNegOrange : kAlarmRed);
            if(neg) {
                // numbered in the same sequence as positive peaks (see
                // Integrator::DetectNegativePeak), shown alongside "NEG" so
                // the negative peak's index is visible on the graph too --
                // mirrors the positive-peak layout (number, then label).
                len = std::snprintf(lbl, sizeof lbl, "%d", p.Num);
                TextOutA(dc, xm - 4, yap + 6, lbl, len);
                len = std::snprintf(lbl, sizeof lbl, "NEG");
                TextOutA(dc, xm - 12, yap + 24, lbl, len);
            }
            else {
                len = std::snprintf(lbl, sizeof lbl, "%d", p.Num);
                TextOutA(dc, xm - 4, yap - 18, lbl, len);
                if(r.component >= 0) {
                    len = (int)r.name.size();
                    TextOutA(dc, xm - 4 * len, yap - 36, r.name.c_str(), len);
                }
            }
        }
        SetTextColor(dc, RGB(0,0,0));
        DeleteObject(posPen); DeleteObject(negPen);
    }
    SelectObject(dc, oldPen);
    SelectObject(dc, oldFont);
    DeleteObject(frame); DeleteObject(tracePen);
}

// ---- main window --------------------------------------------------------------
// Single-frame layout, sections stacked top to bottom: chromatogram (Table A
// + Graph A, or the live trace while acquiring), the acquisition strip
// (phase/backend/zone temperatures, always visible), the element table, and
// the instrument status bar along the very bottom.
static std::string FmtTimeMS(double seconds)
{
    long m = (long)(seconds / 60);
    long s = (long)(seconds - m * 60 + 0.5);
    if(s == 60) { m++; s = 0; }
    char b[16];
    std::snprintf(b, sizeof b, "%ld:%02ld", m, s);
    return b;
}

// mini section header (smaller than DrawHeaderStrip) starting at a given y;
// used to stack multiple panels inside one window. Returns content top.
static int DrawSectionHeader(HDC dc, const RECT &rc, int y, const char *title)
{
    RECT strip = { rc.left, y, rc.right, y + 24 };
    HBRUSH bg = CreateSolidBrush(kHeaderBg);
    FillRect(dc, &strip, bg);
    DeleteObject(bg);
    RECT accent = { rc.left, strip.bottom, rc.right, strip.bottom + 2 };
    HBRUSH ab = CreateSolidBrush(kAccent);
    FillRect(dc, &accent, ab);
    DeleteObject(ab);
    HGDIOBJ old = SelectObject(dc, g_font_ui_bold);
    SetTextColor(dc, kHeaderFg);
    TextOutA(dc, rc.left + 12, strip.top + 4, title, (int)strlen(title));
    SelectObject(dc, old);
    SetTextColor(dc, RGB(0,0,0));
    return strip.bottom + 2;
}

static void PaintMain(HDC dc, const RECT &rc)
{
    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    EnterCriticalSection(&g_acq.cs);
    bool running = g_acq.running;
    bool is_cal = g_acq.is_cal;
    std::string phase = g_acq.phase;
    auto zones = g_acq.zones;
    std::vector<long> live = running ? g_acq.live : std::vector<long>();
    std::vector<Peak> live_peaks = running ? g_acq.live_peaks : std::vector<Peak>();
    long live_baseline = g_acq.live_baseline;
    LeaveCriticalSection(&g_acq.cs);

    // Peaks are identified on the go: as soon as the Integrator finishes one
    // (live_peaks grows during ANALYZE, not just at the end of the run), run
    // it through the same component matchup used for the final report so the
    // table and the growing trace show identifications immediately.
    std::vector<ReportRow> liveRows;
    if(running && !live_peaks.empty()) {
        liveRows = BuildReport(live_peaks, g_method);
        EvaluateAlarms(liveRows, g_method);
    }

    // alarm state for the status bar: final report OR'd with whatever the
    // live-identified peaks have tripped so far during a run
    int alarm = ALARM_NONE;
    for(const ReportRow &r : g_rows)   alarm |= r.alarm;
    for(const ReportRow &r : liveRows) alarm |= r.alarm;

    // second menu row: icon toolbar (Run/Cal/Stop and file/window shortcuts)
    int top = DrawToolbar(dc, rc, running);

    // instrument status bar along the very bottom of the window
    std::vector<std::string> segs;
    segs.push_back("Baseline " + std::to_string(g_baseline));
    segs.push_back("Noise "    + std::to_string(g_noise));
    segs.push_back(alarm == ALARM_NONE ? "ALARM none" :
                   alarm == ALARM_HIGH ? "ALARM HIGH" :
                   alarm == ALARM_LOW  ? "ALARM LOW"  : "ALARM HIGH+LOW");
    segs.push_back(running ? "ACQ " + phase : "ACQ idle");
    if(g_has_b) {
        int alarm_b = ALARM_NONE;
        for(const ReportRow &r : g_rows_b) alarm_b |= r.alarm;
        segs.push_back("DET B " + std::to_string(g_rows_b.size()) + " pk" +
                       (alarm_b ? " *ALARM*" : ""));
    }
    else if(g_method.det_b_enabled)
        segs.push_back("DET B armed");
    segs.push_back("DATA " + std::string(g_data_path.empty() ? "none"
                                         : BaseName(g_data_path).c_str()));
    segs.push_back("METHOD " + std::string(g_method_path.empty() ? "defaults"
                                           : BaseName(g_method_path).c_str()));
    segs.push_back("CAL " + std::string(g_cal_path.empty() ? "none"
                                        : BaseName(g_cal_path).c_str()));
    int bottom = DrawStatusBar(dc, rc, segs);

    int total = bottom - top;
    if(total < 80) return;
    int y = top;
    bool haveData = !g_trace.empty() || running;

    // ---- Section 1: peak table -- identified on the go while a run is in
    // progress (liveRows grows peak-by-peak as the Integrator finishes each
    // one, so identifications show up immediately, not only once the run
    // ends), or the final report / an idle-state banner otherwise. ----------
    int tableH;
    // running rows start 20px lower than the idle/final table (tpanel.top+30
    // vs +10) to leave room for the "ACQUIRING . phase" line above them, so
    // the panel needs 20px more than the idle formula for the same row count
    // -- otherwise the last live-identified row gets clipped.
    if(running) tableH = liveRows.empty() ? 40
                        : 64 + (int)(liveRows.size() < 8 ? liveRows.size() : 8) * 18;
    else if(!haveData) tableH = 40;
    else tableH = 44 + (int)(g_rows.size() < 8 ? g_rows.size() : 8) * 18;
    RECT tpanel = { rc.left, y, rc.right, y + tableH };

    auto draw_row_table = [&](int ty, const std::vector<ReportRow> &rows) {
        double habs = 0, aabs = 0;
        for(const ReportRow &r : rows) {
            habs += r.peak.Height < 0 ? -(double)r.peak.Height : (double)r.peak.Height;
            aabs += r.peak.Area < 0 ? -r.peak.Area : r.peak.Area;
        }
        if(habs <= 0) habs = 1;
        if(aabs <= 0) aabs = 1;

        HGDIOBJ oldFont = SelectObject(dc, g_font_mono);
        char line[200]; int len;
        len = std::snprintf(line, sizeof line,
            "%-4s %-14s %10s %12s %8s %14s %8s %8s  %s",
            "Num", "Name", "Conc", "Height", "%", "Area", "%", "Time", "Alarm");
        SetTextColor(dc, kGridGray);
        TextOutA(dc, 20, ty, line, len); ty += 20;
        for(const ReportRow &r : rows) {
            if(ty > tpanel.bottom - 14) break;
            const Peak &p = r.peak;
            bool neg = p.Height < 0;
            char num[8], conc[24];
            std::snprintf(num, sizeof num, "%d", p.Num);   // one sequence, neg peaks included
            if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
            else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
            const char *al = r.alarm == ALARM_NONE ? "" :
                             r.alarm == ALARM_HIGH ? "HIGH" :
                             r.alarm == ALARM_LOW  ? "LOW"  : "H+L";
            len = std::snprintf(line, sizeof line,
                "%-4s %-14s %10s %12ld %8.2f %14.0f %8.2f %8s  %s",
                num, neg ? "Neg" : r.name.c_str(), conc,
                p.Height, 100.0 * p.Height / habs,
                p.Area,   100.0 * p.Area / aabs,
                FmtTimeMS((double)p.Time / g_method.data_rate).c_str(), al);
            SetTextColor(dc, r.alarm ? kAlarmRed : neg ? kNegOrange : RGB(30,32,34));
            TextOutA(dc, 20, ty, line, len); ty += 18;
        }
        SetTextColor(dc, RGB(0,0,0));
        SelectObject(dc, oldFont);
    };

    if(running) {
        HGDIOBJ of = SelectObject(dc, g_font_ui_bold);
        SetTextColor(dc, kAccent);
        std::string s = std::string(is_cal ? "CALIBRATING" : "ACQUIRING") + "  \xb7  " +
                        (phase.empty() ? std::string("-") : phase);
        if(!liveRows.empty())
            s += "  \xb7  " + std::to_string(liveRows.size()) + " peak(s) identified so far";
        TextOutA(dc, 20, tpanel.top + 10, s.c_str(), (int)s.size());
        SelectObject(dc, of);
        SetTextColor(dc, RGB(0,0,0));
        if(!liveRows.empty())
            draw_row_table(tpanel.top + 30, liveRows);
    }
    else if(!haveData) {
        HGDIOBJ of = SelectObject(dc, g_font_ui);
        SetTextColor(dc, kGridGray);
        const char *hint =
            "No data.  Open a chromatogram (File > Open Data) or press Run to start an acquisition.";
        TextOutA(dc, 20, tpanel.top + 12, hint, (int)strlen(hint));
        SelectObject(dc, of);
        SetTextColor(dc, RGB(0,0,0));
    }
    else {
        draw_row_table(tpanel.top + 10, g_rows);
    }
    y += tableH;
    { RECT div = { rc.left, y, rc.right, y + 3 };
      HBRUSH db = CreateSolidBrush(kAccent); FillRect(dc, &div, db); DeleteObject(db); }
    y += 3;

    // ---- Section 2: acquisition status (always visible, up/down stack) -------
    // One status word (IDLE/RUNNING/STOPPED/ERROR, color-coded) plus a row of
    // compact fields sized to their actual text (no fixed-offset gaps), and a
    // second row for zone temperatures only when there are any to show.
    const char *status = running ? (is_cal ? "CALIBRATING" : "RUNNING")
                        : phase == "STOPPED" ? "STOPPED"
                        : phase == "ERROR"   ? "ERROR"
                        : "IDLE";
    COLORREF statusColor = running ? kAccent
                          : phase == "STOPPED" ? kNegOrange
                          : phase == "ERROR"   ? kAlarmRed
                          : kGridGray;
    int acqH = zones.empty() ? 52 : 72;
    {
        RECT panel = { rc.left, y, rc.right, y + acqH };
        int aTop = DrawSectionHeader(dc, panel, panel.top, "ACQUISITION");
        HGDIOBJ of = SelectObject(dc, g_font_ui);
        char buf[64]; int tx = 20, ty = aTop + 6;
        auto field = [&](const char *text, COLORREF color) {
            int len = (int)std::strlen(text);
            SetTextColor(dc, color);
            TextOutA(dc, tx, ty, text, len);
            SIZE sz; GetTextExtentPoint32A(dc, text, len, &sz);
            tx += sz.cx + 22;
        };
        field(status, statusColor);
        std::snprintf(buf, sizeof buf, "Backend: %s", g_method.hw.backend.c_str());
        field(buf, RGB(30,32,34));
        std::snprintf(buf, sizeof buf, "Phase: %s", phase.empty() ? "-" : phase.c_str());
        field(buf, RGB(30,32,34));
        std::snprintf(buf, sizeof buf, "Points: %zu", live.size());
        field(buf, RGB(30,32,34));
        if(!zones.empty()) {
            tx = 20; ty += 20;
            for(auto &z : zones) {
                std::snprintf(buf, sizeof buf, "%s  %.1f\xb0""C", z.first.c_str(), z.second);
                field(buf, RGB(30,32,34));
            }
        }
        SetTextColor(dc, RGB(0,0,0));
        SelectObject(dc, of);
    }
    y += acqH;
    { RECT div = { rc.left, y, rc.right, y + 3 };
      HBRUSH db = CreateSolidBrush(kAccent); FillRect(dc, &div, db); DeleteObject(db); }
    y += 3;

    // ---- Section 3: element table (component list) ---------------------------
    // Always present, and sized generously: a minimum height so it stays a
    // prominent fixture even with 0-3 components, growing with the actual
    // component count (row cap raised from 8 to 14) rather than staying a
    // cramped 3-row strip.
    const int kElemMaxRows = 14;
    int elemRows = (int)(g_method.components.size() < (size_t)kElemMaxRows
                         ? g_method.components.size() : (size_t)kElemMaxRows);
    int elemH = 71 + elemRows * 17;
    if(elemH < 260) elemH = 260;   // always keep a large, prominent panel
    {
        RECT panel = { rc.left, y, rc.right, y + elemH };
        char hdr[120];
        std::snprintf(hdr, sizeof hdr, "ELEMENT TABLE  \xb7  %zu COMPONENTS  \xb7  %s METHOD",
                      g_method.components.size(),
                      g_method.detect_meth == 0 ? "HEIGHT" : "AREA");
        int eTop = DrawSectionHeader(dc, panel, panel.top, hdr);

        HGDIOBJ oldFont = SelectObject(dc, g_font_mono);
        char line[240]; int len; int ty = eTop + 6;
        len = std::snprintf(line, sizeof line,
            "%-14s %-7s %-7s %-9s %-8s %-8s %-24s %-10s %s",
            "Component", "RT(s)", "Win(s)", "RespFact", "AlarmHi", "AlarmLo",
            "Calibration (conc@resp)", "LastConc", "Alarm");
        SetTextColor(dc, kGridGray);
        TextOutA(dc, 20, ty, line, len); ty += 19;

        size_t shown = 0;
        for(size_t i = 0; i < g_method.components.size(); i++) {
            if(ty > panel.bottom - 14) break;
            shown++;
            const Component &c = g_method.components[i];
            char cal[96] = ""; size_t off = 0;
            for(int s = 0; s < STAND_NUM64 && off + 16 < sizeof cal; s++)
                if(c.cal[s].valid)
                    off += std::snprintf(cal + off, sizeof cal - off, "%g@%g ",
                                         c.cal[s].conc, c.cal[s].resp);
            if(!off) std::snprintf(cal, sizeof cal, "(none)");
            const ReportRow *last = nullptr;
            for(const ReportRow &r : g_rows)
                if(r.component == (int)i) last = &r;
            char conc[24] = "-";
            const char *al = "";
            if(last && last->calibrated)
                std::snprintf(conc, sizeof conc, "%g", last->concentration);
            if(last)
                al = last->alarm == ALARM_NONE ? "" :
                     last->alarm == ALARM_HIGH ? "HIGH" :
                     last->alarm == ALARM_LOW  ? "LOW"  : "H+L";

            len = std::snprintf(line, sizeof line,
                "%-14s %-7g %-7g %-9g %-8g %-8g %-24s %-10s %s",
                c.name.c_str(), c.peak_rt, c.window, c.response,
                c.alarm_high, c.alarm_low, cal, conc, al);
            SetTextColor(dc, (last && last->alarm) ? kAlarmRed
                             : c.active_yn ? RGB(30,32,34) : kGridGray);
            TextOutA(dc, 20, ty, line, len); ty += 17;
        }
        if(g_method.components.empty()) {
            SetTextColor(dc, kGridGray);
            const char *e = "(no components -- Options > Edit Element Table to add some)";
            TextOutA(dc, 20, ty, e, (int)strlen(e));
        }
        else if(shown < g_method.components.size()) {
            SetTextColor(dc, kGridGray);
            char more[48];
            int mlen = std::snprintf(more, sizeof more, "... %zu more (resize window to see all)",
                                     g_method.components.size() - shown);
            TextOutA(dc, 20, ty, more, mlen);
        }
        SetTextColor(dc, RGB(0,0,0));
        SelectObject(dc, oldFont);
    }
    y += elemH;
    { RECT div = { rc.left, y, rc.right, y + 3 };
      HBRUSH db = CreateSolidBrush(kAccent); FillRect(dc, &div, db); DeleteObject(db); }
    y += 3;

    // ---- Section 4: chromatogram, bottom-most and largest -- the same 40px
    // bottom margin the original single-panel layout used, so the time-axis
    // tick labels and axis title always have room and are never painted over
    // by a panel below them. ----------------------------------------------------
    if(y < bottom) {
        RECT plot = { rc.left + 70, y + 8, rc.right - 30, bottom - 40 };
        if(plot.right - plot.left >= 50 && plot.bottom - plot.top >= 50) {
            if(running) {
                if(live.size() >= 2)
                    PaintTrace(dc, plot, live, live_baseline,
                               liveRows.empty() ? nullptr : &liveRows,
                               g_method.data_rate > 0 ? g_method.data_rate : 10);
                else {
                    HGDIOBJ f2 = SelectObject(dc, g_font_ui);
                    SetTextColor(dc, kGridGray);
                    const char *w = "waiting for the ANALYZE phase...";
                    TextOutA(dc, plot.left, plot.top, w, (int)strlen(w));
                    SelectObject(dc, f2);
                    SetTextColor(dc, RGB(0,0,0));
                }
            }
            else if(haveData) {
                PaintTrace(dc, plot, g_trace, g_baseline, &g_rows, g_method.data_rate);
            }
        }
    }
}

// ---- window procedures -----------------------------------------------------------
static void DoubleBufferPaint(HWND hwnd, void (*painter)(HDC, const RECT &))
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    RECT rc; GetClientRect(hwnd, &rc);
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ old = SelectObject(mem, bmp);
    HGDIOBJ oldFont = SelectObject(mem, g_font_ui);
    painter(mem, rc);
    SelectObject(mem, oldFont);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bmp); DeleteDC(mem);
    EndPaint(hwnd, &ps);
}

// ---- settings window -----------------------------------------------------------
// Detector & integration parameters edited in-place (legacy Detector dialog).
// OK re-runs the integrator over the loaded chromatogram with the new values;
// File > Save Method writes them to an INI.
static HWND g_setwnd = nullptr;

struct SetField { const char *label; HWND edit; };
static SetField g_set_fields[] = {
    { "Data rate (pts/s)",             nullptr },
    { "Analysis time (s)",             nullptr },
    { "Segment width (pts)",           nullptr },
    { "Noise/baseline start (s)",      nullptr },
    { "Noise/baseline length (s)",     nullptr },
    { "Min peak height",               nullptr },
    { "Min peak area",                 nullptr },
    { "Peak algorithm (0=proj 1=skim)",nullptr },
    { "Detect method (0=height 1=area)", nullptr },
    // ---- Detector B (optional second channel, legacy NUMDETECTORS=2) ------
    { "Detector B: ADC channel (-1=off)", nullptr },
    { "Detector B: min peak height",   nullptr },
    { "Detector B: detect method (0=height 1=area)", nullptr },
};
enum { SETF_DETB_CHANNEL = 9, SETF_DETB_MINHEIGHT = 10, SETF_DETB_DETMETH = 11 };
static HWND g_set_known = nullptr;      // known peaks only checkbox
static HWND g_set_detb_enable = nullptr; // enable Detector B checkbox

// re-run the integrator over the currently loaded trace after a settings
// change (does not reload files, so edits are not clobbered)
static void ReprocessTrace()
{
    if(g_trace.empty()) return;
    Integrator integ(g_method.det, g_method.data_rate, g_method.analysis_time);
    for(long v : g_trace)
        integ.ProcessPoint(v);
    g_rows = BuildReport(integ.peaks, g_method);
    EvaluateAlarms(g_rows, g_method);
    g_noise    = integ.noise;
    g_baseline = integ.act_thresh;
    if(g_main) InvalidateRect(g_main, nullptr, FALSE);   // element table is part of g_main now
}

static void SetEditText(HWND edit, double v)
{
    char b[32];
    std::snprintf(b, sizeof b, "%g", v);
    SetWindowTextA(edit, b);
}

static double GetEditNum(HWND edit)
{
    char b[64] = "";
    GetWindowTextA(edit, b, sizeof b);
    return std::atof(b);
}

static LRESULT CALLBACK SetWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    const int nf = (int)(sizeof g_set_fields / sizeof g_set_fields[0]);
    switch(msg) {
        case WM_CREATE: {
            HINSTANCE inst = ((LPCREATESTRUCTA)lp)->hInstance;
            int y = 14;
            for(int i = 0; i < nf; i++) {
                if(i == SETF_DETB_CHANNEL) {
                    HWND div = CreateWindowA("STATIC", "Detector B (optional second channel):",
                        WS_CHILD | WS_VISIBLE, 16, y + 3, 340, 20, hwnd, nullptr, inst, nullptr);
                    SendMessageA(div, WM_SETFONT, (WPARAM)g_font_ui_bold, TRUE);
                    y += 26;
                    g_set_detb_enable = CreateWindowA("BUTTON", "Enable Detector B",
                        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                        16, y, 200, 24, hwnd, nullptr, inst, nullptr);
                    SendMessageA(g_set_detb_enable, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                    y += 32;
                }
                HWND lab = CreateWindowA("STATIC", g_set_fields[i].label,
                    WS_CHILD | WS_VISIBLE, 16, y + 3, 230, 20, hwnd, nullptr, inst, nullptr);
                g_set_fields[i].edit = CreateWindowA("EDIT", "",
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                    252, y, 110, 24, hwnd, nullptr, inst, nullptr);
                SendMessageA(lab, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                SendMessageA(g_set_fields[i].edit, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                y += 32;
            }
            g_set_known = CreateWindowA("BUTTON", "Report known (matched) peaks only",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                16, y, 340, 24, hwnd, nullptr, inst, nullptr);
            SendMessageA(g_set_known, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            y += 36;
            HWND ok = CreateWindowA("BUTTON", "Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                160, y, 96, 28, hwnd, (HMENU)(UINT_PTR)IDC_SET_OK, inst, nullptr);
            HWND ca = CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
                266, y, 96, 28, hwnd, (HMENU)(UINT_PTR)IDC_SET_CANCEL, inst, nullptr);
            SendMessageA(ok, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            SendMessageA(ca, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            // populate from the live method
            SetEditText(g_set_fields[0].edit, g_method.data_rate);
            SetEditText(g_set_fields[1].edit, (double)g_method.analysis_time);
            SetEditText(g_set_fields[2].edit, g_method.det.segment_width);
            SetEditText(g_set_fields[3].edit, (double)g_method.det.NandBtime);
            SetEditText(g_set_fields[4].edit, (double)g_method.det.NandBlen);
            SetEditText(g_set_fields[5].edit, (double)g_method.det.MinHeight);
            SetEditText(g_set_fields[6].edit, g_method.det.MinArea);
            SetEditText(g_set_fields[7].edit, g_method.det.peak_alg);
            SetEditText(g_set_fields[8].edit, g_method.detect_meth);
            SetEditText(g_set_fields[SETF_DETB_CHANNEL].edit,   g_method.det_b_adc_channel);
            SetEditText(g_set_fields[SETF_DETB_MINHEIGHT].edit, (double)g_method.det_b.MinHeight);
            SetEditText(g_set_fields[SETF_DETB_DETMETH].edit,   g_method.det_b_detect_meth);
            SendMessageA(g_set_known, BM_SETCHECK,
                         g_method.known_peaks ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessageA(g_set_detb_enable, BM_SETCHECK,
                         g_method.det_b_enabled ? BST_CHECKED : BST_UNCHECKED, 0);
            return 0;
        }
        case WM_COMMAND:
            if(LOWORD(wp) == IDC_SET_OK) {
                int dr = (int)GetEditNum(g_set_fields[0].edit);
                long at = (long)GetEditNum(g_set_fields[1].edit);
                int sw = (int)GetEditNum(g_set_fields[2].edit);
                if(dr <= 0 || at <= 0 || sw <= 0 ||
                   sw > DataQueue::MAX_Q * DataQueue::MULT_Q / 3) {
                    MessageBoxA(hwnd, "data rate, analysis time and segment width "
                                "must be positive (segment width <= 42)",
                                "WPEAK64 Settings", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                g_method.data_rate         = dr;
                g_method.analysis_time     = at;
                g_method.det.segment_width = sw;
                g_method.det.NandBtime     = (long)GetEditNum(g_set_fields[3].edit);
                g_method.det.NandBlen      = (long)GetEditNum(g_set_fields[4].edit);
                g_method.det.MinHeight     = (long)GetEditNum(g_set_fields[5].edit);
                g_method.det.MinArea       = GetEditNum(g_set_fields[6].edit);
                g_method.det.peak_alg      = (int)GetEditNum(g_set_fields[7].edit);
                g_method.detect_meth       = (int)GetEditNum(g_set_fields[8].edit);
                g_method.known_peaks       =
                    SendMessageA(g_set_known, BM_GETCHECK, 0, 0) == BST_CHECKED;
                g_method.det_b_enabled     =
                    SendMessageA(g_set_detb_enable, BM_GETCHECK, 0, 0) == BST_CHECKED;
                g_method.det_b_adc_channel = (int)GetEditNum(g_set_fields[SETF_DETB_CHANNEL].edit);
                g_method.det_b.MinHeight   = (long)GetEditNum(g_set_fields[SETF_DETB_MINHEIGHT].edit);
                g_method.det_b_detect_meth = (int)GetEditNum(g_set_fields[SETF_DETB_DETMETH].edit);
                ReprocessTrace();
                DestroyWindow(hwnd);
                return 0;
            }
            if(LOWORD(wp) == IDC_SET_CANCEL) { DestroyWindow(hwnd); return 0; }
            break;
        case WM_CLOSE:   DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_setwnd = nullptr;  return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ShowSettingsWindow(HINSTANCE inst)
{
    if(g_setwnd) { SetForegroundWindow(g_setwnd); return; }
    g_setwnd = CreateWindowA("WPEAK64_SET", "WPEAK64 - Settings",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                             CW_USEDEFAULT, CW_USEDEFAULT, 400,
                             14 + 12 * 32 + 26 + 32 + 36 + 28 + 60,
                             g_main, nullptr, inst, nullptr);
    ShowWindow(g_setwnd, SW_SHOW);
}

// ---- manual valve / relay control window ---------------------------------------
// Legacy Peak Works toolbar had direct manual buttons for sample/inject/purge
// (S/E/I), autozero (Az/AzB) and the heater (HEAT). This dialog is the
// equivalent: one toggle button per configured hardware line, driving a
// dedicated Hardware connection opened while the dialog is up. Disabled
// while an acquisition is running (that thread owns the hardware then).
struct ManualLine { std::string label; int line; HWND btn; bool state; };
static std::vector<ManualLine> g_manual_list;
static Hardware g_manual_hw;
static HWND g_manual_status = nullptr;

static void BuildManualList()
{
    g_manual_list.clear();
    auto add = [&](const std::string &label, int line) {
        if(line >= 0) g_manual_list.push_back({ label, line, nullptr, false });
    };
    const HardwareConfig &h = g_method.hw;
    add("Sample Valve",       h.sample_valve);
    add("Inject Valve",       h.inject_valve);
    add("Cal Valve",          h.cal_valve);
    add("Purge Valve",        h.purge_valve);
    add("Pump",               h.pump);
    add("Lamp",               h.lamp);
    add("Fan",                h.fan);
    add("Autozero",           h.autozero);
    add("Alarm High (test)",  h.alarm_high_line);
    add("Alarm Low (test)",   h.alarm_low_line);
    for(size_t i = 0; i < h.point_valves.size(); i++) {
        char lbl[32];
        std::snprintf(lbl, sizeof lbl, "Point Valve %zu", i + 1);
        add(lbl, h.point_valves[i]);
    }
    for(const TempZone &z : g_method.zones)
        add("Heater: " + z.name, z.heater_line);
}

static void UpdateManualStatus()
{
    if(!g_manual_status) return;
    char buf[220];
    std::snprintf(buf, sizeof buf, "Backend: %s   \xb7   %s   \xb7   %zu line(s)",
                  g_method.hw.backend.c_str(),
                  g_manual_hw.out ? "connected" : "not connected",
                  g_manual_list.size());
    SetWindowTextA(g_manual_status, buf);
}

static LRESULT CALLBACK ManualWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE: {
            HINSTANCE inst = ((LPCREATESTRUCTA)lp)->hInstance;
            int y = 14;
            for(size_t i = 0; i < g_manual_list.size(); i++) {
                ManualLine &m = g_manual_list[i];
                std::string lab = m.label + ":";
                CreateWindowA("STATIC", lab.c_str(), WS_CHILD | WS_VISIBLE,
                    16, y + 5, 190, 20, hwnd, nullptr, inst, nullptr);
                m.btn = CreateWindowA("BUTTON", m.state ? "ON" : "OFF",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    212, y, 80, 26, hwnd, (HMENU)(UINT_PTR)(IDC_MANUAL_BASE + i),
                    inst, nullptr);
                SendMessageA(m.btn, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                y += 30;
            }
            g_manual_status = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
                16, y + 10, 380, 20, hwnd, nullptr, inst, nullptr);
            SendMessageA(g_manual_status, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            UpdateManualStatus();
            HWND close = CreateWindowA("BUTTON", "Close",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                212, y + 36, 80, 28, hwnd, (HMENU)(UINT_PTR)IDC_SET_CANCEL, inst, nullptr);
            SendMessageA(close, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if(id == IDC_SET_CANCEL) { DestroyWindow(hwnd); return 0; }
            int idx = id - IDC_MANUAL_BASE;
            if(idx >= 0 && idx < (int)g_manual_list.size() && g_manual_hw.out) {
                ManualLine &m = g_manual_list[(size_t)idx];
                m.state = !m.state;
                g_manual_hw.out->Set(m.line, m.state);
                SetWindowTextA(m.btn, m.state ? "ON" : "OFF");
            }
            return 0;
        }
        case WM_CLOSE:   DestroyWindow(hwnd); return 0;
        case WM_DESTROY:
            CloseHardware(g_manual_hw);
            g_manualwnd = nullptr;
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ShowManualWindow(HINSTANCE inst)
{
    if(g_manualwnd) { SetForegroundWindow(g_manualwnd); return; }
    EnterCriticalSection(&g_acq.cs);
    bool running = g_acq.running;
    LeaveCriticalSection(&g_acq.cs);
    if(running) {
        MessageBoxA(g_main, "Cannot open Manual Control while an acquisition is running.",
                    "WPEAK64", MB_OK | MB_ICONWARNING);
        return;
    }
    BuildManualList();
    if(g_manual_list.empty()) {
        MessageBoxA(g_main, "No hardware lines are configured in [hardware]/[tempzone].",
                    "WPEAK64 Manual Control", MB_OK | MB_ICONINFORMATION);
        return;
    }
    std::string err;
    if(!OpenHardware(g_method.hw, g_method.zones, 1.0, g_manual_hw, err)) {
        MessageBoxA(g_main, err.c_str(), "WPEAK64 Manual Control", MB_OK | MB_ICONERROR);
        return;
    }
    int h = 14 + (int)g_manual_list.size() * 30 + 20 + 36 + 40;
    g_manualwnd = CreateWindowA("WPEAK64_MANUAL", "WPEAK64 - Manual Valve / Relay Control",
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                               CW_USEDEFAULT, CW_USEDEFAULT, 330, h,
                               g_main, nullptr, inst, nullptr);
    ShowWindow(g_manualwnd, SW_SHOW);
}

// ---- element table editor -------------------------------------------------------
// Legacy Edit Components dialog equivalent: define/edit components with a
// retention time, RT window, response factor and alarm limits, in a grid
// (SysListView32). Edits happen on a working copy (g_elem_edit); Apply &
// Close copies it into g_method.components/components_b and reprocesses the
// loaded chromatogram; Cancel discards it. Existing calibration (stand[]/
// cal[]) is preserved since edited components are copied, not reconstructed.
// Detector A and B (legacy "Component table 'Detector A'"/"'Detector B'")
// share this one editor -- g_elem_edit holds whichever is currently shown;
// g_elem_edit_other stashes the other one's pending edits across a toggle.
static std::vector<Component> g_elem_edit, g_elem_edit_other;
static bool g_el_showing_b = false;
static HWND g_elemeditwnd = nullptr;
static HWND g_el_list = nullptr, g_el_name = nullptr, g_el_rt = nullptr,
           g_el_window = nullptr, g_el_response = nullptr, g_el_ahigh = nullptr,
           g_el_alow = nullptr, g_el_active = nullptr, g_el_status = nullptr,
           g_el_dacch = nullptr, g_el_dacrange = nullptr,
           g_el_deta = nullptr, g_el_detb = nullptr;
static int g_el_selected = -1;

static void RefreshElList()
{
    if(!g_el_list) return;
    ListView_DeleteAllItems(g_el_list);
    for(size_t i = 0; i < g_elem_edit.size(); i++) {
        const Component &c = g_elem_edit[i];
        LVITEMA it = {};
        it.mask = LVIF_TEXT;
        it.iItem = (int)i;
        char name[64]; std::snprintf(name, sizeof name, "%s", c.name.c_str());
        it.pszText = name;
        int row = ListView_InsertItem(g_el_list, &it);
        char buf[32];
        std::snprintf(buf, sizeof buf, "%g", c.peak_rt);   ListView_SetItemText(g_el_list, row, 1, buf);
        std::snprintf(buf, sizeof buf, "%g", c.window);    ListView_SetItemText(g_el_list, row, 2, buf);
        std::snprintf(buf, sizeof buf, "%g", c.response);  ListView_SetItemText(g_el_list, row, 3, buf);
        std::snprintf(buf, sizeof buf, "%g", c.alarm_high);ListView_SetItemText(g_el_list, row, 4, buf);
        std::snprintf(buf, sizeof buf, "%g", c.alarm_low); ListView_SetItemText(g_el_list, row, 5, buf);
        if(c.dac_channel >= 0) std::snprintf(buf, sizeof buf, "%d", c.dac_channel);
        else                   std::snprintf(buf, sizeof buf, "-");
        ListView_SetItemText(g_el_list, row, 6, buf);
        std::snprintf(buf, sizeof buf, "%g", c.dac_range); ListView_SetItemText(g_el_list, row, 7, buf);
        ListView_SetItemText(g_el_list, row, 8, (LPSTR)(c.active_yn ? "Yes" : "No"));
    }
    if(g_el_status) {
        char buf[96];
        std::snprintf(buf, sizeof buf, "%zu component(s) pending \xb7 Apply to update the loaded method",
                      g_elem_edit.size());
        SetWindowTextA(g_el_status, buf);
    }
}

static void SetElEditNum(HWND edit, double v)
{
    char b[32]; std::snprintf(b, sizeof b, "%g", v);
    SetWindowTextA(edit, b);
}
static double GetElEditNum(HWND edit)
{
    char b[64] = ""; GetWindowTextA(edit, b, sizeof b);
    return std::atof(b);
}

static void PopulateElFields(int idx)
{
    if(idx < 0 || idx >= (int)g_elem_edit.size()) return;
    const Component &c = g_elem_edit[(size_t)idx];
    SetWindowTextA(g_el_name, c.name.c_str());
    SetElEditNum(g_el_rt, c.peak_rt);
    SetElEditNum(g_el_window, c.window);
    SetElEditNum(g_el_response, c.response);
    SetElEditNum(g_el_ahigh, c.alarm_high);
    SetElEditNum(g_el_alow, c.alarm_low);
    SetElEditNum(g_el_dacch, c.dac_channel);
    SetElEditNum(g_el_dacrange, c.dac_range);
    SendMessageA(g_el_active, BM_SETCHECK, c.active_yn ? BST_CHECKED : BST_UNCHECKED, 0);
    g_el_selected = idx;
}

static void ClearElFields()
{
    SetWindowTextA(g_el_name, "");
    SetElEditNum(g_el_rt, 0);
    SetElEditNum(g_el_window, 5);
    SetElEditNum(g_el_response, 0);
    SetElEditNum(g_el_ahigh, 0);
    SetElEditNum(g_el_alow, 0);
    SetElEditNum(g_el_dacch, -1);
    SetElEditNum(g_el_dacrange, 0);
    SendMessageA(g_el_active, BM_SETCHECK, BST_CHECKED, 0);
    g_el_selected = -1;
    ListView_SetItemState(g_el_list, -1, 0, LVIS_SELECTED);
}

// reads the edit fields into c, leaving c.stand[]/c.cal[] (calibration) untouched
static bool ReadElFieldsInto(HWND hwnd, Component &c)
{
    char name[64] = "";
    GetWindowTextA(g_el_name, name, sizeof name);
    if(!name[0]) {
        MessageBoxA(hwnd, "Component name is required.", "WPEAK64 Element Table",
                    MB_OK | MB_ICONWARNING);
        return false;
    }
    c.name        = name;
    c.peak_rt     = GetElEditNum(g_el_rt);
    c.window      = GetElEditNum(g_el_window);
    c.response    = GetElEditNum(g_el_response);
    c.alarm_high  = GetElEditNum(g_el_ahigh);
    c.alarm_low   = GetElEditNum(g_el_alow);
    c.dac_channel = (int)GetElEditNum(g_el_dacch);
    c.dac_range   = GetElEditNum(g_el_dacrange);
    c.active_yn   = SendMessageA(g_el_active, BM_GETCHECK, 0, 0) == BST_CHECKED;
    return true;
}

static void ShowCalibrationStandards(HINSTANCE inst, HWND owner);

static LRESULT CALLBACK ElemEditWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE: {
            HINSTANCE inst = ((LPCREATESTRUCTA)lp)->hInstance;
            g_el_deta = CreateWindowA("BUTTON", "Detector A components",
                WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                12, 12, 170, 20, hwnd, (HMENU)(UINT_PTR)IDC_EL_DETA, inst, nullptr);
            g_el_detb = CreateWindowA("BUTTON", "Detector B components",
                WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                190, 12, 170, 20, hwnd, (HMENU)(UINT_PTR)IDC_EL_DETB, inst, nullptr);
            SendMessageA(g_el_deta, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            SendMessageA(g_el_detb, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            SendMessageA(g_el_showing_b ? g_el_detb : g_el_deta, BM_SETCHECK, BST_CHECKED, 0);
            g_el_list = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                12, 38, 658, 200, hwnd, (HMENU)(UINT_PTR)IDC_EL_LIST, inst, nullptr);
            ListView_SetExtendedListViewStyle(g_el_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            struct { const char *t; int w; } cols[] = {
                { "Name", 120 }, { "RT (s)", 55 }, { "Window (s)", 70 },
                { "Response", 70 }, { "AlarmHi", 55 }, { "AlarmLo", 55 },
                { "DAC Ch", 55 }, { "DAC Range", 70 }, { "Active", 50 },
            };
            for(int i = 0; i < 9; i++) {
                LVCOLUMNA col = {};
                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.cx = cols[i].w;
                col.pszText = (LPSTR)cols[i].t;
                ListView_InsertColumn(g_el_list, i, &col);
            }
            SendMessageA(g_el_list, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            int y = 250;
            auto label = [&](const char *text, int x, int yy, int w) {
                CreateWindowA("STATIC", text, WS_CHILD | WS_VISIBLE, x, yy, w, 18,
                             hwnd, nullptr, inst, nullptr);
            };
            label("Name", 12, y + 3, 60);
            g_el_name = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 200, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_NAME, inst, nullptr);
            y += 28;
            label("RT (s)", 12, y + 3, 60);
            g_el_rt = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_RT, inst, nullptr);
            label("Window (s)", 172, y + 3, 70);
            g_el_window = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                250, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_WINDOW, inst, nullptr);
            y += 28;
            label("Response", 12, y + 3, 60);
            g_el_response = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_RESPONSE, inst, nullptr);
            y += 28;
            label("Alarm High", 12, y + 3, 70);
            g_el_ahigh = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_AHIGH, inst, nullptr);
            label("Alarm Low", 172, y + 3, 70);
            g_el_alow = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                250, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_ALOW, inst, nullptr);
            y += 28;
            label("DAC Channel", 12, y + 3, 90);
            g_el_dacch = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                110, y, 60, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_DACCH, inst, nullptr);
            label("DAC Range", 190, y + 3, 80);
            g_el_dacrange = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                278, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_DACRANGE, inst, nullptr);
            y += 30;
            g_el_active = CreateWindowA("BUTTON", "Active",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                12, y, 90, 22, hwnd, (HMENU)(UINT_PTR)IDC_EL_ACTIVE, inst, nullptr);
            y += 32;

            HWND badd = CreateWindowA("BUTTON", "Add New", WS_CHILD | WS_VISIBLE,
                12, y, 110, 26, hwnd, (HMENU)(UINT_PTR)IDC_EL_ADD, inst, nullptr);
            HWND bupd = CreateWindowA("BUTTON", "Update Selected", WS_CHILD | WS_VISIBLE,
                128, y, 130, 26, hwnd, (HMENU)(UINT_PTR)IDC_EL_UPDATE, inst, nullptr);
            HWND bdel = CreateWindowA("BUTTON", "Delete Selected", WS_CHILD | WS_VISIBLE,
                264, y, 130, 26, hwnd, (HMENU)(UINT_PTR)IDC_EL_DELETE, inst, nullptr);
            HWND bcalib = CreateWindowA("BUTTON", "Calibration Standards...", WS_CHILD | WS_VISIBLE,
                404, y, 190, 26, hwnd, (HMENU)(UINT_PTR)IDC_EL_CALIB, inst, nullptr);
            y += 34;
            g_el_status = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
                12, y, 658, 18, hwnd, nullptr, inst, nullptr);
            y += 22;
            HWND note = CreateWindowA("STATIC",
                "Calibration Standards... sets std1..std8 (select a component first); measured\n"
                "responses come from Run > Start Calibration. DAC Channel/Range send a 0-Vref\n"
                "analog output proportional to concentration (see [hardware] dac_i2c_addrs).",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 12, y, 658, 52, hwnd, nullptr, inst, nullptr);
            y += 60;
            HWND bapply = CreateWindowA("BUTTON", "Apply && Close",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                432, y, 110, 28, hwnd, (HMENU)(UINT_PTR)IDC_EL_APPLY, inst, nullptr);
            HWND bcancel = CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
                548, y, 100, 28, hwnd, (HMENU)(UINT_PTR)IDC_EL_CANCEL, inst, nullptr);

            for(HWND h : { g_el_name, g_el_rt, g_el_window, g_el_response, g_el_ahigh,
                          g_el_alow, g_el_dacch, g_el_dacrange, g_el_active,
                          badd, bupd, bdel, bcalib, note, bapply, bcancel })
                SendMessageA(h, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            RefreshElList();
            ClearElFields();
            return 0;
        }
        case WM_NOTIFY: {
            LPNMHDR nh = (LPNMHDR)lp;
            if(nh->idFrom == IDC_EL_LIST && nh->code == LVN_ITEMCHANGED) {
                LPNMLISTVIEW nlv = (LPNMLISTVIEW)lp;
                if(nlv->uNewState & LVIS_SELECTED)
                    PopulateElFields(nlv->iItem);
            }
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if(id == IDC_EL_ADD) {
                Component c;
                if(ReadElFieldsInto(hwnd, c)) {
                    g_elem_edit.push_back(c);
                    RefreshElList();
                    ListView_SetItemState(g_el_list, (int)g_elem_edit.size() - 1,
                                          LVIS_SELECTED, LVIS_SELECTED);
                    PopulateElFields((int)g_elem_edit.size() - 1);
                }
                return 0;
            }
            if(id == IDC_EL_UPDATE) {
                if(g_el_selected >= 0 && g_el_selected < (int)g_elem_edit.size()) {
                    if(ReadElFieldsInto(hwnd, g_elem_edit[(size_t)g_el_selected]))
                        RefreshElList();
                }
                else
                    MessageBoxA(hwnd, "Select a component in the list first.",
                                "WPEAK64 Element Table", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            if(id == IDC_EL_DELETE) {
                if(g_el_selected >= 0 && g_el_selected < (int)g_elem_edit.size()) {
                    g_elem_edit.erase(g_elem_edit.begin() + g_el_selected);
                    RefreshElList();
                    ClearElFields();
                }
                return 0;
            }
            if(id == IDC_EL_APPLY) {
                if(g_el_showing_b) { g_method.components_b = g_elem_edit; g_method.components = g_elem_edit_other; }
                else                { g_method.components   = g_elem_edit; g_method.components_b = g_elem_edit_other; }
                ReprocessTrace();
                DestroyWindow(hwnd);
                return 0;
            }
            if(id == IDC_EL_CANCEL) { DestroyWindow(hwnd); return 0; }
            if((id == IDC_EL_DETA || id == IDC_EL_DETB) && HIWORD(wp) == BN_CLICKED) {
                bool want_b = id == IDC_EL_DETB;
                if(want_b != g_el_showing_b) {
                    std::swap(g_elem_edit, g_elem_edit_other);
                    g_el_showing_b = want_b;
                    RefreshElList();
                    ClearElFields();
                }
                return 0;
            }
            if(id == IDC_EL_CALIB) {
                if(g_el_selected >= 0 && g_el_selected < (int)g_elem_edit.size())
                    ShowCalibrationStandards((HINSTANCE)GetWindowLongPtrA(hwnd, GWLP_HINSTANCE), hwnd);
                else
                    MessageBoxA(hwnd, "Select a component in the list first.",
                                "WPEAK64 Element Table", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            return 0;
        }
        case WM_CLOSE:   DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_elemeditwnd = nullptr; return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

// ---- calibration standards dialog (legacy STND_DLG.CPP) -------------------------
// Edits std1..std8 (the standard concentrations a calibration run measures
// against) for the component currently selected in the Element Table editor.
// Measured responses (Component::cal[]) are read-only here -- they only come
// from an actual Run > Start Calibration or a loaded calibration file.
static HWND g_calwnd = nullptr;
static HWND g_cal_std[STAND_NUM64] = {};
static HWND g_cal_resp[STAND_NUM64] = {};
static int  g_cal_target = -1;   // index into g_elem_edit being edited

static LRESULT CALLBACK CalStdWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE: {
            HINSTANCE inst = ((LPCREATESTRUCTA)lp)->hInstance;
            const int ids[STAND_NUM64] = { IDC_CAL_STD1, IDC_CAL_STD2, IDC_CAL_STD3, IDC_CAL_STD4,
                                           IDC_CAL_STD5, IDC_CAL_STD6, IDC_CAL_STD7, IDC_CAL_STD8 };
            int y = 46;
            CreateWindowA("STATIC", "Std #", WS_CHILD | WS_VISIBLE, 12, y, 50, 18, hwnd, nullptr, inst, nullptr);
            CreateWindowA("STATIC", "Concentration", WS_CHILD | WS_VISIBLE, 70, y, 100, 18, hwnd, nullptr, inst, nullptr);
            CreateWindowA("STATIC", "Measured response", WS_CHILD | WS_VISIBLE, 210, y, 150, 18, hwnd, nullptr, inst, nullptr);
            y += 22;
            for(int i = 0; i < STAND_NUM64; i++) {
                char lbl[16]; std::snprintf(lbl, sizeof lbl, "Std %d", i + 1);
                HWND l = CreateWindowA("STATIC", lbl, WS_CHILD | WS_VISIBLE, 12, y + 3, 50, 18, hwnd, nullptr, inst, nullptr);
                g_cal_std[i] = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                    70, y, 100, 22, hwnd, (HMENU)(UINT_PTR)ids[i], inst, nullptr);
                g_cal_resp[i] = CreateWindowA("STATIC", "(not calibrated)",
                    WS_CHILD | WS_VISIBLE, 210, y + 3, 200, 18, hwnd, nullptr, inst, nullptr);
                SendMessageA(l, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                SendMessageA(g_cal_std[i], WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                SendMessageA(g_cal_resp[i], WM_SETFONT, (WPARAM)g_font_ui, TRUE);
                y += 28;
            }
            y += 8;
            HWND note = CreateWindowA("STATIC",
                "Measured response comes from Run > Start Calibration (standard N) or a\n"
                "loaded calibration file -- it can't be typed in here.",
                WS_CHILD | WS_VISIBLE, 12, y, 400, 32, hwnd, nullptr, inst, nullptr);
            y += 40;
            HWND bok = CreateWindowA("BUTTON", "Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                140, y, 96, 28, hwnd, (HMENU)(UINT_PTR)IDC_CAL_APPLY, inst, nullptr);
            HWND bca = CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
                246, y, 96, 28, hwnd, (HMENU)(UINT_PTR)IDC_CAL_CANCEL, inst, nullptr);
            SendMessageA(note, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            SendMessageA(bok, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
            SendMessageA(bca, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            // populate from the target component
            if(g_cal_target >= 0 && g_cal_target < (int)g_elem_edit.size()) {
                const Component &c = g_elem_edit[(size_t)g_cal_target];
                char title[128];
                std::snprintf(title, sizeof title, "WPEAK64 - Calibration Standards - %s", c.name.c_str());
                SetWindowTextA(hwnd, title);
                for(int i = 0; i < STAND_NUM64; i++) {
                    char b[32]; std::snprintf(b, sizeof b, "%g", c.stand[i]);
                    SetWindowTextA(g_cal_std[i], b);
                    if(c.cal[i].valid) {
                        std::snprintf(b, sizeof b, "%g @ %g", c.cal[i].conc, c.cal[i].resp);
                        SetWindowTextA(g_cal_resp[i], b);
                    }
                }
            }
            return 0;
        }
        case WM_COMMAND:
            if(LOWORD(wp) == IDC_CAL_APPLY) {
                if(g_cal_target >= 0 && g_cal_target < (int)g_elem_edit.size()) {
                    Component &c = g_elem_edit[(size_t)g_cal_target];
                    for(int i = 0; i < STAND_NUM64; i++) {
                        char b[64] = "";
                        GetWindowTextA(g_cal_std[i], b, sizeof b);
                        c.stand[i] = std::atof(b);
                    }
                    RefreshElList();
                }
                DestroyWindow(hwnd);
                return 0;
            }
            if(LOWORD(wp) == IDC_CAL_CANCEL) { DestroyWindow(hwnd); return 0; }
            break;
        case WM_CLOSE:   DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_calwnd = nullptr; return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ShowCalibrationStandards(HINSTANCE inst, HWND owner)
{
    if(g_calwnd) { SetForegroundWindow(g_calwnd); return; }
    g_cal_target = g_el_selected;
    g_calwnd = CreateWindowA("WPEAK64_CALSTD", "WPEAK64 - Calibration Standards",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                             CW_USEDEFAULT, CW_USEDEFAULT, 430, 46 + 22 + STAND_NUM64 * 28 + 8 + 40 + 60,
                             owner, nullptr, inst, nullptr);
    ShowWindow(g_calwnd, SW_SHOW);
}

static void ShowElementEditor(HINSTANCE inst)
{
    if(g_elemeditwnd) { SetForegroundWindow(g_elemeditwnd); return; }
    g_el_showing_b = false;
    g_elem_edit       = g_method.components;    // working copy; stand[]/cal[] preserved
    g_elem_edit_other = g_method.components_b;
    g_elemeditwnd = CreateWindowA("WPEAK64_ELEMEDIT", "WPEAK64 - Element Table (Edit Components)",
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 700, 672,
                                  g_main, nullptr, inst, nullptr);
    ShowWindow(g_elemeditwnd, SW_SHOW);
}

static std::string FileDialog(HWND hwnd, bool save, const char *filter, const char *defext)
{
    char buf[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner   = hwnd;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = sizeof buf;
    ofn.lpstrDefExt = defext;
    ofn.Flags       = save ? OFN_OVERWRITEPROMPT : (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    BOOL ok = save ? GetSaveFileNameA(&ofn) : GetOpenFileNameA(&ofn);
    return ok ? std::string(buf) : std::string();
}

static void Reanalyze(HWND hwnd)
{
    std::string err;
    if(!RunAnalysis(hwnd, err))
        MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
    InvalidateRect(hwnd, nullptr, FALSE);
}

// ---- valve / port schedule editor (legacy Method dialog's R1A/R1B/X3/X4/X5) -----
// Edits Method::hw.aux_relays: named GPIO lines that turn on/off at fixed
// times (seconds since the run starts), independent of the phase-driven
// sample/inject/purge valves -- for auxiliary triggers, secondary valves,
// data logging pulses, etc. Same edit-in-place-fields-below-a-list pattern
// as the Element Table editor.
static std::vector<AuxRelay> g_relay_edit;
static HWND g_rl_wnd = nullptr;
static HWND g_rl_list = nullptr, g_rl_name = nullptr, g_rl_line = nullptr,
           g_rl_ontime = nullptr, g_rl_offtime = nullptr, g_rl_status = nullptr;
static int g_rl_selected = -1;

static void RefreshRlList()
{
    if(!g_rl_list) return;
    ListView_DeleteAllItems(g_rl_list);
    for(size_t i = 0; i < g_relay_edit.size(); i++) {
        const AuxRelay &r = g_relay_edit[i];
        LVITEMA it = {};
        it.mask = LVIF_TEXT;
        it.iItem = (int)i;
        char name[64]; std::snprintf(name, sizeof name, "%s", r.name.c_str());
        it.pszText = name;
        int row = ListView_InsertItem(g_rl_list, &it);
        char buf[32];
        std::snprintf(buf, sizeof buf, "%d", r.line); ListView_SetItemText(g_rl_list, row, 1, buf);
        if(r.on_time_s  >= 0) std::snprintf(buf, sizeof buf, "%lds", r.on_time_s);
        else                  std::snprintf(buf, sizeof buf, "-");
        ListView_SetItemText(g_rl_list, row, 2, buf);
        if(r.off_time_s >= 0) std::snprintf(buf, sizeof buf, "%lds", r.off_time_s);
        else                  std::snprintf(buf, sizeof buf, "-");
        ListView_SetItemText(g_rl_list, row, 3, buf);
    }
    if(g_rl_status) {
        char buf[96];
        std::snprintf(buf, sizeof buf, "%zu relay(s) pending \xb7 Apply to update the loaded method",
                      g_relay_edit.size());
        SetWindowTextA(g_rl_status, buf);
    }
}

static void PopulateRlFields(int idx)
{
    if(idx < 0 || idx >= (int)g_relay_edit.size()) return;
    const AuxRelay &r = g_relay_edit[(size_t)idx];
    char b[32];
    SetWindowTextA(g_rl_name, r.name.c_str());
    std::snprintf(b, sizeof b, "%d", r.line);      SetWindowTextA(g_rl_line, b);
    std::snprintf(b, sizeof b, "%ld", r.on_time_s); SetWindowTextA(g_rl_ontime, b);
    std::snprintf(b, sizeof b, "%ld", r.off_time_s);SetWindowTextA(g_rl_offtime, b);
    g_rl_selected = idx;
}

static void ClearRlFields()
{
    SetWindowTextA(g_rl_name, "");
    SetWindowTextA(g_rl_line, "-1");
    SetWindowTextA(g_rl_ontime, "-1");
    SetWindowTextA(g_rl_offtime, "-1");
    g_rl_selected = -1;
    ListView_SetItemState(g_rl_list, -1, 0, LVIS_SELECTED);
}

static bool ReadRlFieldsInto(HWND hwnd, AuxRelay &r)
{
    char name[64] = "";
    GetWindowTextA(g_rl_name, name, sizeof name);
    if(!name[0]) {
        MessageBoxA(hwnd, "Relay name is required (e.g. R1A, X3).", "WPEAK64 Valve / Port Schedule",
                    MB_OK | MB_ICONWARNING);
        return false;
    }
    char b[32] = "";
    r.name = name;
    GetWindowTextA(g_rl_line, b, sizeof b);      r.line       = std::atoi(b);
    GetWindowTextA(g_rl_ontime, b, sizeof b);    r.on_time_s  = std::atol(b);
    GetWindowTextA(g_rl_offtime, b, sizeof b);   r.off_time_s = std::atol(b);
    return true;
}

static LRESULT CALLBACK RelayEditWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE: {
            HINSTANCE inst = ((LPCREATESTRUCTA)lp)->hInstance;
            g_rl_list = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                12, 12, 460, 180, hwnd, (HMENU)(UINT_PTR)IDC_RL_LIST, inst, nullptr);
            ListView_SetExtendedListViewStyle(g_rl_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            struct { const char *t; int w; } cols[] = {
                { "Name", 140 }, { "GPIO Line", 100 }, { "On Time", 105 }, { "Off Time", 105 },
            };
            for(int i = 0; i < 4; i++) {
                LVCOLUMNA col = {};
                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.cx = cols[i].w;
                col.pszText = (LPSTR)cols[i].t;
                ListView_InsertColumn(g_rl_list, i, &col);
            }
            SendMessageA(g_rl_list, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            int y = 204;
            auto label = [&](const char *text, int x, int yy, int w) {
                CreateWindowA("STATIC", text, WS_CHILD | WS_VISIBLE, x, yy, w, 18,
                             hwnd, nullptr, inst, nullptr);
            };
            label("Name", 12, y + 3, 60);
            g_rl_name = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 100, 22, hwnd, (HMENU)(UINT_PTR)IDC_RL_NAME, inst, nullptr);
            label("GPIO Line", 194, y + 3, 70);
            g_rl_line = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                272, y, 80, 22, hwnd, (HMENU)(UINT_PTR)IDC_RL_LINE, inst, nullptr);
            y += 28;
            label("On Time (s)", 12, y + 3, 70);
            g_rl_ontime = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, y, 100, 22, hwnd, (HMENU)(UINT_PTR)IDC_RL_ONTIME, inst, nullptr);
            label("Off Time (s)", 194, y + 3, 75);
            g_rl_offtime = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                272, y, 100, 22, hwnd, (HMENU)(UINT_PTR)IDC_RL_OFFTIME, inst, nullptr);
            y += 32;

            HWND badd = CreateWindowA("BUTTON", "Add New", WS_CHILD | WS_VISIBLE,
                12, y, 100, 26, hwnd, (HMENU)(UINT_PTR)IDC_RL_ADD, inst, nullptr);
            HWND bupd = CreateWindowA("BUTTON", "Update Selected", WS_CHILD | WS_VISIBLE,
                118, y, 130, 26, hwnd, (HMENU)(UINT_PTR)IDC_RL_UPDATE, inst, nullptr);
            HWND bdel = CreateWindowA("BUTTON", "Delete Selected", WS_CHILD | WS_VISIBLE,
                254, y, 130, 26, hwnd, (HMENU)(UINT_PTR)IDC_RL_DELETE, inst, nullptr);
            y += 34;
            g_rl_status = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
                12, y, 460, 18, hwnd, nullptr, inst, nullptr);
            y += 22;
            HWND note = CreateWindowA("STATIC",
                "Times are seconds since run start (0s = EQUILIBRATE).\n"
                "-1 = disabled; Off Time -1 stays on once triggered.",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 12, y, 460, 34, hwnd, nullptr, inst, nullptr);
            y += 44;
            HWND bapply = CreateWindowA("BUTTON", "Apply && Close",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                234, y, 110, 28, hwnd, (HMENU)(UINT_PTR)IDC_RL_APPLY, inst, nullptr);
            HWND bcancel = CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
                350, y, 100, 28, hwnd, (HMENU)(UINT_PTR)IDC_RL_CANCEL, inst, nullptr);

            for(HWND h : { g_rl_name, g_rl_line, g_rl_ontime, g_rl_offtime,
                          badd, bupd, bdel, note, bapply, bcancel })
                SendMessageA(h, WM_SETFONT, (WPARAM)g_font_ui, TRUE);

            RefreshRlList();
            ClearRlFields();
            return 0;
        }
        case WM_NOTIFY: {
            LPNMHDR nh = (LPNMHDR)lp;
            if(nh->idFrom == IDC_RL_LIST && nh->code == LVN_ITEMCHANGED) {
                LPNMLISTVIEW nlv = (LPNMLISTVIEW)lp;
                if(nlv->uNewState & LVIS_SELECTED)
                    PopulateRlFields(nlv->iItem);
            }
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if(id == IDC_RL_ADD) {
                AuxRelay r;
                if(ReadRlFieldsInto(hwnd, r)) {
                    g_relay_edit.push_back(r);
                    RefreshRlList();
                    ListView_SetItemState(g_rl_list, (int)g_relay_edit.size() - 1,
                                          LVIS_SELECTED, LVIS_SELECTED);
                    PopulateRlFields((int)g_relay_edit.size() - 1);
                }
                return 0;
            }
            if(id == IDC_RL_UPDATE) {
                if(g_rl_selected >= 0 && g_rl_selected < (int)g_relay_edit.size()) {
                    if(ReadRlFieldsInto(hwnd, g_relay_edit[(size_t)g_rl_selected]))
                        RefreshRlList();
                }
                else
                    MessageBoxA(hwnd, "Select a relay in the list first.",
                                "WPEAK64 Valve / Port Schedule", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            if(id == IDC_RL_DELETE) {
                if(g_rl_selected >= 0 && g_rl_selected < (int)g_relay_edit.size()) {
                    g_relay_edit.erase(g_relay_edit.begin() + g_rl_selected);
                    RefreshRlList();
                    ClearRlFields();
                }
                return 0;
            }
            if(id == IDC_RL_APPLY) {
                g_method.hw.aux_relays = g_relay_edit;
                DestroyWindow(hwnd);
                return 0;
            }
            if(id == IDC_RL_CANCEL) { DestroyWindow(hwnd); return 0; }
            return 0;
        }
        case WM_CLOSE:   DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_rl_wnd = nullptr; return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ShowRelayEditor(HINSTANCE inst)
{
    if(g_rl_wnd) { SetForegroundWindow(g_rl_wnd); return; }
    g_relay_edit = g_method.hw.aux_relays;
    g_rl_wnd = CreateWindowA("WPEAK64_RELAYEDIT", "WPEAK64 - Valve / Port Schedule",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                             CW_USEDEFAULT, CW_USEDEFAULT, 500, 470,
                             g_main, nullptr, inst, nullptr);
    ShowWindow(g_rl_wnd, SW_SHOW);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HINSTANCE inst = (HINSTANCE)GetWindowLongPtrA(hwnd, GWLP_HINSTANCE);
    switch(msg) {
        case WM_COMMAND:
            switch(LOWORD(wp)) {
                case IDM_OPEN_DATA: {
                    std::string p = FileDialog(hwnd, false,
                        "Chromatogram CSV (*.csv)\0*.csv\0All files\0*.*\0", "csv");
                    if(!p.empty()) { g_data_path = p; Reanalyze(hwnd); }
                    return 0;
                }
                case IDM_OPEN_METHOD: {
                    std::string p = FileDialog(hwnd, false,
                        "Method file (*.ini)\0*.ini\0All files\0*.*\0", "ini");
                    if(!p.empty()) { g_method_path = p; Reanalyze(hwnd); }
                    return 0;
                }
                case IDM_OPEN_CAL: {
                    std::string p = FileDialog(hwnd, false,
                        "Calibration file (*.ini)\0*.ini\0All files\0*.*\0", "ini");
                    if(!p.empty()) { g_cal_path = p; Reanalyze(hwnd); }
                    return 0;
                }
                case IDM_SAVE_METHOD: {
                    std::string p = FileDialog(hwnd, true,
                        "Method file (*.ini)\0*.ini\0", "ini");
                    if(!p.empty()) {
                        std::string err;
                        if(!SaveMethod(p, g_method, err))
                            MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
                        else
                            g_method_path = p;
                    }
                    return 0;
                }
                case IDM_SETTINGS:
                    ShowSettingsWindow(inst);
                    return 0;
                case IDM_MANUAL:
                    ShowManualWindow(inst);
                    return 0;
                case IDM_ELEMENTS:
                    ShowElementEditor(inst);
                    return 0;
                case IDM_RELAYS:
                    ShowRelayEditor(inst);
                    return 0;
                case IDM_ABOUT:
                    MessageBoxA(hwnd,
                        "GC301c Gas Chromatograph\n"
                        "WPEAK64 - 64-bit Windows port of WPEAK 2.4.43\n\n"
                        "Peak detection (positive + negative), component\n"
                        "identification, multipoint calibration, acquisition\n"
                        "via ADS1115/GPIO, alarms, TWA/STEL reporting.\n\n"
                        "PID Analyzers (HNU Technology)",
                        "About WPEAK64", MB_OK | MB_ICONINFORMATION);
                    return 0;
                case IDM_EXPORT: {
                    std::string p = FileDialog(hwnd, true,
                        "Report CSV (*.csv)\0*.csv\0", "csv");
                    if(!p.empty()) {
                        std::string err;
                        if(!WriteReportCsv(p, g_rows, g_method, g_noise, g_baseline, err))
                            MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
                        else if(g_has_b) {
                            // sibling "<name>_b.csv" next to the detector A report
                            std::string pb = p;
                            size_t dot = pb.find_last_of('.');
                            if(dot == std::string::npos) pb += "_b";
                            else pb.insert(dot, "_b");
                            if(!WriteReportCsv(pb, g_rows_b, g_method.AsDetectorB(),
                                               g_noise_b, g_baseline_b, err))
                                MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
                        }
                    }
                    return 0;
                }
                case IDM_PRINT_REPORT: {
                    // "Print" the modern way: render a self-contained,
                    // printable HTML report (table + SVG chromatogram) and
                    // open it in the default browser -- Ctrl+P from there
                    // reaches any installed printer or "Save as PDF",
                    // without WPEAK64 driving a printer DC directly.
                    char tmp[MAX_PATH];
                    GetTempPathA(sizeof tmp, tmp);
                    std::string p = std::string(tmp) + "wpeak64_report.html";
                    std::string err;
                    if(!WriteReportHtml(p, g_trace, g_rows, g_method, g_noise, g_baseline, err)) {
                        MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    if(g_has_b) {
                        std::string pb = std::string(tmp) + "wpeak64_report_b.html";
                        std::string berr;
                        WriteReportHtml(pb, g_trace_b, g_rows_b, g_method.AsDetectorB(),
                                        g_noise_b, g_baseline_b, berr);
                    }
                    ShellExecuteA(hwnd, "open", p.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                    return 0;
                }
                case IDM_RUN_START:
                    // live trace draws in the main window's graph pane
                    if(StartAcquisition(hwnd, false, 0))
                        InvalidateRect(hwnd, nullptr, FALSE);
                    return 0;
                case IDM_RUN_CAL:
                    if(StartAcquisition(hwnd, true, 1))
                        InvalidateRect(hwnd, nullptr, FALSE);
                    return 0;
                case IDM_RUN_ABORT:
                    EnterCriticalSection(&g_acq.cs);
                    g_acq.abort_req = true;
                    LeaveCriticalSection(&g_acq.cs);
                    return 0;
                case IDM_EXIT:     DestroyWindow(hwnd);  return 0;
            }
            break;
        case WM_ACQ_DONE: {
            EnterCriticalSection(&g_acq.cs);
            bool ok = g_acq.ok;
            std::string err = g_acq.err;
            AcquireResult res = g_acq.result;
            bool was_cal = g_acq.is_cal;
            LeaveCriticalSection(&g_acq.cs);
            if(!ok) {
                if(err != "aborted")
                    MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
            }
            else {
                g_trace    = res.trace;
                g_rows     = res.rows;
                g_noise    = res.noise;
                g_baseline = res.baseline;
                g_has_b     = res.has_b;
                g_trace_b   = res.trace_b;
                g_rows_b    = res.rows_b;
                g_noise_b   = res.noise_b;
                g_baseline_b = res.baseline_b;
                char title[256];
                std::snprintf(title, sizeof title,
                              "GC301c - WPEAK64 - acquired %s%s",
                              was_cal ? "calibration run" : "run",
                              (res.alarm_state | res.alarm_state_b) ? "  *** ALARM ***" : "");
                SetWindowTextA(hwnd, title);
            }
            InvalidateRect(hwnd, nullptr, FALSE);   // element table is part of g_main now
            return 0;
        }
        case WM_LBUTTONDOWN: {   // toolbar icon clicks
            int id = ToolbarButtonAt((int)(short)LOWORD(lp), (int)(short)HIWORD(lp));
            if(id) PostMessageA(hwnd, WM_COMMAND, (WPARAM)id, 0);
            return 0;
        }
        case WM_TIMER:   // repaint the live trace while a run is in progress
            EnterCriticalSection(&g_acq.cs);
            {
                bool running = g_acq.running;
                LeaveCriticalSection(&g_acq.cs);
                if(running) InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_PAINT: DoubleBufferPaint(hwnd, PaintMain); return 0;
        case WM_SIZE:  InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

// command line: optional [run.csv] [method.ini] in any order (.ini = method)
static void ParseCmdLine(LPSTR cmd)
{
    std::string s = cmd ? cmd : "";
    size_t pos = 0;
    while(pos < s.size()) {
        while(pos < s.size() && s[pos] == ' ') pos++;
        if(pos >= s.size()) break;
        std::string tok;
        if(s[pos] == '"') {
            size_t end = s.find('"', pos + 1);
            tok = s.substr(pos + 1, end == std::string::npos ? std::string::npos : end - pos - 1);
            pos = end == std::string::npos ? s.size() : end + 1;
        }
        else {
            size_t end = s.find(' ', pos);
            tok = s.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
            pos = end == std::string::npos ? s.size() : end;
        }
        if(tok.size() > 4 && !_stricmp(tok.c_str() + tok.size() - 4, ".ini"))
            g_method_path = tok;
        else if(!_stricmp(tok.c_str(), "/autorun"))
            g_autorun = true;
        else if(!tok.empty())
            g_data_path = tok;
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nShow)
{
    InitializeCriticalSection(&g_acq.cs);
    CreateFonts();
    LoadToolbarBitmaps(hInst);
    ParseCmdLine(lpCmdLine);
    std::string err;
    if(!RunAnalysis(nullptr, err)) {
        MessageBoxA(nullptr, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
        return 1;
    }

    INITCOMMONCONTROLSEX icc = { sizeof icc, ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSA wc = {};
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpfnWndProc   = WndProc;      wc.lpszClassName = "WPEAK64";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = SetWndProc;   wc.lpszClassName = "WPEAK64_SET";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassA(&wc);
    wc.lpfnWndProc   = ManualWndProc; wc.lpszClassName = "WPEAK64_MANUAL";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = ElemEditWndProc; wc.lpszClassName = "WPEAK64_ELEMEDIT";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = CalStdWndProc; wc.lpszClassName = "WPEAK64_CALSTD";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = RelayEditWndProc; wc.lpszClassName = "WPEAK64_RELAYEDIT";
    RegisterClassA(&wc);

    HMENU file = CreatePopupMenu();
    AppendMenuA(file, MF_STRING, IDM_OPEN_DATA,   "Open &Data (CSV)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_METHOD, "Open &Method (INI)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_CAL,    "Open &Calibration (INI)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_SAVE_METHOD, "&Save Method (INI)...");
    AppendMenuA(file, MF_STRING, IDM_EXPORT,      "&Export Report (CSV)...");
    AppendMenuA(file, MF_STRING, IDM_PRINT_REPORT,"&Print Report...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXIT,        "E&xit");
    HMENU run = CreatePopupMenu();
    AppendMenuA(run, MF_STRING, IDM_RUN_START, "&Start Run");
    AppendMenuA(run, MF_STRING, IDM_RUN_CAL,   "Start &Calibration (std 1)");
    AppendMenuA(run, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(run, MF_STRING, IDM_RUN_ABORT, "&Abort");
    HMENU opts = CreatePopupMenu();
    AppendMenuA(opts, MF_STRING, IDM_SETTINGS, "&Detector && Integration...");
    AppendMenuA(opts, MF_STRING, IDM_ELEMENTS, "Edit &Element Table...");
    AppendMenuA(opts, MF_STRING, IDM_RELAYS,   "Valve / &Port Schedule...");
    AppendMenuA(opts, MF_STRING, IDM_MANUAL,   "&Manual Valve / Relay Control...");
    HMENU help = CreatePopupMenu();
    AppendMenuA(help, MF_STRING, IDM_ABOUT, "&About WPEAK64...");
    HMENU menubar = CreateMenu();
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)file, "&File");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)run,  "&Acquire");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)opts, "&Options");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)help, "&Help");
    // (Run/Stop/Manual shortcuts live as icons on the toolbar row below the menu)

    g_main = CreateWindowA("WPEAK64",
                           "GC301c - WPEAK64 (64-bit Windows port)",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           1250, 1020, nullptr, menubar, hInst, nullptr);
    RunAnalysis(g_main, err);  // refresh title with loaded file names
    ShowWindow(g_main, nShow);
    UpdateWindow(g_main);
    SetTimer(g_main, 1, 200, nullptr);   // live-trace repaint during runs

    if(g_autorun) {            // kiosk-style start: begin a run immediately
        PostMessageA(g_main, WM_COMMAND, IDM_RUN_START, 0);
    }

    MSG msg;
    while(GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteCriticalSection(&g_acq.cs);
    return 0;
}
