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
#include "synth64.h"

using namespace wpeak64;

enum {
    IDM_OPEN_DATA = 101, IDM_OPEN_METHOD = 102, IDM_EXPORT = 103,
    IDM_EXIT = 104, IDM_OPEN_CAL = 105,
    IDM_RUN_START = 201, IDM_RUN_CAL = 202, IDM_RUN_ABORT = 203,
    IDM_WIN_ACQ = 301, IDM_WIN_ELEM = 302,
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

    std::vector<long> y;
    if(g_data_path.empty()) {
        SynthConfig cfg;
        cfg.data_rate = m.data_rate;
        cfg.analysis_time = m.analysis_time;
        y = MakeChromatogram(cfg, DefaultPeaks());
    }
    else if(!LoadChromatogram(g_data_path, y, err))
        return false;

    Integrator integ(m.det, m.data_rate, m.analysis_time);
    for(long v : y)
        integ.ProcessPoint(v);

    g_method   = m;
    g_trace    = y;
    g_rows     = BuildReport(integ.peaks, m);
    EvaluateAlarms(g_rows, m);
    g_noise    = integ.noise;
    g_baseline = integ.act_thresh;

    char title[512];
    std::snprintf(title, sizeof title, "GC301c - WPEAK64 (64-bit Windows port) - %s%s%s",
                  g_data_path.empty() ? "synthetic demo" : g_data_path.c_str(),
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
static void PaintMain(HDC dc, const RECT &rc)
{
    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc, TRANSPARENT);
    int top = DrawHeaderStrip(dc, rc, "GC301c GAS CHROMATOGRAPH  \xb7  WPEAK64");

    const int tableW = 420;
    RECT plot = rc;
    plot.left += 50; plot.right -= tableW + 10; plot.top = top + 16; plot.bottom -= 40;
    if(plot.right - plot.left < 50 || plot.bottom - plot.top < 50) return;
    if(g_trace.empty()) return;

    PaintTrace(dc, plot, g_trace, g_baseline, &g_rows, g_method.data_rate);

    int tx = plot.right + 20, ty = plot.top;
    char line[160]; int len;
    HGDIOBJ oldFont = SelectObject(dc, g_font_ui);
    len = std::snprintf(line, sizeof line, "Noise %ld    Baseline %ld    %s method",
                        g_noise, g_baseline,
                        g_method.detect_meth == 0 ? "height" : "area");
    TextOutA(dc, tx, ty, line, len); ty += 26;
    SelectObject(dc, g_font_mono);
    len = std::snprintf(line, sizeof line, "Num  Component     RT(s)  Height   Conc.   Alarm");
    SetTextColor(dc, kGridGray);
    TextOutA(dc, tx, ty, line, len); ty += 20;
    for(const ReportRow &r : g_rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        char conc[24];
        if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
        else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
        const char *alarm = r.alarm == ALARM_NONE ? "" :
                            r.alarm == ALARM_HIGH ? "HIGH" :
                            r.alarm == ALARM_LOW  ? "LOW"  : "H+L";
        if(neg)
            len = std::snprintf(line, sizeof line, "NEG  %-12s %5.1f  %7ld  (not quantified)",
                                r.name.c_str(), (double)p.Time / g_method.data_rate, p.Height);
        else
            len = std::snprintf(line, sizeof line, "%-4d %-12s %5.1f  %7ld  %-7s %s",
                                p.Num, r.name.c_str(), (double)p.Time / g_method.data_rate,
                                p.Height, conc, alarm);
        SetTextColor(dc, r.alarm ? kAlarmRed : neg ? kNegOrange : RGB(30,32,34));
        TextOutA(dc, tx, ty, line, len); ty += 19;
    }
    SetTextColor(dc, RGB(0,0,0));
    SelectObject(dc, oldFont);
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
    plot.left += 50; plot.right -= 30; plot.top = top + 44; plot.bottom -= 40;
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

static LRESULT CALLBACK AcqWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_CREATE:  SetTimer(hwnd, 1, 200, nullptr); return 0;
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
                             900, 480, nullptr, nullptr, inst, nullptr);
    ShowWindow(g_acqwnd, SW_SHOW);
}

static void ShowElemWindow(HINSTANCE inst)
{
    if(g_elemwnd) { SetForegroundWindow(g_elemwnd); return; }
    g_elemwnd = CreateWindowA("WPEAK64_ELEM", "WPEAK64 - Element Table",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              1000, 420, nullptr, nullptr, inst, nullptr);
    ShowWindow(g_elemwnd, SW_SHOW);
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
                    if(StartAcquisition(hwnd, false, 0)) ShowAcqWindow(inst);
                    return 0;
                case IDM_RUN_CAL:
                    if(StartAcquisition(hwnd, true, 1)) ShowAcqWindow(inst);
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

    HMENU file = CreatePopupMenu();
    AppendMenuA(file, MF_STRING, IDM_OPEN_DATA,   "Open &Data (CSV)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_METHOD, "Open &Method (INI)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_CAL,    "Open &Calibration (INI)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXPORT,      "&Export Report (CSV)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXIT,        "E&xit");
    HMENU run = CreatePopupMenu();
    AppendMenuA(run, MF_STRING, IDM_RUN_START, "&Start Run");
    AppendMenuA(run, MF_STRING, IDM_RUN_CAL,   "Start &Calibration (std 1)");
    AppendMenuA(run, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(run, MF_STRING, IDM_RUN_ABORT, "&Abort");
    HMENU view = CreatePopupMenu();
    AppendMenuA(view, MF_STRING, IDM_WIN_ACQ,  "&Acquisition Window");
    AppendMenuA(view, MF_STRING, IDM_WIN_ELEM, "&Element Table");
    HMENU menubar = CreateMenu();
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)file, "&File");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)run,  "&Run");
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)view, "&View");

    g_main = CreateWindowA("WPEAK64",
                           "GC301c - WPEAK64 (64-bit Windows port)",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           1200, 640, nullptr, menubar, hInst, nullptr);
    RunAnalysis(g_main, err);  // refresh title with loaded file names
    ShowWindow(g_main, nShow);
    UpdateWindow(g_main);

    if(g_autorun) {            // kiosk-style start: both windows + a run
        ShowElemWindow(hInst);
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
