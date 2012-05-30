#include "common.h"

volatile unsigned char interrupt_occured=FALSE;
volatile unsigned char temps_mesure=0;

// related to RGB
volatile unsigned char rgb_count=0;


/*
// related to IR
unsigned char state_count=0;
volatile unsigned char count=0;
*/

void isr_button_off (void)
{
  PCMSK&=~(_BV(PCINT5));	//REMOVE PINB5 ISR
}
void isr_button_on (void)
{
  PCMSK|=_BV(PCINT5);		//SET PINB5 ISR
}


/***********************************************************************************/
//====================== IR & RGB LED RELATED
/*
WARNING !!!
Due to TIMER0 share in IR transmission, we CAN'T receive and transmit in THE SAME TIME !
So, emmit when you want and the rest of time, wait for reception.
*/
void IR_rx_on_tx_off (void)
{
  PCMSK|=_BV(PCINT3);	//Pin setting the ISR pin change = PB3 == IR reception
  TCCR0B &= ~(_BV(WGM02));//going in normal count mode
  TCCR0A &= ~(_BV(WGM01));
  TCCR0A &= ~(_BV(WGM00));
  TCCR0B&=~(_BV(CS02));
  TCCR0B |= _BV(CS00);	//clk/64
  TCCR0B |= _BV(CS01);
  TCNT0=0;
	
}

void IR_rx_off_tx_on (void)
{
  PCMSK&=~(_BV(PCINT3));		//disable interruption caused by IR reception
  TCCR0B &= ~(_BV(WGM02));	//going in CTC mode for emmit
  TCCR0A |= _BV(WGM01);	
  TCCR0A &= ~(_BV(WGM00));
  TCCR0B&=~(_BV(CS02));
  TCCR0B |= _BV(CS00);	//clk/1
  TCCR0B &= ~(_BV(CS01));
  TCNT0=0;
}

void rgb_led_on (void)
{
  couleur_en_cours=next_couleur; //load the chosen colour (next_couleur) then launch the timer
  TCCR1|=_BV(CS10);  //clk/64
  TCCR1|=_BV(CS11);  
  TCCR1|=_BV(CS12); 
  TCCR1&=(~(_BV(CS13)));  
  TIMSK|=_BV(TOIE1);	//enable timer1 ovf interrupt
  TCNT1=0;
  state.rgb_timeout=0;
  LED_ON;
}

void rgb_led_off (void)
{
  couleur_en_cours.red=0; couleur_en_cours.green=0; couleur_en_cours.blue=0;
  TCCR1&=0xF0;  // stop Timer1
  TIMSK&=~(_BV(TOIE1));	//disable timer1 ovf interrupt
  TCNT1=0;
  state.rgb_timeout=0;
  LED_OFF;
}

		
/***********************************************************************************/
//====================== IR SEND & RECEIVE RELATED 
/*
Send via infrared, following the NEC protocol, four bytes.
Remember, in order to respect the protocol, COM1+COM2 = 0xFF and DAT1+DAT2 = 0xFF.
*/
void send_NEC(unsigned char command_1,unsigned char command_2,unsigned char data_1,unsigned char data_2)
{
  
  mark(T_START_MARQUE);
  space(T_START_SPACE);
		
  send_NEC_char(command_1);
  send_NEC_char(command_2);
  send_NEC_char(data_1);
  send_NEC_char(data_2);
		
  mark(T_MARQUE);
  space(1);			
}

void send_NEC_char (unsigned char send_data) // send a character LSB first
{
  unsigned char decalage =0;
  for(decalage=0;decalage<=7;decalage++)
    {
      if(send_data&(1<<decalage))
		{
		  mark(T_MARQUE);
		  space(T_ONE_SPACE);
		}
      else
		{
		  mark(T_MARQUE);
		  space(T_ZERO_SPACE);					
		}
    }
}



void rcv_IR (volatile unsigned char *limit_var, unsigned char limit_val)
{
  unsigned long provisoire=0;
  while(ir_rcv.mode!=END_RCV&&ir_rcv.error_encountered==FALSE&&state.mode_fct==RECEIVE&&(*limit_var)!=limit_val)
    {
		
		  if(interrupt_occured)
		{
		  interrupt_occured=FALSE; // reset the interrupt occured flag
		  
		  if ((ir_rcv.mode==IDLE)&&(ir_rcv.front_declencheur==DESCENDANT))
			{
			  provisoire = temps_mesure;
			  provisoire*=1024; //1024 due to clk timer = clk/1024
			  provisoire/=8;	//8 dut to clk sys = 8MHz so divide by 8 put the value in microseconds
			  
			  if(((PINB&0x08)!=0)&& (provisoire<=T_START_MARQUE_H) && (provisoire >=T_START_MARQUE_L))
				{		 // waiting for receiver falling edge
				  ir_rcv.mode=RECEIVING;
				  ir_rcv.front_declencheur ^=1;
				  ir_rcv.state_count++;
				}else ir_rcv.error_encountered=TRUE;											
			} else //Edge 2 then 4.....end
			{	
			  ir_rcv.front_declencheur ^=1;
			  analyse_temps(temps_mesure);
			}
		}
    }
}


