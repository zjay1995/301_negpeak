// runstore64.cpp -- run persistence and history. See runstore64.h.

#include "runstore64.h"
#include "acquire64.h"
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

namespace wpeak64 {

static bool MakeDir(const std::string &path)
{
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

static std::string NowIso()
{
    char buf[32];
    std::time_t t = std::time(nullptr);
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S", std::localtime(&t));
    return buf;
}

static int NextRunNumber(const std::string &jobdir)
{
    std::vector<RunRecord> hist;
    std::string err;
    LoadRunList(jobdir, hist, err);
    int n = 0;
    for(const RunRecord &r : hist)
        if(r.run_num > n) n = r.run_num;
    return n + 1;
}

bool SaveRun(const std::string &jobdir, const Method &m,
             const AcquireResult &res, const std::string &type,
             int point, int standard, int alarm_state,
             RunRecord &rec, std::string &err)
{
    if(!MakeDir(jobdir)) { err = "cannot create job directory: " + jobdir; return false; }

    rec = RunRecord();
    rec.run_num   = NextRunNumber(jobdir);
    rec.timestamp = NowIso();
    rec.type      = type;
    rec.point     = point;
    rec.standard  = standard;
    rec.noise     = res.noise;
    rec.baseline  = res.baseline;
    rec.npeaks    = (int)res.rows.size();
    rec.alarm     = alarm_state == ALARM_NONE ? "" :
                    alarm_state == ALARM_HIGH ? "HIGH" :
                    alarm_state == ALARM_LOW  ? "LOW"  : "HIGH+LOW";
    rec.has_b     = res.has_b;
    if(res.has_b) {
        rec.noise_b    = res.noise_b;
        rec.baseline_b = res.baseline_b;
        rec.npeaks_b   = (int)res.rows_b.size();
        rec.alarm_b    = res.alarm_state_b == ALARM_NONE ? "" :
                         res.alarm_state_b == ALARM_HIGH ? "HIGH" :
                         res.alarm_state_b == ALARM_LOW  ? "LOW"  : "HIGH+LOW";
    }
    char dbuf[32];
    std::snprintf(dbuf, sizeof dbuf, "run_%06d", rec.run_num);
    rec.dir = dbuf;

    std::string rundir = jobdir + "/" + rec.dir;
    if(!MakeDir(rundir)) { err = "cannot create run directory: " + rundir; return false; }

    { // meta.ini
        std::ofstream f(rundir + "/meta.ini");
        if(!f) { err = "cannot write " + rundir + "/meta.ini"; return false; }
        f << "[run]\n"
          << "num=" << rec.run_num << "\n"
          << "time=" << rec.timestamp << "\n"
          << "type=" << rec.type << "\n"
          << "point=" << rec.point << "\n"
          << "standard=" << rec.standard << "\n"
          << "noise=" << rec.noise << "\n"
          << "baseline=" << rec.baseline << "\n"
          << "peaks=" << rec.npeaks << "\n"
          << "alarm=" << rec.alarm << "\n";
        if(res.has_b)
            f << "detector_b=1\n"
              << "noise_b=" << rec.noise_b << "\n"
              << "baseline_b=" << rec.baseline_b << "\n"
              << "peaks_b=" << rec.npeaks_b << "\n"
              << "alarm_b=" << rec.alarm_b << "\n";
    }
    { // trace.csv
        std::ofstream f(rundir + "/trace.csv");
        if(!f) { err = "cannot write " + rundir + "/trace.csv"; return false; }
        f << "time_s,counts\n";
        for(size_t i = 0; i < res.trace.size(); i++)
            f << (double)i / m.data_rate << "," << res.trace[i] << "\n";
    }
    if(!WriteReportCsv(rundir + "/report.csv", res.rows, m,
                       res.noise, res.baseline, err))
        return false;
    if(!WriteReportHtml(rundir + "/report.html", res.trace, res.rows, m,
                        res.noise, res.baseline, err))
        return false;

    if(res.has_b) {
        Method mb = m.AsDetectorB();
        { // trace_b.csv
            std::ofstream f(rundir + "/trace_b.csv");
            if(!f) { err = "cannot write " + rundir + "/trace_b.csv"; return false; }
            f << "time_s,counts\n";
            for(size_t i = 0; i < res.trace_b.size(); i++)
                f << (double)i / m.data_rate << "," << res.trace_b[i] << "\n";
        }
        if(!WriteReportCsv(rundir + "/report_b.csv", res.rows_b, mb,
                           res.noise_b, res.baseline_b, err))
            return false;
        if(!WriteReportHtml(rundir + "/report_b.html", res.trace_b, res.rows_b, mb,
                            res.noise_b, res.baseline_b, err))
            return false;
    }

    { // append to the run list
        std::string listpath = jobdir + "/runlist.csv";
        bool fresh = !std::ifstream(listpath);
        std::ofstream f(listpath, std::ios::app);
        if(!f) { err = "cannot append " + listpath; return false; }
        if(fresh)
            f << "num,dir,time,type,point,standard,noise,baseline,peaks,alarm,"
                 "has_b,noise_b,baseline_b,peaks_b,alarm_b\n";
        f << rec.run_num << "," << rec.dir << "," << rec.timestamp << ","
          << rec.type << "," << rec.point << "," << rec.standard << ","
          << rec.noise << "," << rec.baseline << "," << rec.npeaks << ","
          << rec.alarm << ","
          << (rec.has_b ? 1 : 0) << "," << rec.noise_b << "," << rec.baseline_b << ","
          << rec.npeaks_b << "," << rec.alarm_b << "\n";
    }
    return true;
}

bool LoadRunList(const std::string &jobdir, std::vector<RunRecord> &out,
                 std::string &err)
{
    out.clear();
    std::ifstream f(jobdir + "/runlist.csv");
    if(!f) return true;                       // no history yet: fine
    std::string line;
    std::getline(f, line);                    // header
    while(std::getline(f, line)) {
        RunRecord r;
        char dir[64] = "", time[40] = "", type[16] = "", alarm[16] = "", alarm_b[16] = "";
        int has_b = 0;
        // alarm/alarm_b may be empty -> %15[^,] can fail there; parse leniently.
        // Trailing has_b/noise_b/baseline_b/peaks_b/alarm_b columns are absent
        // in run lists written before Detector B support -- n>=9 still parses
        // those older lines fine (has_b defaults to false).
        int n = std::sscanf(line.c_str(),
                            "%d,%63[^,],%39[^,],%15[^,],%d,%d,%ld,%ld,%d,%15[^,],"
                            "%d,%ld,%ld,%d,%15s",
                            &r.run_num, dir, time, type, &r.point, &r.standard,
                            &r.noise, &r.baseline, &r.npeaks, alarm,
                            &has_b, &r.noise_b, &r.baseline_b, &r.npeaks_b, alarm_b);
        if(n >= 9) {
            r.dir = dir; r.timestamp = time; r.type = type; r.alarm = alarm;
            if(n >= 14) r.has_b = has_b != 0;
            if(n >= 15) r.alarm_b = alarm_b;
            out.push_back(r);
        }
        else {
            err = jobdir + "/runlist.csv: malformed line: " + line;
            return false;
        }
    }
    return true;
}

bool LoadRunConcentrations(const std::string &jobdir, const RunRecord &rec,
                           std::vector<std::pair<std::string,double>> &out,
                           std::string &err)
{
    out.clear();
    std::string path = jobdir + "/" + rec.dir + "/report.csv";
    std::ifstream f(path);
    if(!f) { err = "cannot open " + path; return false; }
    std::string line;
    while(std::getline(f, line)) {
        if(line.empty() || line[0] == '#' || line.compare(0, 4, "num,") == 0)
            continue;
        // num,component,rt_s,height,area,from_s,to_s,concentration,type,alarm
        std::string fields[10];
        size_t pos = 0;
        for(int i = 0; i < 10; i++) {
            size_t comma = line.find(',', pos);
            fields[i] = line.substr(pos, comma == std::string::npos
                                         ? std::string::npos : comma - pos);
            if(comma == std::string::npos) break;
            pos = comma + 1;
        }
        if(fields[8] != "positive" || fields[7].empty())
            continue;                      // uncalibrated or negative peak
        out.push_back({ fields[1], std::atof(fields[7].c_str()) });
    }
    return true;
}

} // namespace wpeak64
