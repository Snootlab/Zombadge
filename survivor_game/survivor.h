#ifndef ZB_SURVIVOR_H
#define ZB_SURVIVOR_H
#include "../common/common.h"

//========= FUNCTIONS DECLARATIONS
void configuration (void);
void analyse_trame(void);
void tempo_ten_us (unsigned long);
//related to end condition of rcv_IR
volatile unsigned char limit_default=1;
volatile unsigned char * limit_default_ptr=&(limit_default);

#endif
