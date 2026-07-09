// wpeak64_gui.cpp -- native 64-bit Windows GUI for the WPEAK port.
// Replaces the legacy Borland OWL chromatogram window with plain Win32/GDI:
// draws the chromatogram trace, the measured baseline, and markers for every
// detected peak (positive peaks numbered; negative peaks marked "NEG" --
// detected but not quantified, matching the GC301c behavior).
//
// Build (MinGW-w64): see Makefile target build/wpeak64.exe

#include <windows.h>
#include <cstdio>
#include <string>
#include "peak64.h"
#include "synth64.h"

using namespace wpeak64;

static SynthConfig       g_cfg;
static std::vector<long> g_trace;
static Integrator       *g_integ = nullptr;

static void RunAnalysis()
{
    DetectorSettings det;
    det.MinHeight = 20;

    static Integrator integ(det, g_cfg.data_rate, g_cfg.analysis_time);
    g_integ = &integ;

    g_trace = MakeChromatogram(g_cfg, DefaultPeaks());
    for(long v : g_trace)
        integ.ProcessPoint(v);
}

static void Paint(HDC dc, const RECT &rc)
{
    // layout: plot area + right-hand peak table
    const int tableW = 300;
    RECT plot = rc;
    plot.left += 50; plot.right -= tableW + 10; plot.top += 40; plot.bottom -= 40;
    if(plot.right - plot.left < 50 || plot.bottom - plot.top < 50) return;

    FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // data ranges
    long ymin = g_trace[0], ymax = g_trace[0];
    for(long v : g_trace) { if(v < ymin) ymin = v; if(v > ymax) ymax = v; }
    long yspan = ymax - ymin; if(yspan < 1) yspan = 1;
    ymin -= yspan / 10; ymax += yspan / 10; yspan = ymax - ymin;
    const long n = (long)g_trace.size();

    auto X = [&](double i)    { return plot.left + (int)((double)(plot.right - plot.left) * i / (n - 1)); };
    auto Y = [&](double v)    { return plot.bottom - (int)((double)(plot.bottom - plot.top) * (v - ymin) / yspan); };

    // frame + title
    SetBkMode(dc, TRANSPARENT);
    HPEN frame = CreatePen(PS_SOLID, 1, RGB(120,120,120));
    HGDIOBJ oldPen = SelectObject(dc, frame);
    Rectangle(dc, plot.left, plot.top, plot.right, plot.bottom);
    TextOutA(dc, plot.left, 10,
             "GC301c Gas Chromatograph - WPEAK64 (64-bit port) - synthetic run",
             (int)strlen("GC301c Gas Chromatograph - WPEAK64 (64-bit port) - synthetic run"));

    // time axis labels every 20 s
    for(long s = 0; s <= g_cfg.analysis_time; s += 20) {
        int x = X((double)s * g_cfg.data_rate);
        MoveToEx(dc, x, plot.bottom, nullptr); LineTo(dc, x, plot.bottom + 5);
        char lbl[16]; int len = std::snprintf(lbl, sizeof lbl, "%lds", s);
        TextOutA(dc, x - 8, plot.bottom + 8, lbl, len);
    }

    // baseline (measured act_thresh)
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

    // peak markers
    HPEN posPen = CreatePen(PS_SOLID, 2, RGB(200,0,0));
    HPEN negPen = CreatePen(PS_SOLID, 2, RGB(200,120,0));
    for(const Peak &p : g_integ->peaks) {
        bool neg = p.Height < 0;
        SelectObject(dc, neg ? negPen : posPen);
        int xa = X((double)p.From), xb = X((double)p.To), xm = X((double)p.Time);
        int yb = Y((double)g_integ->act_thresh);
        // From/To bracket on the baseline
        MoveToEx(dc, xa, yb - 6, nullptr); LineTo(dc, xa, yb + 6);
        MoveToEx(dc, xb, yb - 6, nullptr); LineTo(dc, xb, yb + 6);
        MoveToEx(dc, xa, yb, nullptr);     LineTo(dc, xb, yb);
        // apex label
        int ylab = neg ? Y((double)(g_integ->act_thresh + p.Height)) + 6
                       : Y((double)(g_integ->act_thresh + p.Height)) - 18;
        char lbl[16];
        int len = neg ? std::snprintf(lbl, sizeof lbl, "NEG")
                      : std::snprintf(lbl, sizeof lbl, "%d", p.Num);
        SetTextColor(dc, neg ? RGB(200,120,0) : RGB(200,0,0));
        TextOutA(dc, xm - 6, ylab, lbl, len);
    }
    SetTextColor(dc, RGB(0,0,0));

    // right-hand peak table
    int tx = plot.right + 20, ty = plot.top;
    char line[128]; int len;
    len = std::snprintf(line, sizeof line, "Noise=%ld  Baseline=%ld",
                        g_integ->noise, g_integ->act_thresh);
    TextOutA(dc, tx, ty, line, len); ty += 24;
    len = std::snprintf(line, sizeof line, "Num   RT(s)   Height     Area");
    TextOutA(dc, tx, ty, line, len); ty += 18;
    for(const Peak &p : g_integ->peaks) {
        bool neg = p.Height < 0;
        if(neg)
            len = std::snprintf(line, sizeof line, "NEG   %5.1f  %7ld   (not quantified)",
                                (double)p.Time / g_cfg.data_rate, p.Height);
        else
            len = std::snprintf(line, sizeof line, "%-4d  %5.1f  %7ld  %9.0f",
                                p.Num, (double)p.Time / g_cfg.data_rate, p.Height, p.Area);
        SetTextColor(dc, neg ? RGB(200,120,0) : RGB(0,0,0));
        TextOutA(dc, tx, ty, line, len); ty += 18;
    }
    SetTextColor(dc, RGB(0,0,0));

    SelectObject(dc, oldPen);
    DeleteObject(frame); DeleteObject(basePen); DeleteObject(tracePen);
    DeleteObject(posPen); DeleteObject(negPen);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT rc; GetClientRect(hwnd, &rc);
            // double buffer
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

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow)
{
    RunAnalysis();

    WNDCLASSA wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "WPEAK64";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("WPEAK64",
                              "GC301c - WPEAK64 (64-bit Windows port)",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              1100, 620, nullptr, nullptr, hInst, nullptr);
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
