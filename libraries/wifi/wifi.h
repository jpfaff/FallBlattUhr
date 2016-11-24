/*
 * wifi.h
 *
 *  Created: 12.10.2016
 *  Author: Jonas Pfaff
 */
 
#ifndef wifi_H_
#define wifi_H_

#include <avr/io.h>

//Functions
void ap_create(void);
uint8_t wifi_connect_startup(void);
uint8_t wifi_connect(char *ssid, char *pw);
void setup_server(void);
void load_website(char ch, uint8_t NTP, uint8_t RTC, uint8_t wifi, uint8_t hour, uint8_t minute, uint8_t gmt);
uint8_t connect_NTP(uint8_t* values);

#endif /*wifi_H_*/
