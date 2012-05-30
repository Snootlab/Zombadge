#include "survivor.h"


int main (void)
{
  configuration();
  sei();
  
 while(1)
    {
 
		  if(state.mode_fct==RECEIVE&&state.alive==TRUE)				//while we are in RECEIVE mode (no action on the button) and still alive, we are waiting to receive IR transmissions
		{	
		  IR_rx_on_tx_off();
		  next_couleur.red=0;next_couleur.green=255;next_couleur.blue=0;// turn on the green led for half a second
		  rgb_led_on();			
		  while(state.mode_fct==RECEIVE&&state.alive==TRUE)
			{
			  rcv_IR(limit_default_ptr,0);
			  analyse_trame();
			}
		  rgb_led_off();
		}else if(state.mode_fct==EMMIT&&state.alive==TRUE)					//while the button is pushed, we are alive and allowed to kill, emmit and change the RBG cycle
		  {
			
			IR_rx_off_tx_on();
			next_couleur.red=255;next_couleur.green=0;next_couleur.blue=0;// turn on the red led for half a second
			rgb_led_on();	
			tempo_ten_us(150000);			
			while(state.mode_fct==EMMIT&&state.alive==TRUE)				//while the button is pushed, we are alive and not allowed to kill, not emmit and change the RBG cycle
			  {
				send_NEC(MY_ID,MY_ID_COMP,KILL_ORDER,KILL_ORDER_COMP);
				tempo_ten_us(4000);
			  }
			rgb_led_off();
		  } else															//when we are dead, change the RGB cycle and put the state.alive = FALSE (not allowed to emmit/receive anymore)
			{
				IR_rx_off_tx_on();
				next_couleur.red=0;next_couleur.green=0;next_couleur.blue=255;// turn on the blue led for half a second
				rgb_led_on();		
				while(1);
			}
    }
}
/**********************************************************************************
***********************************************************************************
***************    F U N C T I O N S    D E F I N I T I O N S    ******************
***********************************************************************************
***********************************************************************************/


/***********************************************************************************/
//====================== CONFIGS & INTERRUPTIONS RELATED
void configuration(void)
{
	DDRB=(masque_rgb|masque_irled);  
	OCR0A  = 104;						//setting the value for the pulse generation by TIMER0
	PORTB=0x17;							//turn off RGB led and IR led
	GIMSK|=_BV(PCIE);					//Enable ISR sur PinChange
	PCMSK|=_BV(PCINT5);				//Set changes on PIN5 as ISR sources
	state.mode_fct = RECEIVE; 			//init of the differents structures
	state.alive = TRUE;
	state.rgb_timeout = 0;
	ir_rcv.temps_precedant=0; 
	ir_rcv.state_count=0;
	ir_rcv.mode=IDLE;
	ir_rcv.front_declencheur=DESCENDANT;
	ir_rcv.error_encountered=FALSE;
	ir_rcv.command1=0;
	ir_rcv.command2=0;
	ir_rcv.data1=0;
	ir_rcv.data2=0;
	init_colors();
}

/*
After quitting the rcv_IR, this function analyse what we received.
If error = 1, let's reset the reception to his beginning configuration.
If the data frame's lenght is normal and no error encoutered, then do the data error detection (complementarity).
If no error in data received is detected, then do the given action.
*/
void analyse_trame(void)
{
  if(ir_rcv.error_encountered!=FALSE)
    {
      reset_reception();
    }
  else if(ir_rcv.state_count==67 && ir_rcv.mode==END_RCV)
    {
	// If reception encountered no troubles :
      if( (ir_rcv.command1+ir_rcv.command2==0xFF) && (ir_rcv.data1+ir_rcv.data2==0xFF) )
		{

		if(ir_rcv.data1==(unsigned char)KILL_ORDER)
			{
			  tempo_ten_us(2000000);
			  rgb_led_off();			//Stop the ISR
			  state.alive=DEAD;		//Z-(ombadge) is dead baby, Z-(ombadge) is dead...
			} 
		}
     
    }
  reset_reception();
}
