#ifndef ZB_DETECTIVE_H
#define ZB_DETECTIVE_H
#include "../common/common.h"
#define MY_ID				0x51
#define MY_ID_COMP			(~(MY_ID))
#define KILL_ORDER			0x22
#define KILL_ORDER_COMP		(~(KILL_ORDER))
#define DEAD				0x33
#define VICTIM				0x44
#define ASSASSIN			0x55

//========= FUNCTIONS DECLARATIONS
void configuration (void);
void analyse_trame(void);
void tempo_ten_us (unsigned long);
//related to end condition of rcv_IR
volatile unsigned char limit_default=1;
volatile unsigned char * limit_default_ptr=&(limit_default);
#endif
