/*

Title: IR Learning Remote Receiver
Author: Shawon M. Shahryiar

MCU: STC8A8K64S4A12
Clock: IRC 12MHz
*/


#include "STC8xxx.h"
#include "BSP.h"
#include "LCD.c"


#define sync_high     	5850    //approx 1.3 * 4500us
#define sync_low      	3150    //approx 0.7 * 4500us
#define one_high      	2200    //approx 1.3 * (2250 - 562.5)us
#define one_low       	1180    //approx 0.7 * (2250 - 562.5)us  
#define zero_high     	732     //approx 1.3 * (1125 - 562.5)us
#define zero_low      	394     //approx 0.7 * (1125 - 562.5)us

#define learn_off				0
#define learn_key_1			1
#define learn_key_2			2

#define debounce_delay	90
#define reset_delay			4000

#define LED_ON					P55_high
#define LED_OFF					P55_low

#define RELAY_1_ON			P16_high
#define RELAY_1_OFF			P16_low

#define RELAY_2_ON			P17_high
#define RELAY_2_OFF			P17_low

#define RESET_KEY				P50_get_input		
#define KEY_1						P52_get_input		
#define KEY_2						P53_get_input		


#define IAP_Base_Addr		0x0000    


uint8_t bits = 0; 
uint8_t learn_mode = 0;
uint8_t received = 0; 

uint8_t code_storage[4] = {0, 0, 0, 0};
uint16_t frames[33];


void setup(void);
void erase_frames(void);
uint8_t decode(uint8_t start_pos, uint8_t end_pos);
uint16_t decode_IR(void);
void blink_LED(uint16_t dly);
void update_storage(void);
void load_storage_data(void);
uint16_t make16(uint8_t hb, uint8_t lb);


void EXT_0_ISR(void)        
interrupt 0
{
  frames[bits] = TMR0_get_counter(); 
  bits++; 
  TMR0_start; 

  if(bits >= 33) 
  { 
    received = 1; 
    _disable_global_interrupt; 
    TMR0_stop; 
  } 
  
  TMR0_load_counter_16(0x0000); 
}


void main(void)
{
	uint8_t i = 0;
	uint8_t relay_1_state = 0;
	uint8_t relay_2_state = 0;
	
	uint16_t ir_code = 0;
	uint16_t temp_value = 0;
		
	char text[20];
    
  setup();
	
	//Read for stored code
	load_storage_data();
	
	LCD_text(0, 0, "IR Code:");
	LCD_text(0, 1, "R1:");
	LCD_text(9, 1, "R2:");

	load_storage_data();
	
  while(1)
  {		
	 if(received != FALSE)
	 {
		 //Read detected code
		 
		 ir_code = decode_IR();
		 
		 //Show the code
		 sprintf(text, "%04X ", ir_code);
		 LCD_text(12, 0, text);
		 
		 switch(learn_mode)
		 {
			 //Learn the IR code for relay 1
			 case learn_key_1:
			 {
				 code_storage[1] = (ir_code & 0x00FF);
				 code_storage[0] = ((ir_code & 0xFF00) >> 8);
				 update_storage();				 
				 LED_OFF;
				 learn_mode = learn_off;
				 break;
			 }
			 
			 //Learn the IR code for relay 2
			 case learn_key_2:
			 {
				 code_storage[3] = (ir_code & 0x00FF);
				 code_storage[2] = ((ir_code & 0xFF00) >> 8);
				 update_storage();
				 LED_OFF;
				 learn_mode = learn_off;
				 break;
			 }
			 //Compare the codes and change the relay states 
			 default:
			 {
				 temp_value =  make16(code_storage[0], code_storage[1]);
				 
				 if(ir_code == temp_value)
				 {
						relay_1_state = ~relay_1_state;
				 }
				 
				 temp_value =  make16(code_storage[2], code_storage[3]);
				 
				 if(ir_code == temp_value)
				 {
						relay_2_state = ~relay_2_state;
				 }				 
				 
				 ir_code = 0;
				 LED_OFF;
				 break;
			 }
		 }
		 
		 erase_frames();
		 _enable_global_interrupt;
	 }
	 
	 else
	 {
			//show the stored codes for the two relays
			temp_value =  make16(code_storage[0], code_storage[1]);
		  sprintf(text, "%04X ", temp_value);
		  LCD_text(3, 1, text);
			
		  temp_value =  make16(code_storage[2], code_storage[3]);
		  sprintf(text, "%04X ", temp_value);
	 	  LCD_text(12, 1, text);
		  
		 //Reset and erase the stored IR codes
		  if(!RESET_KEY)
			{
				delay_ms(reset_delay);
				if(!RESET_KEY)
				{
					LED_OFF;
					for(i = 0; i < 2; i++)
					{
						code_storage[i] = 0xFF;
						blink_LED(100);
					}
					
					IAP_erase(IAP_Base_Addr);
					LED_OFF;
				}
			}
			
			//Enter IR learn mode for Relay 1 or toggle Relay 1
			if(!KEY_1)
			{
				while(!KEY_1);
				delay_ms(debounce_delay);
				
				if(!KEY_1)
				{
					LED_OFF;
					learn_mode = learn_key_1;
					delay_ms(100);
					
					for(i = 0; i < 4; i++)
					{
						blink_LED(40);
					}
					
					LED_ON; 
				}
				else 
				{
					relay_1_state = ~relay_1_state;
				}		
			}
			
			//Enter IR learn mode for Relay 2 or toggle Relay 2
			if(!KEY_2)
			{
				while(!KEY_2);
				delay_ms(debounce_delay);
				
				if(!KEY_2)
				{
					LED_OFF;
					learn_mode = learn_key_2;
					delay_ms(100);
					
					for(i = 0; i < 4; i++)
					{
						blink_LED(40);
					}
					
					LED_ON; 
				}
				else  
				{
					relay_2_state = ~relay_2_state;
				}		
			}
			
			//Toggle Relay 1 state
			switch(relay_1_state)
			{
				case 0:
				{
					RELAY_1_OFF;
					break;
				}
				default:
				{
					RELAY_1_ON;
					break;
				}
			}
			
			//Toggle Relay 2 state
			switch(relay_2_state)
			{
				case 0:
				{
					RELAY_2_OFF;
					break;
				}
				default:
				{
					RELAY_2_ON;
					break;
				}
			}		
	 }	 
  };
}


