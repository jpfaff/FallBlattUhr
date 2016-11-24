/*
 * fallblatt.h
 *
 *  Created: 04.10.2016
 *  Author: Jonas Pfaff
 */
 
#ifndef fallblatt_H_
#define fallblatt_H_

#include <avr/io.h>

/*Settings. Change following according to pinout.*/
#define START_PORT  PORTC
#define START_DDR   DDRC
#define START_PIN   _BV(PC0)

#define ADL_PORT    PORTC
#define ADL_DDR     DDRC
#define ADL_PIN     _BV(PC1)

#define ADCM_PORT   PORTC
#define ADCM_DDR    DDRC
#define ADCM_PIN    _BV(PC3)

#define ADCH_PORT   PORTC
#define ADCH_DDR    DDRC
#define ADCH_PIN    _BV(PC2)

#define DATA0_PORT  PORTD
#define DATA0_DDR   DDRD
#define DATA0_PIN   PIND
#define DATA0       _BV(PD7)

#define DATA1_PORT  PORTD
#define DATA1_DDR   DDRD
#define DATA1_PIN   PIND
#define DATA1       _BV(PD6)

#define DATA2_PORT  PORTD
#define DATA2_DDR   DDRD
#define DATA2_PIN   PIND
#define DATA2       _BV(PD5)

#define DATA3_PORT  PORTD
#define DATA3_DDR   DDRD
#define DATA3_PIN   PIND
#define DATA3       _BV(PD4)

#define DATA4_PORT  PORTD
#define DATA4_DDR   DDRD
#define DATA4_PIN   PIND
#define DATA4       _BV(PD3)

#define DATA5_PORT  PORTD
#define DATA5_DDR   DDRD
#define DATA5_PIN   PIND
#define DATA5       _BV(PD2)  
/*Settings END*/

//define macros for easier understanding
#define SET_START (START_PORT |= START_PIN)
#define RESET_START (START_PORT &= ~START_PIN)

#define SET_ADL (ADL_PORT |= ADL_PIN)
#define RESET_ADL (ADL_PORT &= ~ADL_PIN)

#define SET_ADCM (ADCM_PORT |= ADCM_PIN)
#define RESET_ADCM (ADCM_PORT &= ~ADCM_PIN)

#define SET_ADCH (ADCH_PORT |= ADCH_PIN)
#define RESET_ADCH (ADCH_PORT &= ~ADCH_PIN)

#define READ_DATA0 (DATA0_PIN & DATA0)
#define READ_DATA1 (DATA1_PIN & DATA1)
#define READ_DATA2 (DATA2_PIN & DATA2)
#define READ_DATA3 (DATA3_PIN & DATA3)
#define READ_DATA4 (DATA4_PIN & DATA4)
#define READ_DATA5 (DATA5_PIN & DATA5)

//Functions
void fallblatt_init(void);
uint8_t set_positions(uint8_t hours, uint8_t minutes);

#endif /*fallblatt_H_*/
