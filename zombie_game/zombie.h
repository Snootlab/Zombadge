#ifndef ZB_ZOMBIE_H
#define ZB_ZOMBIE_H
#include "../common/common.h"
#define INFECTION_POINT		5
#define ZOMBIE_POINT		90
#define max_zombie_ID		1
#define max_zombie_value	149
#define STATE_ADDRESS 		0
#define ID_ADDRESS 			1
#define HEALTH			254
#define HEALTH_COMP		1
#define ALLOW_MEDIC		1000
#define REMOTE_CODE_POWER	0x45
//========= FUNCTIONS DECLARATIONS

void update_health (void);
void update_status (unsigned int );
void analyse_trame(void);				// put in common maybe ?
void action(void);
void configuration(void);
void tempo_ten_us (unsigned long);
void clean_tab(void);
void read_status(void);
unsigned char medic_time (unsigned char);

//========= GLOBAL VARIABLES DECLARATIONS

unsigned char MY_STATE=0;
unsigned char MY_STATE_COMP=0xFF;
unsigned char MY_ID_zombie=0;
unsigned char MY_ID_zombie_COMP=0xFF;
unsigned char updates=0;
unsigned char refresh_counter=0; 					//when equal to 200 (approx 10mins past) do a refresh of our infection's state
unsigned int total_infection=0;
unsigned char MEDIC_FLAG=FALSE;
unsigned int respawn_medic=0;
volatile unsigned char zombies_seen[(max_zombie_ID+1)][(max_zombie_value+1)];

#endif