void reset_reception(void){
  ir_rcv.temps_precedant=0;
  ir_rcv.state_count=0;
  ir_rcv.mode=IDLE;
  ir_rcv.front_declencheur=DESCENDANT;
  ir_rcv.error_encountered=FALSE;
  ir_rcv.command1=0;
  ir_rcv.command2=0;
  ir_rcv.data1=0;
  ir_rcv.data2=0;
  TCCR0B|=_BV(CS02);TCCR0B|=_BV(CS00);TCCR0B&=~(_BV(CS01));			
}


void write_data (unsigned char bit,unsigned char write_count){
  if(bit==1){
    switch (ir_rcv.mode)
      {
      case (COM1):
	write_count=((write_count/2)-2);// on this way, write_count = 0 for the first bit
	ir_rcv.command1|=_BV(write_count);
	break;
      case (COM2):
	write_count=((write_count/2)-10);// on this way, write_count = 0 for the first bit
	ir_rcv.command2|=_BV(write_count);
	break;		
      case (DAT1):
	write_count=((write_count/2)-18);// on this way, write_count = 0 for the first bit
	ir_rcv.data1|=_BV(write_count);
	break;
      case (DAT2):
	write_count=((write_count/2)-26);// on this way, write_count = 0 for the first bit
	ir_rcv.data2|=_BV(write_count);
	break;				
      }
  }
  else {
    if (bit==0){
      switch (ir_rcv.mode){
      case (COM1):
	write_count=((write_count/2)-2);// on this way, write_count = 0 for the first bit
	ir_rcv.command1&=~(_BV(write_count));
	break;
	
      case (COM2):
	write_count=((write_count/2)-10);// on this way, write_count = 0 for the first bit
	ir_rcv.command2&=~(_BV(write_count));
	break;		
      case (DAT1):
	write_count=((write_count/2)-18);// on this way, write_count = 0 for the first bit
	ir_rcv.data1&=~(_BV(write_count));
	break;
      case (DAT2):
	write_count=((write_count/2)-26);	// on this way, write_count = 0 for the first bit
	ir_rcv.data2&=~(_BV(write_count));
	break;				
      }
    }
    else{
      ir_rcv.error_encountered=TRUE; 
    }
  }
}  


void mark(unsigned int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  TCNT0=0;
  TCCR0A |= _BV(COM0A0);   // turn on OC0A PWM output
  tempo_ten_us (time/10);
}

void space(unsigned int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TCNT0=0;
  TCCR0A &= ~(_BV(COM0A0));  // turn off OC0A PWM output
  tempo_ten_us (time/10);
}

/***********************************************************************************/
//====================== INTERRUPTION SUBROUTINES RELATED
/*
This interruption occurs when there is a level change on the selected pins 
for the source selection, see isr_button_off/on and IR_rx_off_tx_on/IR_rx_on_tx_off
Then we have to know wich pin caused the interruption and do the action assigned to this new state.
Only used for IR tx/rx and button action.
*/
ISR (PCINT0_vect) {

	 if(PINB&0x20)
		{
			if(state.mode_fct==EMMIT)state.mode_fct=RECEIVE;
			else
				{
					temps_mesure=TCNT0;
					TCNT0=0;
					interrupt_occured=TRUE;
				}
		}else
			{
				if(state.mode_fct==RECEIVE)  state.mode_fct=EMMIT;
			}
	 tempo_ten_us(10);	 
}



/*
This interruption occurs when the TIMER1 is in overflow (255 before start again from 0).
It's used at a time unit for led blinking's time.
Only used for the RGB led.
*/
ISR (TIM1_OVF_vect)
{
  rgb_count++; //number of times where the interruption occured

  if(couleur_en_cours.red==0)
    {
      RED_OFF;
    } 
  else 	
    couleur_en_cours.red--;
  if(couleur_en_cours.green==0)
    {
      GREEN_OFF;
    } else 	couleur_en_cours.green--;
  if(couleur_en_cours.blue==0)
    {
      BLUE_OFF;
    } else 	couleur_en_cours.blue--;
		
  if(rgb_count==255)
    {
      rgb_count=0;
      state.rgb_timeout++;
      rgb_mode();
      couleur_en_cours=next_couleur;
    }
  TCNT1=0;

}


