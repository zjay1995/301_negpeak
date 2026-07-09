// wpeak64_gui.cpp -- native 64-bit Windows GUI for the WPEAK port.
// Replaces the legacy Borland OWL chromatogram window with plain Win32/GDI:
// draws the chromatogram trace, the measured baseline, markers for every
// detected peak (positive peaks numbered and labelled with their matched
// component name; negative peaks marked "NEG" -- detected but not
// quantified), plus a peak/concentration table.
//
// File menu: Open Data (CSV), Open Method (INI), Export Report (CSV).
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
#include "synth64.h"

using namespace wpeak64;

enum { IDM_OPEN_DATA = 101, IDM_OPEN_METHOD = 102, IDM_EXPORT = 103, IDM_EXIT = 104 };

static Method                 g_method;
static std::vector<long>      g_trace;
static Integrator            *g_integ = nullptr;
static std::vector<ReportRow> g_rows;
static std::string            g_data_path;    // empty = synthetic demo
static std::string            g_method_path;  // empty = defaults

static bool RunAnalysis(HWND hwnd, std::string &err)
{
    Method m;
    m.det.MinHeight = 20;                     // demo default
    if(!g_method_path.empty() && !LoadMethod(g_method_path, m, err))
        return false;

    std::vector<long> y;
    if(g_data_path.empty()) {
        SynthConfig cfg;
        cfg.data_rate = m.data_rate;
        cfg.analysis_time = m.analysis_time;
        y = MakeChromatogram(cfg, DefaultPeaks());
    }
    else if(!LoadChromatogram(g_data_path, y, err))
        return false;

    delete g_integ;
    g_integ = new Integrator(m.det, m.data_rate, m.analysis_time);
    for(long v : y)
        g_integ->ProcessPoint(v);

    g_method = m;
    g_trace  = y;
    g_rows   = BuildReport(g_integ->peaks, m);

    char title[512];
    std::snprintf(title, sizeof title, "GC301c - WPEAK64 (64-bit Windows port) - %s%s%s",
                  g_data_path.empty() ? "synthetic demo" : g_data_path.c_str(),
                  g_method_path.empty() ? "" : " / ",
                  g_method_path.c_str());
    if(hwnd) SetWindowTextA(hwnd, title);
    return true;
}

