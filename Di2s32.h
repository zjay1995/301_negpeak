#include "hdrdef32.h"

#ifndef ACHN
#define ACHN 29
#endif

typedef struct tagCODASHDR
{
   unsigned short header[2];
   struct hdrcmn hdrc;
   struct chinfo cary[ACHN];
   unsigned short vbflags;
} CODASHDR;

typedef struct di_anin_struct {
   unsigned short chan; /* input channel. 0 to 15 */
   unsigned short diff; /* input config. 0 = single end, 1 = diff. */
   unsigned short gain; /* gain code. 0 to 3 */
   unsigned short unipolar; /* unipolar or bipolar. 0 = BI, 1 = UN */
} DI_ANIN_STRUCT;

typedef struct di_info_struct {
   unsigned short port; /* device port address */
   unsigned short buf_in_chn; /* device input channel */
   unsigned short buf_out_chn; /* device output channel */
   unsigned short sft_lvl; /* software interrupt level */
   unsigned short hrd_lvl; /* hardware interrupt level */
   short *buf_in_ptr; /* input buffer pointer */
   unsigned short buf_in_size; /* input buffer size in words */
   short *buf_out_ptr; /* output buffer pointer */
   unsigned short buf_out_size; /* output buffer size in words */
   char tsr_version[20]; /* TSR version & hardware model */
   char dsp_version[20]; /* DSP program version */
   char sdk_version[20]; /* SDK library version */
   unsigned long serial_no; /* PCB serial number */
   unsigned long last_cal; /* last calibration time in sec since 1-1-1970 */
   char board_id[10]; /* PCB model name */
   char pgh_pgl;  /* type of PGA 0 = pgh, 1 = pgl */
   char hrdwr_rev; /* ASCII char REV letter */
   char pal0_rev; /* ASCII char REV of PAL0 */
   char pal1_rev; /* ASCII char REV of PAL1 */
} DI_INFO_STRUCT;

typedef struct di_info_struct32 {
   unsigned short port; /* device port address */
   unsigned short buf_in_chn; /* device input channel */
   unsigned short buf_out_chn; /* device output channel */
   unsigned short sft_lvl; /* software interrupt level */
   unsigned short hrd_lvl; /* hardware interrupt level */
   unsigned short padding; /* for 4-byte alignment of following */
   short *buf_in_ptr; /* input buffer pointer */
   unsigned buf_in_size; /* input buffer size in words */
   short *buf_out_ptr; /* output buffer pointer */
   unsigned buf_out_size; /* output buffer size in words */
   char tsr_version[20]; /* TSR version & hardware model */
   char dsp_version[20]; /* DSP program version */
   char sdk_version[20]; /* SDK library version */
   unsigned long serial_no; /* PCB serial number */
   unsigned long last_cal; /* last calibration time in sec since 1-1-1970 */
   char board_id[10]; /* PCB model name */
   char pgh_pgl;  /* type of PGA 0 = pgh, 1 = pgl */
   char hrdwr_rev; /* ASCII char REV letter */
   char pal0_rev; /* ASCII char REV of PAL0 */
   char pal1_rev; /* ASCII char REV of PAL1 */
} DI_INFO_STRUCT32;

typedef struct di_mode_struct {
   unsigned short mode:4; /* mode 0 = off, 1 = triggered analog, 2 = digital */
   unsigned short hystx:4; /* hysteresis index for DI-500 */
   unsigned short scnx:8; /* scanlist index of DI-500 trig chan, 0 for first chan */
   unsigned short trig_level; /* trigger level */
   unsigned short trig_slope; /* trigger slope 0 = +, 1 = - */
   unsigned short trig_pre; /* pre trigger number fo samples before trigger */
   unsigned short trig_post; /* post trigger length of scan before trigger occurs */
} DI_MODE_STRUCT;

