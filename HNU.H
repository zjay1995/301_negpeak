#ifndef  _HNU_H
#define  _HNU_H

/*
#define  PEAK           2
#define  END            3
#define  DOWN           0
#define  UP             1
*/

// Acquire from flags
#define	NOT_LOADING    0
#define	FROM_COMM      1
#define	FROM_FILE      2
#define  REDRAW         3
#define  EDITING_PARMS  4


// GCON modes
#define  MANUAL      0
#define  CONTINUOUS  1
#define  REPEAT      2
#define  MANSAVE     0
#define  AUTOSAVE    1
#define  SAVEONALARM 2
#define  TWA         3
#define  STEL        4
#define  ACQRAWDATA  1
#define  INTEGRATE   2

// GCON file types
#define  CAL            1
#define  JOB            2
#define  METHOD		3
#define  CHECKCAL	4


/*
// various externs
extern char          quiet;
extern char          deactivate_comments;
extern char          detector_label[2][41];
extern int           last_comment;
extern int           text_column[4];
extern int           header_row[2];
extern char          win_idx[NUMDETECTORS];
extern uchar         rslts_up_down;
extern BOOL          acq_load_from;    // current data acquisition mode
extern BOOL          prev_load_from;   // prev. data acquisition mode
extern char          *failure;
extern char          *notice;
*/

#endif
