#ifndef  _MYDI_H
#define  _MYDI_H

#define WINNT
#include "di2s32.h"


#ifndef __WIN32__


int my_di_com(unsigned di_command,unsigned di_data);

int my_di_info32(struct di_info_struct32 *info);

int my_di_open(unsigned devno);
int my_di_set_port_mode(unsigned mode);
void my_di_set_data_mode(unsigned datamode);
unsigned my_di_get_data_mode(void);

//int my_di_reset(void);
int my_di_close(void);

unsigned my_di_buffer_status(unsigned chn);
short * my_di_buffer_alloc(unsigned chn,unsigned size);
unsigned my_di_buffer_free(unsigned chn);

unsigned my_di_digin(void);
int my_di_anin(struct di_anin_struct *analog_input);
unsigned my_di_digout(unsigned i);
int my_di_anout(unsigned dac_data,unsigned range);
unsigned my_di_mode(struct di_mode_struct *mode);

int my_di_start_scan(void);
int my_di_stop_scan(void);

int my_di_inlist(struct di_inlist_struct *input_list);
int my_di_outlist(struct di_outlist_struct *output_list);
int my_di_list_length(unsigned in_length,unsigned out_length);

int my_di_burst_rate(unsigned rate);
int my_di_strerr(unsigned err_code,char *err_str);


unsigned my_di_buffer_size(void);
int my_di_copy_array(unsigned buf_index,const short *source,unsigned word_count);
int my_di_copy_buffer(unsigned buf_index,short *dest, unsigned word_count);


#else // __WIN32__


#define my_di_com           di_com
#define my_di_info32        di_info32

#define my_di_open          di_open
#define my_di_set_port_mode di_set_port_mode
#define my_di_set_data_mode di_set_data_mode
#define my_di_get_data_mode di_get_data_mode

//int my_di_reset(void);
#define my_di_close         di_close

#define my_di_buffer_status di_buffer_status
#define my_di_buffer_alloc  di_buffer_alloc
#define my_di_buffer_free   di_buffer_free

#define my_di_digin         di_digin
#define my_di_anin          di_anin
#define my_di_digout        di_digout
#define my_di_anout         di_anout
#define my_di_mode          di_mode

#define my_di_start_scan    di_start_scan
#define my_di_stop_scan     di_stop_scan

#define my_di_inlist        di_inlist
#define my_di_outlist       di_outlist
#define my_di_list_length   di_list_length

#define my_di_burst_rate    di_burst_rate
#define my_di_strerr        di_strerr


#define my_di_buffer_size   di_buffer_size
#define my_di_copy_array    di_copy_array
#define my_di_copy_buffer   di_copy_buffer


#pragma comment(lib, "di2s32.lib")

#endif


#endif // MYDI