void setup(void)
{
  CLK_set_sys_clk(IRC_24M, 2, MCLK_SYSCLK_div_1, MCLK_SYSCLK_no_output);
  
  erase_frames();
	
	P50_input_mode;
	P50_pull_up_enable;
	P52_input_mode;
	P52_pull_up_enable;
	P53_input_mode;
	P53_pull_up_enable;

	P16_push_pull_mode;
	P17_push_pull_mode;
	
	P55_push_pull_mode;
	
	IAP_clear;	
	
  TMR0_setup(TMR0_16_bit_non_auto_reload, \
             TMR0_sysclk, \
             TMR0_clk_prescalar_12T, \
             TMR0_ext_gate, \
             TMR0_no_clk_out);
  
  EXT_0_priority_0;
  EXT_0_falling_edge_detection_only;
  _enable_EXT_0_interrupt;
  _enable_global_interrupt;
	
  LCD_init();
  LCD_clear_home();
	
	LED_OFF;
}


void erase_frames(void)
{
	for(bits = 0; bits < 33; bits++)
	{
		frames[bits] = 0x0000;
	}

	TMR0_load_counter_16(0x0000);
  
	received = 0;
	bits = 0;
}


uint8_t decode(uint8_t start_pos, uint8_t end_pos)
{
	uint8_t value = 0;

	for(bits = start_pos; bits <= end_pos; bits++)
	{
		value <<= 1;
		
		if((frames[bits] >= one_low) && (frames[bits] <= one_high))
		{
			value |= 1;
		}
		
		else if((frames[bits] >= zero_low) && (frames[bits] <= zero_high))
		{
			value |= 0;
		}
		
		else if((frames[bits] >= sync_low) && (frames[bits] <= sync_high))
		{
			return 0xFF;
		}
	}

	return value;
}


uint16_t decode_IR(void)
{
	uint8_t ir_lb = 0;
	uint8_t ir_hb = 0;
	
	ir_hb = decode(2, 9);
	ir_lb = decode(18, 25);
	
	return make16(ir_hb, ir_lb);
}


void blink_LED(uint16_t dly)
{
	LED_ON;
	delay_ms(dly);
	LED_OFF;
	delay_ms(dly);
}


void update_storage(void)
{
	uint8_t i = 0;
	
	IAP_erase(IAP_Base_Addr);
	delay_ms(10);
	
	for(i = 0; i < 4; i++)
	{
		IAP_write((IAP_Base_Addr + i), code_storage[i]);    
	}
}


void load_storage_data(void)
{
	uint8_t i = 0;
	
	for(i = 0; i < 4; i++)
	{
		code_storage[i] = IAP_read((IAP_Base_Addr + i));    
	}
}


uint16_t make16(uint8_t hb, uint8_t lb)
{
	uint16_t temp = hb;
	
	temp <<= 8;
	temp |= lb;
	
	return temp;
}
