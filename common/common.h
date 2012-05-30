#ifndef ZB_COMMON_H
#define ZB_COMMON_H

#define __AVR_ATtiny85__ 1
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>


//======== RGB Conf
//TIMER1 pour PWM RGB
#define masque_rgb		0x16
#define masque_irled	0x01
#define red_mask 		0x02
#define blue_mask 		0x04
#define green_mask		0x10

#define set_pwm_ir		0x40

#define RED_ON 		PORTB&= ~(red_mask)
#define RED_OFF 	PORTB|= red_mask
#define BLUE_ON 	PORTB&= ~(blue_mask)
#define BLUE_OFF 	PORTB|= blue_mask
#define GREEN_ON 	PORTB&= ~(green_mask)
#define GREEN_OFF 	PORTB|= green_mask
#define LED_OFF		PORTB|= masque_rgb
#define LED_ON		PORTB&= ~(masque_rgb)

//======== IR Conf
//TIMER0 pour Tx/Rx IR
#define T_START_MARQUE		9000
#define T_START_MARQUE_H	10800
#define T_START_MARQUE_L	7200
#define T_START_SPACE		4500
#define T_START_SPACE_H		5400
#define T_START_SPACE_L		3600
#define T_MARQUE			560
#define T_MARQUE_H			672
#define T_MARQUE_L			448
#define T_ONE_SPACE			1600
#define T_ONE_SPACE_H		1920
#define T_ONE_SPACE_L		1280
#define T_ZERO_SPACE		560
#define T_ZERO_SPACE_H		672
#define T_ZERO_SPACE_L		448
#define T_ONE_H				(T_MARQUE_H+T_ONE_SPACE_H)
#define T_ONE_L				(T_MARQUE_L+T_ONE_SPACE_L)
#define T_ZERO_H			(T_MARQUE_H+T_ZERO_SPACE_H)
#define T_ZERO_L			(T_MARQUE_L+T_ZERO_SPACE_L)

//======== IR traitement data
#define	send_masque			0x80000000
#define sort_masque			0x80000000
#define IDLE				0
#define RECEIVING			1 
#define START				2 
#define COM1				3
#define COM2				4
#define	DAT1				5 
#define DAT2				6
#define END_BIT				7
#define END_RCV				8
#define DESCENDANT			0
#define MONTANT				1
#define RECEIVE				2
#define EMMIT				3
//========= IR Data game
#define MY_ID				0x51
#define MY_ID_COMP			(~(MY_ID))
#define KILL_ORDER			0x22
#define KILL_ORDER_COMP		(~(KILL_ORDER))
#define DEAD				0x33

//========= GENERAL VALUES
#define TRUE				1
#define FALSE				0






// structures
typedef struct {	unsigned char red;
					unsigned char green;
					unsigned char blue;}couleur_RGB;

typedef struct {	unsigned long temps_precedant; 		//previous measured time
					unsigned char state_count;			//number of edge received
					unsigned char mode;					//the reception status
					unsigned char front_declencheur;	//triggering edge
					unsigned char error_encountered;	//well, simple to understand...
					unsigned char command1;				//received byte
					unsigned char command2;				//received byte
					unsigned char data1;				//received byte
					unsigned char data2;}ir_receive;	//received byte

volatile ir_receive ir_rcv;
volatile couleur_RGB couleur_en_cours;
volatile couleur_RGB next_couleur;
volatile couleur_RGB RGB_blue;
volatile couleur_RGB RGB_green;
volatile couleur_RGB RGB_red;
volatile couleur_RGB RGB_off;	
volatile couleur_RGB RGB_alternative_emt;
volatile couleur_RGB RGB_alternative_rcv;
/***********************************************************************************/
//====================== COMMON

inline void tempo_ten_us (unsigned long duree)
{
  unsigned int tampon=1;
   while (duree != 0)
    {
      tampon++;
      tampon--;
      tampon++;
      tampon--;
      duree--;
    }

}

void isr_button_off (void);
void isr_button_on (void);
void IR_rx_on_tx_off (void);
void IR_rx_off_tx_on (void);
void rgb_led_on (void);
void rgb_led_off (void);
void send_NEC(unsigned char ,unsigned char ,unsigned char ,unsigned char );
void send_NEC_char (unsigned char );
void rcv_IR (  volatile unsigned char * , unsigned char);
void reset_reception(void);
void write_data (unsigned char ,unsigned char );
void mark(unsigned int );
void space(unsigned int );
void analyse_temps(unsigned char );
void rgb_mode (void);
void init_colors(void);
// related to global

//========= GLOBAL VARIABLES DECLARATIONS
					
typedef struct {	unsigned char mode_fct;
					unsigned char alive;
					unsigned char rgb_timeout;
					unsigned char timeout;
					unsigned char role;}	global_state;		

volatile global_state state ;	
 
#endif