static void Paint(HDC dc, const RECT &rc)
{
    const int tableW = 360;
    RECT plot = rc;
    plot.left += 50; plot.right -= tableW + 10; plot.top += 40; plot.bottom -= 40;
    if(plot.right - plot.left < 50 || plot.bottom - plot.top < 50) return;

    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    if(g_trace.empty() || !g_integ) return;

    long ymin = g_trace[0], ymax = g_trace[0];
    for(long v : g_trace) { if(v < ymin) ymin = v; if(v > ymax) ymax = v; }
    long yspan = ymax - ymin; if(yspan < 1) yspan = 1;
    ymin -= yspan / 10; ymax += yspan / 10; yspan = ymax - ymin;
    const long n = (long)g_trace.size();
    const int data_rate = g_method.data_rate;

    auto X = [&](double i) { return plot.left + (int)((double)(plot.right - plot.left) * i / (n - 1)); };
    auto Y = [&](double v) { return plot.bottom - (int)((double)(plot.bottom - plot.top) * (v - ymin) / yspan); };

    SetBkMode(dc, TRANSPARENT);
    HPEN frame = CreatePen(PS_SOLID, 1, RGB(120,120,120));
    HGDIOBJ oldPen = SelectObject(dc, frame);
    Rectangle(dc, plot.left, plot.top, plot.right, plot.bottom);
    {
        const char *hdr = "GC301c Gas Chromatograph - WPEAK64 (64-bit port)";
        TextOutA(dc, plot.left, 10, hdr, (int)strlen(hdr));
    }

    // time axis: ~6 labels
    long total_s = n / data_rate;
    long step = total_s > 0 ? (total_s + 5) / 6 : 1; if(step < 1) step = 1;
    for(long s = 0; s <= total_s; s += step) {
        int x = X((double)s * data_rate);
        MoveToEx(dc, x, plot.bottom, nullptr); LineTo(dc, x, plot.bottom + 5);
        char lbl[16]; int len = std::snprintf(lbl, sizeof lbl, "%lds", s);
        TextOutA(dc, x - 8, plot.bottom + 8, lbl, len);
    }

    // baseline
    HPEN basePen = CreatePen(PS_DOT, 1, RGB(0,150,0));
    SelectObject(dc, basePen);
    MoveToEx(dc, plot.left, Y((double)g_integ->act_thresh), nullptr);
    LineTo(dc, plot.right, Y((double)g_integ->act_thresh));

    // trace
    HPEN tracePen = CreatePen(PS_SOLID, 1, RGB(0,0,200));
    SelectObject(dc, tracePen);
    MoveToEx(dc, X(0), Y((double)g_trace[0]), nullptr);
    for(long i = 1; i < n; i++)
        LineTo(dc, X((double)i), Y((double)g_trace[(size_t)i]));

    // peak markers + labels
    HPEN posPen = CreatePen(PS_SOLID, 2, RGB(200,0,0));
    HPEN negPen = CreatePen(PS_SOLID, 2, RGB(200,120,0));
    for(const ReportRow &r : g_rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        SelectObject(dc, neg ? negPen : posPen);
        int xa = X((double)p.From), xb = X((double)p.To), xm = X((double)p.Time);
        int yb = Y((double)g_integ->act_thresh);
        MoveToEx(dc, xa, yb - 6, nullptr); LineTo(dc, xa, yb + 6);
        MoveToEx(dc, xb, yb - 6, nullptr); LineTo(dc, xb, yb + 6);
        MoveToEx(dc, xa, yb, nullptr);     LineTo(dc, xb, yb);

        int yap = Y((double)(g_integ->act_thresh + p.Height));
        char lbl[64]; int len;
        SetTextColor(dc, neg ? RGB(200,120,0) : RGB(200,0,0));
        if(neg) {
            len = std::snprintf(lbl, sizeof lbl, "NEG");
            TextOutA(dc, xm - 12, yap + 6, lbl, len);
        }
        else {
            len = std::snprintf(lbl, sizeof lbl, "%d", p.Num);
            TextOutA(dc, xm - 4, yap - 18, lbl, len);
            if(r.component >= 0) {  // matched component name above the number
                len = (int)r.name.size();
                TextOutA(dc, xm - 4 * len, yap - 34, r.name.c_str(), len);
            }
        }
    }
    SetTextColor(dc, RGB(0,0,0));

    // right-hand peak table
    int tx = plot.right + 20, ty = plot.top;
    char line[160]; int len;
    len = std::snprintf(line, sizeof line, "Noise=%ld  Baseline=%ld  (%s method)",
                        g_integ->noise, g_integ->act_thresh,
                        g_method.detect_meth == 0 ? "height" : "area");
    TextOutA(dc, tx, ty, line, len); ty += 24;
    len = std::snprintf(line, sizeof line, "Num  Component     RT(s)  Height   Conc.");
    TextOutA(dc, tx, ty, line, len); ty += 18;
    for(const ReportRow &r : g_rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        char conc[24];
        if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
        else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
        if(neg)
            len = std::snprintf(line, sizeof line, "NEG  %-12s %5.1f  %7ld  (not quantified)",
                                r.name.c_str(), (double)p.Time / data_rate, p.Height);
        else
            len = std::snprintf(line, sizeof line, "%-4d %-12s %5.1f  %7ld  %s",
                                p.Num, r.name.c_str(), (double)p.Time / data_rate,
                                p.Height, conc);
        SetTextColor(dc, neg ? RGB(200,120,0) : RGB(0,0,0));
        TextOutA(dc, tx, ty, line, len); ty += 18;
    }
    SetTextColor(dc, RGB(0,0,0));

    SelectObject(dc, oldPen);
    DeleteObject(frame); DeleteObject(basePen); DeleteObject(tracePen);
    DeleteObject(posPen); DeleteObject(negPen);
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
                case IDM_EXPORT: {
                    std::string p = FileDialog(hwnd, true,
                        "Report CSV (*.csv)\0*.csv\0", "csv");
                    if(!p.empty() && g_integ) {
                        std::string err;
                        if(!WriteReportCsv(p, g_rows, g_method,
                                           g_integ->noise, g_integ->act_thresh, err))
                            MessageBoxA(hwnd, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
                    }
                    return 0;
                }
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    return 0;
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT rc; GetClientRect(hwnd, &rc);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
            HGDIOBJ old = SelectObject(mem, bmp);
            Paint(mem, rc);
            BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old);
            DeleteObject(bmp); DeleteDC(mem);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE:
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
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
        else if(!tok.empty())
            g_data_path = tok;
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nShow)
{
    ParseCmdLine(lpCmdLine);
    std::string err;
    if(!RunAnalysis(nullptr, err)) {
        MessageBoxA(nullptr, err.c_str(), "WPEAK64", MB_OK | MB_ICONERROR);
        return 1;
    }

    WNDCLASSA wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "WPEAK64";
    RegisterClassA(&wc);

    HMENU file = CreatePopupMenu();
    AppendMenuA(file, MF_STRING, IDM_OPEN_DATA,   "Open &Data (CSV)...");
    AppendMenuA(file, MF_STRING, IDM_OPEN_METHOD, "Open &Method (INI)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXPORT,      "&Export Report (CSV)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXIT,        "E&xit");
    HMENU menubar = CreateMenu();
    AppendMenuA(menubar, MF_POPUP, (UINT_PTR)file, "&File");

    HWND hwnd = CreateWindowA("WPEAK64",
                              "GC301c - WPEAK64 (64-bit Windows port)",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              1160, 640, nullptr, menubar, hInst, nullptr);
    RunAnalysis(hwnd, err);  // refresh title with loaded file names
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