typedef struct di_inlist_struct {
   unsigned short chan; /* input channel. 0 to 15 */
   unsigned short diff; /* input config. 0 = single ended, 1 = diff */
   unsigned short gain; /* gain. 0 to 3 */
   unsigned short unipolar; /* unipolar or bipolar, 0 = BI, 1= UN */
   unsigned short dig_out_enable; /* 1 = digital output enable */
   unsigned short dig_out; /* D0 to D4 = data */
   unsigned short ave; /* sample averaging, 0 = off, 1 = on */
   unsigned short counter; /* scan position counter */
} DI_INLIST_STRUCT;

typedef struct di_outlist_struct {
   unsigned short unipolar; /* unipolar or bipolar. 0 = BI, 1 = UN */
   unsigned short digital; /* digital output from buffer = 1, analog output = 0 */
   unsigned short dig_out_enable; /* 1 = digital out enable with analog */
   unsigned short dig_data; /* D0 to D7 = data for output */
   unsigned short counter; /* scan position counter */
} DI_OUTLIST_STRUCT;

#ifndef NODIFUNCS
#ifdef __cplusplus
extern "C" {
#endif
extern int PASCAL di_com(unsigned di_command,unsigned di_data);
#ifdef WINNT
extern int PASCAL di_open(unsigned devno);
extern int PASCAL di_set_port_mode(unsigned mode);
extern void PASCAL di_set_data_mode(unsigned datamode);
extern unsigned PASCAL di_get_data_mode(void);
#else
extern int PASCAL di_open(void);
#endif
extern int PASCAL di_reset(void);
extern int PASCAL di_close(void);
extern int PASCAL di_info(struct di_info_struct *info);
extern int PASCAL di_info32(struct di_info_struct32 *info);
extern short * PASCAL di_buffer_alloc(unsigned chn,unsigned size);
extern unsigned PASCAL di_buffer_status(unsigned chn);
extern int PASCAL di_buffer_free(unsigned chn);
extern unsigned PASCAL di_digin(void);
extern int PASCAL di_anin(struct di_anin_struct *analog_input);
extern unsigned PASCAL di_digout(unsigned i);
extern int PASCAL di_anout(unsigned dac_data,unsigned range);
extern unsigned PASCAL di_mode(struct di_mode_struct *mode);
extern int PASCAL di_start_scan(void);
extern int PASCAL di_stop_scan(void);
extern int PASCAL di_inlist(struct di_inlist_struct *input_list);
extern int PASCAL di_outlist(struct di_outlist_struct *output_list);
extern int PASCAL di_list_length(unsigned in_length,unsigned out_length);
extern int PASCAL di_burst_rate(unsigned rate);
extern int PASCAL di_strerr(unsigned err_code,char *err_str);
extern unsigned PASCAL di_buffer_size(void);
extern int PASCAL di_copy_buffer(unsigned buf_index,short *dest,
   unsigned word_count);
extern int PASCAL di_copy_array(unsigned buf_index,short *source,
   unsigned word_count);
extern int PASCAL di_ct_event(unsigned bit_and,unsigned bit_xor);
extern int PASCAL di_ct_one_shot(unsigned bit_and,unsigned bit_xor,
   unsigned bit_out, unsigned width);
extern int PASCAL di_ct_wave(unsigned bit_out,unsigned pos_width,
   unsigned neg_width);
extern int PASCAL di_ct_status(void);
extern int PASCAL di_ct_stop(void);
extern int PASCAL di_trigger_status(void);
extern unsigned PASCAL di_status_read(short *dest,
   unsigned num_scans);
extern void PASCAL di_status_reset(void);
extern int PASCAL di_copy_mux(short *dest);
#ifdef WINVER
extern CODASHDR * PASCAL di_get_acq_header(void);
extern unsigned PASCAL di_copy_header(unsigned hdr_index,CODASHDR *dest,
   unsigned byte_count,unsigned clear_bits);
extern unsigned PASCAL di_copy_header32(unsigned hdr_index,void *dest,
   unsigned byte_count,unsigned clear_bits);
extern unsigned PASCAL di_buffer_size(void);
extern void PASCAL di_msg_enable(HWND hWnd,unsigned msgid,unsigned millisec);
#else
extern void PASCAL di_heapend(void *heapend);
#endif
extern unsigned long PASCAL di_int(unsigned intcmd);
#ifdef __cplusplus
}
#endif
#endif
