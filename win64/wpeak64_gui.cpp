// wpeak64_gui.cpp -- native 64-bit Windows GUI for the WPEAK port.
//
// Three windows (replacing the legacy OWL windows):
//   main window        chromatogram viewer: trace, baseline, numbered peaks
//                      with component names, NEG markers, peak table
//   acquisition window live view of a running acquisition: phase, zone
//                      temperatures, growing trace (View > Acquisition)
//   element table      component table: RT windows, response factors,
//                      standards, calibration points, latest concentrations
//                      and alarm states (View > Element Table)
//
// File menu: Open Data (CSV), Open Method (INI), Open Calibration (INI),
// Export Report (CSV). Run menu: Start Run / Start Cal (background thread,
// simulation backend unless the method configures real hardware), Abort.
// Command line (optional): [run.csv] [method.ini] in any order.
//
// Build (MinGW-w64): see Makefile target build/wpeak64.exe

#include <windows.h>
#include <commdlg.h>
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
    IDM_RUN_START = 201, IDM_RUN_CAL = 202, IDM_RUN_ABORT = 203,
    IDM_WIN_ACQ = 301, IDM_WIN_ELEM = 302, IDM_CLOSE_WIN = 303,
    IDM_ABOUT = 401, IDM_SETTINGS = 501,
    IDC_SET_OK = 601, IDC_SET_CANCEL = 602,
};
static const UINT WM_ACQ_DONE = WM_APP + 1;

static Method                 g_method;
static std::vector<long>      g_trace;
static std::vector<ReportRow> g_rows;
static long                   g_noise = 0, g_baseline = 0;
static std::string            g_data_path;    // empty = synthetic demo
static std::string            g_method_path;  // empty = defaults
static std::string            g_cal_path;     // calibration file (optional)

static HWND g_main = nullptr, g_acqwnd = nullptr, g_elemwnd = nullptr;
static bool g_autorun = false;   // /autorun: open windows + start a run at launch

