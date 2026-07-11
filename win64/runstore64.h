// runstore64.h -- run persistence and run history for WPEAK64.
//
// Replaces the legacy binary .job file (JOB.CPP: method + run list + cal
// results + raw data in one file) with a job DIRECTORY of plain files:
//
//   jobdir/
//     runlist.csv                       append-only run history (the RunList)
//     run_000123/
//       meta.ini                        type, point, standard, noise/baseline,
//                                       timestamps, alarm state
//       trace.csv                       raw chromatogram (wpeak64_cli -d ...)
//       report.csv                      identified peaks + concentrations
//
// Every acquisition (run or cal) is persisted when a job directory is given,
// so the instrument keeps a complete audit trail in formats any tool reads.

#ifndef RUNSTORE64_H
#define RUNSTORE64_H

#include "method64.h"
#include <string>
#include <vector>

namespace wpeak64 {

struct AcquireResult;   // acquire64.h

struct RunRecord {
    int         run_num = 0;
    std::string dir;             // run_NNNNNN
    std::string timestamp;       // ISO 8601 local time
    std::string type;            // "run" or "cal"
    int         point = 1;       // sample point number (1-based)
    int         standard = 0;    // cal standard number, 0 for normal runs
    long        noise = 0, baseline = 0;
    int         npeaks = 0;
    std::string alarm;           // "", "HIGH", "LOW", "HIGH+LOW"

    // ---- Detector B (only meaningful when has_b) -----------------------------
    bool        has_b = false;
    long        noise_b = 0, baseline_b = 0;
    int         npeaks_b = 0;
    std::string alarm_b;
};

// Persist one completed acquisition into the job directory (created if
// needed). Fills rec with what was written. Returns false + err on failure.
bool SaveRun(const std::string &jobdir, const Method &m,
             const AcquireResult &res, const std::string &type,
             int point, int standard, int alarm_state,
             RunRecord &rec, std::string &err);

// Read the run history (runlist.csv). Missing file = empty history (ok).
bool LoadRunList(const std::string &jobdir, std::vector<RunRecord> &out,
                 std::string &err);

// Parse a stored run's report.csv into (component, concentration) pairs --
// calibrated positive peaks only. Used by the TWA/STEL calculation.
bool LoadRunConcentrations(const std::string &jobdir, const RunRecord &rec,
                           std::vector<std::pair<std::string,double>> &out,
                           std::string &err);

} // namespace wpeak64

#endif // RUNSTORE64_H