void analyse_temps(unsigned char temps){
  unsigned long time_us=0;
  unsigned char write_bit=0;

  if((ir_rcv.mode==RECEIVING)&&(ir_rcv.front_declencheur==DESCENDANT)&&(ir_rcv.state_count==1)) //ETAT 2
    {
      time_us=((unsigned long)temps*128);//put time in µs (time * 1024(=clk/div)/8(=to put in microseconds)
      if(time_us<=T_START_SPACE_H && time_us>=T_START_SPACE_L) 
	{

	  TCCR0B|=_BV(CS01);TCCR0B|=_BV(CS00);TCCR0B&=~(_BV(CS02));		//set clk/64 for the rest of the transmission	
	  TCNT0=0;
	  ir_rcv.mode=COM1;
	  ir_rcv.state_count++;	
	}
      else  {
	ir_rcv.error_encountered=TRUE; 
      }
    }
  else // EDGE 4.....end
    {
      time_us=((unsigned long)temps*8); //put time in µs (time * 64(=clk/div)/8(=to put in microseconds)
      if((time_us<=T_MARQUE_H && time_us>=T_MARQUE_L) || (time_us<=T_ONE_SPACE_H && time_us>=T_ONE_SPACE_L) || (time_us<=T_ZERO_SPACE_H && time_us>=T_ZERO_SPACE_L))
	{
	  
	  ir_rcv.state_count++;
	  
	  if((ir_rcv.state_count%2==0) && ir_rcv.state_count <67) //if state_count is even : analyse the pulse duration and write the corresponding bit in the data received structure
	    {	write_bit=0;
	      if((time_us+ir_rcv.temps_precedant)<=T_ONE_H &&(time_us+ir_rcv.temps_precedant)>=T_ONE_L)
		{
		  write_bit=1;
		  write_data(write_bit,ir_rcv.state_count);
		}
	      else if ((time_us+ir_rcv.temps_precedant)<=T_ZERO_H &&(time_us+ir_rcv.temps_precedant)>=T_ZERO_L)
		{
		  write_bit=0;
		  write_data(write_bit,ir_rcv.state_count);
		}else ir_rcv.error_encountered=TRUE; 
	    }
	  
	  switch (ir_rcv.state_count)		// used to know where we are in the reception field, wich byte we are receiving
	    {
	    case (18): 
	      ir_rcv.mode=COM2;
	      break;
	    case (34) : 
	      ir_rcv.mode=DAT1;
	      break;
	    case (50) :
	      ir_rcv.mode=DAT2;
	      break;
	      
	    case (66) :
	      ir_rcv.mode=END_BIT;
	      break;					
	      
	    case (67) :
	      ir_rcv.mode=END_RCV;
	      break;		
	      
	    default:
	      break;
	      
	    }					
	}
      else{
	ir_rcv.error_encountered=TRUE; 
      }
    }
  ir_rcv.temps_precedant=time_us;	
}

void rgb_mode (void)
/*
At every end of counting (TCNT1 have done the 255 round 255 times), we turn on/turn off the led, timing is defined
by the value of state.rgb_timeout before turning off the led and turning it on again.
*/
{
if(state.alive==DEAD)
	{
		if(state.rgb_timeout==1)
			{
			  next_couleur.red=0;next_couleur.green=0;next_couleur.blue=0;
			}
			  else if (state.rgb_timeout == 12)
			{
			  next_couleur=RGB_blue;
			  state.rgb_timeout =0;
			  LED_ON;
			}
	}
	else if(state.mode_fct==RECEIVE)
		{
			  if(state.rgb_timeout==1)
			{
			  next_couleur.red=0;next_couleur.green=0;next_couleur.blue=0;
			}
			  else if (state.rgb_timeout == 5)
			{
			  next_couleur=RGB_green;
			 if((RGB_alternative_rcv.red!=RGB_off.red)||(RGB_alternative_rcv.green!=RGB_off.green)||(RGB_alternative_rcv.blue!=RGB_off.blue))next_couleur=RGB_alternative_rcv;			//test. Final :  simplify  "if" after validation
			  state.rgb_timeout =0;
			  LED_ON;
			}
		}
	else if(state.mode_fct==EMMIT)	
		{
			  if(state.rgb_timeout==1)
			{
			  next_couleur.red=0;next_couleur.green=0;next_couleur.blue=0;
			}
			  else if (state.rgb_timeout == 2)
			{
			  next_couleur=RGB_red;
			  if((RGB_alternative_emt.red!=RGB_off.red)||(RGB_alternative_emt.green!=RGB_off.green)||(RGB_alternative_emt.blue!=RGB_off.blue))next_couleur=RGB_alternative_emt;			//test. Final :  simplify  "if" after validation
			  state.rgb_timeout =0;
			  LED_ON;
			}	
		}

}

void init_colors(void)
{
 RGB_blue.red=0;RGB_blue.green=0;RGB_blue.blue=255;
 RGB_green.red=0;RGB_green.green=255;RGB_green.blue=0;
 RGB_red.red=255;RGB_red.green=0;RGB_red.blue=0;
 RGB_off.red=0;RGB_off.green=0;RGB_off.blue=0;
 RGB_alternative_emt.red=0;RGB_alternative_emt.green=0;RGB_alternative_emt.blue=0;
 RGB_alternative_rcv.red=0;RGB_alternative_rcv.green=0;RGB_alternative_rcv.blue=0;
}