// ---- modern industrial style -------------------------------------------------
// Segoe UI for labels/headers (falls back to Tahoma/Arial where missing),
// Consolas for tabular numeric data; charcoal header strip per window.
static HFONT g_font_ui = nullptr, g_font_ui_bold = nullptr, g_font_mono = nullptr;
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
// Settings | Element Table | Acquisition | About. Icons are GDI-drawn.
struct ToolButton { int id; bool sep_before; };
static const ToolButton kToolButtons[] = {
    { IDM_OPEN_DATA,   false }, { IDM_OPEN_METHOD, false },
    { IDM_SAVE_METHOD, false }, { IDM_EXPORT,      false },
    { IDM_RUN_START,   true  }, { IDM_RUN_CAL,     false }, { IDM_RUN_ABORT, false },
    { IDM_SETTINGS,    true  }, { IDM_WIN_ELEM,    false },
    { IDM_WIN_ACQ,     false }, { IDM_ABOUT,       false },
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
        case IDM_OPEN_DATA: {                        // folder
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
        case IDM_SAVE_METHOD: {                      // floppy disk
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
        case IDM_WIN_ELEM: {                         // table grid
            Rectangle(dc, r.left+8, r.top+9, r.right-8, r.bottom-9);
            MoveToEx(dc, r.left+8, cy, nullptr);  LineTo(dc, r.right-8, cy);
            MoveToEx(dc, cx-3, r.top+9, nullptr); LineTo(dc, cx-3, r.bottom-9);
            break;
        }
        case IDM_WIN_ACQ: {                          // mini chromatogram
            POINT p[] = { {r.left+7,cy+6}, {r.left+11,cy+6}, {r.left+14,cy-8},
                          {r.left+17,cy+6}, {r.left+20,cy+9}, {r.left+23,cy+2},
                          {r.right-7,cy+2} };
            Polyline(dc, p, 7);
            break;
        }
        case IDM_ABOUT: {
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
        // button face
        HBRUSH face = CreateSolidBrush(RGB(55, 60, 66));
        RECT br = r;
        FillRect(dc, &br, face);
        DeleteObject(face);
        int id = kToolButtons[i].id;
        bool enabled = true;
        if(id == IDM_RUN_START || id == IDM_RUN_CAL) enabled = !running;
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

// charcoal strip with a white title and teal accent line; returns content top
static int DrawHeaderStrip(HDC dc, const RECT &rc, const char *title)
{
    RECT strip = rc; strip.bottom = strip.top + 36;
    HBRUSH bg = CreateSolidBrush(kHeaderBg);
    FillRect(dc, &strip, bg);
    DeleteObject(bg);
    RECT accent = strip; accent.top = strip.bottom; accent.bottom = strip.bottom + 3;
    HBRUSH ab = CreateSolidBrush(kAccent);
    FillRect(dc, &accent, ab);
    DeleteObject(ab);
    HGDIOBJ old = SelectObject(dc, g_font_ui_bold);
    SetTextColor(dc, kHeaderFg);
    TextOutA(dc, rc.left + 16, strip.top + 8, title, (int)strlen(title));
    SelectObject(dc, old);
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
    LeaveCriticalSection(&g_acq.cs);
    PostMessageA(g_main, WM_ACQ_DONE, 0, 0);
    return 0;
}

static bool StartAcquisition(HWND hwnd, bool is_cal, int standard)
{
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
                len = std::snprintf(lbl, sizeof lbl, "NEG");
                TextOutA(dc, xm - 12, yap + 6, lbl, len);
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
// Layout follows the legacy Peak Works frame on the GC301C: the peak table
// ("Table A") full-width on top, the chromatogram ("Graph A") full-width
// below it, and an instrument status bar along the bottom.
static std::string FmtTimeMS(double seconds)
{
    long m = (long)(seconds / 60);
    long s = (long)(seconds - m * 60 + 0.5);
    if(s == 60) { m++; s = 0; }
    char b[16];
    std::snprintf(b, sizeof b, "%ld:%02ld", m, s);
    return b;
}

static void PaintMain(HDC dc, const RECT &rc)
{
    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    // totals for the % columns (legacy: share of the summed magnitudes)
    int nneg = 0, alarm = ALARM_NONE;
    double habs = 0, aabs = 0;
    for(const ReportRow &r : g_rows) {
        if(r.peak.Height < 0) nneg++;
        alarm |= r.alarm;
        habs += r.peak.Height < 0 ? -(double)r.peak.Height : (double)r.peak.Height;
        aabs += r.peak.Area < 0 ? -r.peak.Area : r.peak.Area;
    }
    if(habs <= 0) habs = 1;
    if(aabs <= 0) aabs = 1;

    // bottom instrument status bar (oven temp from the last acquisition)
    EnterCriticalSection(&g_acq.cs);
    bool running = g_acq.running;
    std::string phase = g_acq.phase;
    auto zones = g_acq.zones;
    std::vector<long> live = running ? g_acq.live : std::vector<long>();
    LeaveCriticalSection(&g_acq.cs);

    // second menu row: icon toolbar (Run/Cal/Stop and file/window shortcuts)
    int top = DrawToolbar(dc, rc, running);

    char seg[96];
    std::vector<std::string> segs;
    for(auto &z : zones) {
        std::snprintf(seg, sizeof seg, "%s  %.1f \xb0""C", z.first.c_str(), z.second);
        segs.push_back(seg);
    }
    std::snprintf(seg, sizeof seg, "Baseline %ld", g_baseline); segs.push_back(seg);
    std::snprintf(seg, sizeof seg, "Noise %ld", g_noise);       segs.push_back(seg);
    segs.push_back(alarm == ALARM_NONE ? "ALARM none" :
                   alarm == ALARM_HIGH ? "ALARM HIGH" :
                   alarm == ALARM_LOW  ? "ALARM LOW"  : "ALARM HIGH+LOW");
    segs.push_back(running ? "ACQ " + phase : "ACQ idle");
    segs.push_back("DATA " + std::string(g_data_path.empty() ? "none"
                                         : BaseName(g_data_path).c_str()));
    segs.push_back("METHOD " + std::string(g_method_path.empty() ? "defaults"
                                           : BaseName(g_method_path).c_str()));
    segs.push_back("CAL " + std::string(g_cal_path.empty() ? "none"
                                        : BaseName(g_cal_path).c_str()));
    int bottom = DrawStatusBar(dc, rc, segs);

    // during an acquisition the live trace draws HERE, in the main graph
    // pane (legacy Graph A), not in a separate window
    if(running) {
        HGDIOBJ of = SelectObject(dc, g_font_ui_bold);
        SetTextColor(dc, kAccent);
        std::string s = "ACQUIRING  \xb7  " + (phase.empty() ? std::string("-") : phase);
        for(auto &z : zones) {
            char zb[48];
            std::snprintf(zb, sizeof zb, "   \xb7   %s %.1f \xb0""C", z.first.c_str(), z.second);
            s += zb;
        }
        TextOutA(dc, 20, top + 14, s.c_str(), (int)s.size());
        SelectObject(dc, of);
        SetTextColor(dc, RGB(0,0,0));

        RECT plot = rc;
        plot.left += 50; plot.right -= 30;
        plot.top = top + 52; plot.bottom = bottom - 40;
        if(plot.right - plot.left >= 50 && plot.bottom - plot.top >= 50) {
            if(live.size() >= 2)
                PaintTrace(dc, plot, live, 0, nullptr,
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
        return;
    }

    // empty startup: nothing loaded and nothing acquired yet
    if(g_trace.empty()) {
        HGDIOBJ of = SelectObject(dc, g_font_ui);
        SetTextColor(dc, kGridGray);
        const char *hint =
            "No data.  Open a chromatogram (File > Open Data) or press Run to start an acquisition.";
        TextOutA(dc, 20, top + 16, hint, (int)strlen(hint));
        SelectObject(dc, of);
        SetTextColor(dc, RGB(0,0,0));
        return;
    }

    // ---- Table A: full width on top ------------------------------------------
    HGDIOBJ oldFont = SelectObject(dc, g_font_mono);
    char line[200]; int len;
    int ty = top + 12;
    len = std::snprintf(line, sizeof line,
        "%-4s %-14s %10s %12s %8s %14s %8s %8s  %s",
        "Num", "Name", "Conc", "Height", "%", "Area", "%", "Time", "Alarm");
    SetTextColor(dc, kGridGray);
    TextOutA(dc, 20, ty, line, len); ty += 22;

    int max_table_bottom = top + 12 + 22 + (bottom - top) * 2 / 5;   // cap at ~40%
    for(const ReportRow &r : g_rows) {
        if(ty > max_table_bottom) break;
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        char num[8], conc[24];
        if(neg) std::snprintf(num, sizeof num, "-");
        else    std::snprintf(num, sizeof num, "%d", p.Num);
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
        TextOutA(dc, 20, ty, line, len); ty += 19;
    }
    SetTextColor(dc, RGB(0,0,0));
    SelectObject(dc, oldFont);

    // divider between table and graph (like the legacy child-window edge)
    {
        RECT div = rc; div.top = ty + 8; div.bottom = div.top + 2;
        HBRUSH db = CreateSolidBrush(kAccent);
        FillRect(dc, &div, db);
        DeleteObject(db);
    }

    // ---- Graph A: full width below --------------------------------------------
    RECT plot = rc;
    plot.left += 50; plot.right -= 30;
    plot.top = ty + 24;
    plot.bottom = bottom - 40;
    if(plot.right - plot.left < 50 || plot.bottom - plot.top < 50) return;
    PaintTrace(dc, plot, g_trace, g_baseline, &g_rows, g_method.data_rate);
}

// ---- acquisition window ---------------------------------------------------------
static void PaintAcq(HDC dc, const RECT &rc)
{
    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    EnterCriticalSection(&g_acq.cs);
    bool running = g_acq.running;
    std::string phase = g_acq.phase;
    auto zones = g_acq.zones;
    std::vector<long> live = g_acq.live;
    bool is_cal = g_acq.is_cal;
    LeaveCriticalSection(&g_acq.cs);

    char hdr[120];
    std::snprintf(hdr, sizeof hdr, "ACQUISITION  \xb7  %s %s",
                  is_cal ? "CALIBRATION" : "RUN",
                  running ? "IN PROGRESS" : "IDLE");
    int top = DrawHeaderStrip(dc, rc, hdr);

    // bottom info bar: backend, phase, sample count, zone temperatures
    std::vector<std::string> segs;
    segs.push_back("BACKEND  " + g_method.hw.backend);
    segs.push_back("PHASE  " + (phase.empty() ? std::string("-") : phase));
    {
        char seg[64];
        std::snprintf(seg, sizeof seg, "POINTS  %zu", live.size());
        segs.push_back(seg);
        for(auto &z : zones) {
            std::snprintf(seg, sizeof seg, "%s  %.1f \xb0""C", z.first.c_str(), z.second);
            segs.push_back(seg);
        }
    }
    int bottom = DrawStatusBar(dc, rc, segs);

    char line[200]; int len;
    HGDIOBJ oldFont = SelectObject(dc, g_font_ui);
    int tx = 20, ty = top + 10;
    len = std::snprintf(line, sizeof line, "Phase: %s",
                        phase.empty() ? "-" : phase.c_str());
    SetTextColor(dc, kAccent);
    TextOutA(dc, tx, ty, line, len);
    tx += 200;
    SetTextColor(dc, RGB(30,32,34));
    for(auto &z : zones) {
        len = std::snprintf(line, sizeof line, "%s  %.1f \xb0""C", z.first.c_str(), z.second);
        TextOutA(dc, tx, ty, line, len);
        tx += 160;
    }
    len = std::snprintf(line, sizeof line, "points: %zu", live.size());
    TextOutA(dc, tx, ty, line, len);
    SetTextColor(dc, RGB(0,0,0));
    SelectObject(dc, oldFont);

    RECT plot = rc;
    plot.left += 50; plot.right -= 30; plot.top = top + 44;
    plot.bottom = bottom - 40;
    if(plot.right - plot.left < 50 || plot.bottom - plot.top < 50) return;
    int data_rate = g_method.data_rate > 0 ? g_method.data_rate : 10;
    if(live.size() >= 2)
        PaintTrace(dc, plot, live, 0, nullptr, data_rate);
    else {
        oldFont = SelectObject(dc, g_font_ui);
        SetTextColor(dc, kGridGray);
        const char *w = "waiting for the ANALYZE phase...";
        TextOutA(dc, plot.left, plot.top, w, (int)strlen(w));
        SetTextColor(dc, RGB(0,0,0));
        SelectObject(dc, oldFont);
    }
}

// ---- element table window --------------------------------------------------------
static void PaintElem(HDC dc, const RECT &rc)
{
    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    char hdr[120];
    std::snprintf(hdr, sizeof hdr, "ELEMENT TABLE  \xb7  %zu COMPONENTS  \xb7  %s METHOD",
                  g_method.components.size(),
                  g_method.detect_meth == 0 ? "HEIGHT" : "AREA");
    int top = DrawHeaderStrip(dc, rc, hdr);

    // bottom info bar: component/calibration/alarm summary
    {
        int ncal = 0, nact = 0, alarm = ALARM_NONE;
        for(const Component &c : g_method.components) {
            if(c.active_yn) nact++;
            for(int s = 0; s < STAND_NUM64; s++)
                if(c.cal[s].valid) { ncal++; break; }
        }
        for(const ReportRow &r : g_rows) alarm |= r.alarm;
        char seg[64];
        std::vector<std::string> segs;
        std::snprintf(seg, sizeof seg, "ACTIVE  %d/%zu", nact, g_method.components.size());
        segs.push_back(seg);
        std::snprintf(seg, sizeof seg, "CALIBRATED  %d/%zu", ncal, g_method.components.size());
        segs.push_back(seg);
        segs.push_back(alarm == ALARM_NONE ? "ALARM  none" :
                       alarm == ALARM_HIGH ? "ALARM  HIGH" :
                       alarm == ALARM_LOW  ? "ALARM  LOW"  : "ALARM  HIGH+LOW");
        segs.push_back("METHOD  " + std::string(g_method_path.empty() ? "defaults"
                                                : BaseName(g_method_path).c_str()));
        DrawStatusBar(dc, rc, segs);
    }

    char line[240]; int len;
    HGDIOBJ oldFont = SelectObject(dc, g_font_mono);
    int ty = top + 12;
    len = std::snprintf(line, sizeof line,
        "%-14s %-7s %-7s %-9s %-8s %-8s %-24s %-10s %s",
        "Component", "RT(s)", "Win(s)", "RespFact", "AlarmHi", "AlarmLo",
        "Calibration (conc@resp)", "LastConc", "Alarm");
    SetTextColor(dc, kGridGray);
    TextOutA(dc, 20, ty, line, len); ty += 22;

    for(size_t i = 0; i < g_method.components.size(); i++) {
        const Component &c = g_method.components[i];
        // calibration summary
        char cal[96] = ""; size_t off = 0;
        for(int s = 0; s < STAND_NUM64 && off + 16 < sizeof cal; s++)
            if(c.cal[s].valid)
                off += std::snprintf(cal + off, sizeof cal - off, "%g@%g ",
                                     c.cal[s].conc, c.cal[s].resp);
        if(!off) std::snprintf(cal, sizeof cal, "(none)");
        // latest result for this component
        const ReportRow *last = nullptr;
        for(const ReportRow &r : g_rows)
            if(r.component == (int)i) last = &r;
        char conc[24] = "-";
        const char *alarm = "";
        if(last && last->calibrated)
            std::snprintf(conc, sizeof conc, "%g", last->concentration);
        if(last)
            alarm = last->alarm == ALARM_NONE ? "" :
                    last->alarm == ALARM_HIGH ? "HIGH" :
                    last->alarm == ALARM_LOW  ? "LOW"  : "H+L";

        len = std::snprintf(line, sizeof line,
            "%-14s %-7g %-7g %-9g %-8g %-8g %-24s %-10s %s",
            c.name.c_str(), c.peak_rt, c.window, c.response,
            c.alarm_high, c.alarm_low, cal, conc, alarm);
        SetTextColor(dc, (last && last->alarm) ? kAlarmRed
                         : c.active_yn ? RGB(30,32,34) : kGridGray);
        TextOutA(dc, 20, ty, line, len); ty += 19;
    }
    SetTextColor(dc, kGridGray);

    ty += 12;
    SelectObject(dc, g_font_ui);
    len = std::snprintf(line, sizeof line,
        "Standards (method): std1..std%d per component \xb7 calibrate with Run > Start Calibration",
        STAND_NUM64);
    TextOutA(dc, 20, ty, line, len);
    SetTextColor(dc, RGB(0,0,0));
    SelectObject(dc, oldFont);
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

// Run/Window menu shared by the secondary windows; run commands are
// forwarded to the main window which owns the acquisition thread.
static HMENU BuildChildMenu(bool with_run)
{
    HMENU bar = CreateMenu();
    if(with_run) {
        HMENU run = CreatePopupMenu();
        AppendMenuA(run, MF_STRING, IDM_RUN_START, "&Start Run");
        AppendMenuA(run, MF_STRING, IDM_RUN_CAL,   "Start &Calibration (std 1)");
        AppendMenuA(run, MF_SEPARATOR, 0, nullptr);
        AppendMenuA(run, MF_STRING, IDM_RUN_ABORT, "&Abort");
        AppendMenuA(bar, MF_POPUP, (UINT_PTR)run, "&Run");
    }
    HMENU win = CreatePopupMenu();
    AppendMenuA(win, MF_STRING, IDM_CLOSE_WIN, "&Close");
    AppendMenuA(bar, MF_POPUP, (UINT_PTR)win, "&Window");
    return bar;
}

static LRESULT CALLBACK AcqWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE:  SetTimer(hwnd, 1, 200, nullptr); return 0;
        case WM_COMMAND:
            if(LOWORD(wp) == IDM_CLOSE_WIN) { SendMessageA(hwnd, WM_CLOSE, 0, 0); return 0; }
            PostMessageA(g_main, WM_COMMAND, wp, 0);   // run commands
            return 0;
        case WM_TIMER:   InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_PAINT:   DoubleBufferPaint(hwnd, PaintAcq); return 0;
        case WM_SIZE:    InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_CLOSE:   KillTimer(hwnd, 1); DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_acqwnd = nullptr; return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static LRESULT CALLBACK ElemWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE:  SetTimer(hwnd, 1, 500, nullptr); return 0;
        case WM_COMMAND:
            if(LOWORD(wp) == IDM_CLOSE_WIN) { SendMessageA(hwnd, WM_CLOSE, 0, 0); return 0; }
            PostMessageA(g_main, WM_COMMAND, wp, 0);
            return 0;
        case WM_TIMER:   InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_PAINT:   DoubleBufferPaint(hwnd, PaintElem); return 0;
        case WM_SIZE:    InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_CLOSE:   KillTimer(hwnd, 1); DestroyWindow(hwnd); return 0;
        case WM_DESTROY: g_elemwnd = nullptr; return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ShowAcqWindow(HINSTANCE inst)
{
    if(g_acqwnd) { SetForegroundWindow(g_acqwnd); return; }
    g_acqwnd = CreateWindowA("WPEAK64_ACQ", "WPEAK64 - Acquisition",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             900, 480, nullptr, BuildChildMenu(true), inst, nullptr);
    ShowWindow(g_acqwnd, SW_SHOW);
}

static void ShowElemWindow(HINSTANCE inst)
{
    if(g_elemwnd) { SetForegroundWindow(g_elemwnd); return; }
    g_elemwnd = CreateWindowA("WPEAK64_ELEM", "WPEAK64 - Element Table",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              1000, 420, nullptr, BuildChildMenu(false), inst, nullptr);
    ShowWindow(g_elemwnd, SW_SHOW);
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
};
static HWND g_set_known = nullptr;   // known peaks only checkbox

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
    if(g_main)    InvalidateRect(g_main, nullptr, FALSE);
    if(g_elemwnd) InvalidateRect(g_elemwnd, nullptr, FALSE);
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
            SendMessageA(g_set_known, BM_SETCHECK,
                         g_method.known_peaks ? BST_CHECKED : BST_UNCHECKED, 0);
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
                             14 + 9 * 32 + 36 + 28 + 60,
                             g_main, nullptr, inst, nullptr);
    ShowWindow(g_setwnd, SW_SHOW);
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
                    }
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
                case IDM_WIN_ACQ:  ShowAcqWindow(inst);  return 0;
                case IDM_WIN_ELEM: ShowElemWindow(inst); return 0;
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
                char title[256];
                std::snprintf(title, sizeof title,
                              "GC301c - WPEAK64 - acquired %s%s",
                              was_cal ? "calibration run" : "run",
                              res.alarm_state ? "  *** ALARM ***" : "");
                SetWindowTextA(hwnd, title);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            if(g_elemwnd) InvalidateRect(g_elemwnd, nullptr, FALSE);
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
    ParseCmdLine(lpCmdLine);
    std::string err;
    if(!RunAnalysis(nullptr, err)) {
        MessageBoxA(nullptr, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
        return 1;
    }

    WNDCLASSA wc = {};
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpfnWndProc   = WndProc;      wc.lpszClassName = "WPEAK64";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = AcqWndProc;   wc.lpszClassName = "WPEAK64_ACQ";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = ElemWndProc;  wc.lpszClassName = "WPEAK64_ELEM";
    RegisterClassA(&wc);
    wc.lpfnWndProc   = SetWndProc;   wc.lpszClassName = "WPEAK64_SET";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassA(&wc);

    HMENU file = CreatePopupMenu();
    AppendMenuA(file, MF_STRING, IDM_OPEN_DATA,   "Open &Data (CSV)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_METHOD, "Open &Method (INI)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_CAL,    "Open &Calibration (INI)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_SAVE_METHOD, "&Save Method (INI)...");
    AppendMenuA(file, MF_STRING, IDM_EXPORT,      "&Export Report (CSV)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXIT,        "E&xit");
    HMENU run = CreatePopupMenu();
    AppendMenuA(run, MF_STRING, IDM_RUN_START, "&Start Run");
    AppendMenuA(run, MF_STRING, IDM_RUN_CAL,   "Start &Calibration (std 1)");
    AppendMenuA(run, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(run, MF_STRING, IDM_RUN_ABORT, "&Abort");
    HMENU opts = CreatePopupMenu();
    AppendMenuA(opts, MF_STRING, IDM_SETTINGS, "&Detector && Integration...");
    HMENU view = CreatePopupMenu();
    AppendMenuA(view, MF_STRING, IDM_WIN_ACQ,  "&Acquisition Window");
    AppendMenuA(view, MF_STRING, IDM_WIN_ELEM, "&Element Table");
    HMENU help = CreatePopupMenu();
    AppendMenuA(help, MF_STRING, IDM_ABOUT, "&About WPEAK64...");
    HMENU menubar = CreateMenu();
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)file, "&File");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)run,  "&Acquire");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)opts, "&Options");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)view, "&Window");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)help, "&Help");
    // (Run/Stop shortcuts live as icons on the toolbar row below the menu)

    g_main = CreateWindowA("WPEAK64",
                           "GC301c - WPEAK64 (64-bit Windows port)",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           1200, 640, nullptr, menubar, hInst, nullptr);
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
