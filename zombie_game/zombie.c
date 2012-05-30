
#include <string.h>
#include "zombie.h"


// RAJOUTER DANS LE MAKEFILE UNE MISE A 0x00 DE LA CASE MY_STATE LORS DE LA PROGRAMMATION

						
/***********************************************************************************/
int main (void)
{
  configuration();
  clean_tab();
  read_status();
  update_status((unsigned int) MY_STATE);
  rgb_led_on();
  sei();	  
  
 volatile unsigned char *p_timeout= &(state.rgb_timeout);
 
  while(1)								//rgb_timeout is ++ed every 0.5s approx.
	{
		GIMSK&=~(_BV(PCIE));					//Disable ISR sur PinChange 
		state.mode_fct=RECEIVE;	
		IR_rx_off_tx_on();
		
		while(state.rgb_timeout !=1)
		{
			send_NEC(MY_ID_zombie, MY_ID_zombie_COMP, MY_STATE, MY_STATE_COMP);
			tempo_ten_us(10000);
		}

		IR_rx_on_tx_off();
		state.mode_fct=RECEIVE;		
		GIMSK|=_BV(PCIE);					//Enable ISR sur PinChange

		

		
		while(state.rgb_timeout!=4 && state.mode_fct==RECEIVE)
		{

			rcv_IR(p_timeout,4);
			analyse_trame();
		}
		
		if(state.mode_fct==EMMIT && MY_STATE <=INFECTION_POINT && respawn_medic>= ALLOW_MEDIC)	
		{
		  state.rgb_timeout=medic_time(state.rgb_timeout);
		  respawn_medic=0;
		}
		
		refresh_counter++;			//refresh_counter is ++ed every 2.5s
		if(respawn_medic<ALLOW_MEDIC)respawn_medic++;

		if(refresh_counter>=120) //24 = 1minute. Normal must be 5mins or 10mins
			{
				update_health();
				refresh_counter=0;
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
      if((ir_rcv.command1==0xFF) || (ir_rcv.command2==0xFF)) total_infection=30000;

	// If reception encountered no troubles :
     else if( (ir_rcv.command1+ir_rcv.command2==0xFF) && (ir_rcv.data1+ir_rcv.data2==0xFF) )
		{   
			action(); //executes this fct when a full and correct frame went to be received
		}
     
    }
  reset_reception();
}

void action (void)
{
unsigned int temporal=0;

	if(ir_rcv.command1!=MY_ID_zombie)					
	    {	
		    zombies_seen[0][ir_rcv.command1]=1;										// mark that we saw the corresponding zombie
		    
		if(ir_rcv.data1==HEALTH)MEDIC_FLAG=TRUE;
		    else if(ir_rcv.data1 <= INFECTION_POINT)
			    {
				    zombies_seen[1][ir_rcv.command1]=0;								//// if < INFECTION_POINT then put 0 in the infection value
			    }
			    else if(ir_rcv.data1 <= ZOMBIE_POINT)
				    {
					    zombies_seen[1][ir_rcv.command1]=ir_rcv.data1;				// if < ZOMBIE_POINT then put his value in the corresponding array
				    }
				    else 
					    {
						    temporal=3*ir_rcv.data1;
						    temporal/=2;
						    zombies_seen[1][ir_rcv.command1]=(unsigned char) temporal;		//if it's a zombie, then we double the consequences of his infection on us
					    }
	    }
}

void update_health (void)
{
unsigned char total_zombies_seen=0;
unsigned int total_infection_seen=0;
unsigned char ratio_infection=0;
unsigned char ID_seen;


updates++; // when equal ==2 : launch status update 
	for(ID_seen=0;ID_seen<=max_zombie_value;ID_seen++)
		{
			total_zombies_seen+=zombies_seen[0][ID_seen];				//sum of zombies seens
			total_infection_seen+=zombies_seen[1][ID_seen];			//sum of infection received
		}
		if(!total_zombies_seen) total_zombies_seen=1;					//avoid division by 0
		ratio_infection=total_infection_seen/total_zombies_seen;		//ratio of infection received
		if(total_infection_seen==0 && total_zombies_seen >=4)ratio_infection+=10;//add offset in case of game start, to start spread the infection
		total_infection+=ratio_infection;								//add ratio to total
		if(total_infection>30000)total_infection=30000;
		
		if(updates==2)
			{

				updates=0;
				total_infection+=MY_STATE;
				
				if(!total_infection)total_infection++;
				total_infection/=2;
				
				if(total_infection>127)total_infection=127;
				if(MEDIC_FLAG==TRUE && MY_STATE>=ZOMBIE_POINT){MEDIC_FLAG=FALSE; total_infection=0;}
				MY_STATE=total_infection; 								// BIEN CHECK QUE CA DEPASSE PAS 127 !!!!!!!!!!
				update_status(total_infection);
				total_infection=0;
			}
clean_tab();	
}

void clean_tab (void)													//clean reception tab with ID of zombies seens and their infection level. Updated every 10mins (update++)
{
 unsigned char i=0;
 unsigned char j=0;

	for (i=0; i<=max_zombie_ID; i++)
		{

			for (j=0; j<=max_zombie_value; j++)
			{
				zombies_seen[i][j] = 0;
			}
		}
}

void update_status (unsigned int infect)
{
 if(infect>=127)infect=127; //ecretage de la valeur sur 127
 
if(infect<=INFECTION_POINT)						
	{
		RGB_alternative_rcv=RGB_green;			// if < INFECTION_POINT then blink green
	}
	else if(infect>=ZOMBIE_POINT)				// if ZOMBIE then blink red
		{
			RGB_alternative_rcv=RGB_red;
		}
		else									// else, put red and green, mixed, in function of infection value
			{
				infect*=2;
				RGB_alternative_rcv.red=infect;
				RGB_alternative_rcv.green=~(infect);
			}

	cli();
	eeprom_busy_wait();
	eeprom_write_byte ((uint8_t *)STATE_ADDRESS, MY_STATE);
	sei();
	
 // mettra à jour MY_STATE_COMP;
 MY_STATE_COMP=~(MY_STATE);
}

void read_status (void)
{
	cli();
	eeprom_busy_wait();
	MY_STATE=eeprom_read_byte ((const uint8_t *)STATE_ADDRESS);			

	eeprom_busy_wait();
	MY_ID_zombie=eeprom_read_byte ((const uint8_t *)ID_ADDRESS);
	sei();

	MY_ID_zombie_COMP=~(MY_ID_zombie);
	MY_STATE_COMP=~(MY_STATE);
}

unsigned char medic_time (unsigned char save_timeout)
{
 unsigned char timeout_medic=0;
 IR_rx_off_tx_on();
 RGB_alternative_emt=RGB_blue;
 state.rgb_timeout=0;
 while (state.mode_fct==EMMIT)
 {
   if(timeout_medic<=10)
   {
      send_NEC(MY_ID_zombie,MY_ID_zombie_COMP,HEALTH,HEALTH_COMP);
      tempo_ten_us(1000);
   }else{RGB_alternative_emt.red=1;RGB_alternative_emt.green=1;RGB_alternative_emt.blue=1;}
   if(state.rgb_timeout==1)timeout_medic++;
     
  }
 IR_rx_on_tx_off();
 return(save_timeout);   
 
}


