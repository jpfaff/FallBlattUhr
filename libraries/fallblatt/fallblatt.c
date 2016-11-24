/*
 * fallblatt.h
 *
 *  Created: 04.10.2016
 *  Author: Jonas Pfaff
 */

#include "fallblatt.h"       
#include <util/delay.h>

void fallblatt_init(void)
{
    START_DDR |= START_PIN;   //START     out
    ADL_DDR |= ADL_PIN;   //SET_ADL   out
    ADCH_DDR |= ADCH_PIN;   //SET_ADCH  out
    ADCM_DDR |= ADCM_PIN;   //SET_ADCM  out

    DATA0_DDR &= ~DATA0;  //PIN0..5   input with pull-up
    DATA0_PORT |= DATA0;
    DATA1_DDR &= ~DATA1;
    DATA1_PORT |= DATA1;
    DATA2_DDR &= ~DATA2;
    DATA2_PORT |= DATA2;
    DATA3_DDR &= ~DATA3;
    DATA3_PORT |= DATA3;
    DATA4_DDR &= ~DATA4;
    DATA4_PORT |= DATA4;
    DATA5_DDR &= ~DATA5;
    DATA5_PORT |= DATA5;   
}

uint8_t set_positions(uint8_t hours, uint8_t minutes)
{
    uint8_t value[6];
    uint8_t number_h = 0;
    uint8_t number_m = 0;
    uint8_t wanted_h = 0;
    uint8_t wanted_m = 0;
    uint8_t done_m = 0;
    uint8_t done_h = 0;
    
    if(minutes >= 0 && minutes < 31) wanted_m = minutes + 1;
    else if(minutes >= 31 && minutes < 60) wanted_m = minutes + 2;
    else wanted_m = 1;
    
    if(hours >= 0 && hours < 25) wanted_h = hours + 1;
    else wanted_h = 1;
    
    while(done_m != 1 || done_h != 1)
    {
        if(done_h != 1)
        {
            //CHECK HOURS
            _delay_us(50); 
            SET_ADL;
            SET_ADCH;
            _delay_us(50);
            value[0] = READ_DATA0;
            value[1] = READ_DATA1;
            value[2] = READ_DATA2;
            value[3] = READ_DATA3;
            value[4] = READ_DATA4;
            value[5] = READ_DATA5;
            
            for(uint8_t i=0; i<6; i++)
            {
                if(value[i] == 0) value[i] = 1;
                else value[i] = 0;
                number_h ^= (-value[i] ^ number_h) & (1 << i);
            }
            
            if(number_h != wanted_h)
            {
                SET_START;
                _delay_us(50);
                RESET_ADL;
                _delay_us(50);
                RESET_ADCH;
                RESET_START;
            }
            else if(number_h == wanted_h)
            {
                RESET_ADL;
                RESET_ADCH;
                done_h = 1;
            }
        }
    
        if(done_m != 1)
        {
            //CHECK MINUTES 
            _delay_us(50);
            SET_ADL;
            SET_ADCM;
            _delay_us(50);
            value[0] = READ_DATA0;
            value[1] = READ_DATA1;
            value[2] = READ_DATA2;
            value[3] = READ_DATA3;
            value[4] = READ_DATA4;
            value[5] = READ_DATA5;
            
            for(uint8_t i=0; i<6; i++)
            {
                if(value[i] == 0) value[i] = 1;
                else value[i] = 0;
                number_m ^= (-value[i] ^ number_m) & (1 << i);
            }
            
            //If number is not wanted restart motor. Else reset signals but keep motor off.        
            if(number_m != wanted_m)
            {
                SET_START;
                _delay_us(50);
                RESET_ADL;
                _delay_us(50);
                RESET_ADCM;
                RESET_START;
            }
            else
            {
                RESET_ADL;
                RESET_ADCM;
                done_m = 1;
            }
        }
    }
    
    return 0;
}
