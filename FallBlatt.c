#include <avr/io.h>         
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#include "pcf8563/PCF8563.h"
#include "fallblatt/fallblatt.h"
#include "uart/uart.h"
#include "wifi/wifi.h"

int main (void) 
{
   
    //Define Inputs and Outputs
    fallblatt_init();
    set_positions(4, 4);
    _delay_ms(100);
    
    /*Initialize PCF RTC to a standard time*/
    PCF_Init(0);
    
    PCF_DateTime dateTime;
    dateTime.second = 0;
    dateTime.minute = 0;
    dateTime.hour = 0;
    dateTime.day = 1;
    dateTime.weekday = 1;
    dateTime.month = 1;
    dateTime.year = 2016;
    
    _delay_ms(2000);

    PCF_SetDateTime(&dateTime);
    
    /*Initialize UART*/
    uart_init();
    
    /*Initialize other variables*/
    uint8_t changed = 1;
    uint8_t old_h = 0;
    uint8_t old_m = 0;
    
    uint8_t NTP_on = 1;
    uint8_t NTP_check = 1;
    uint8_t RTC_on = 1;
    uint8_t wifi_ok = 1;
    uint8_t gmt = 1;
    
    char serial_data[100];
    
    
    /*
        Try to connect to Wifi with data saved in EEPROM.
        If no data or not possible (timeout) create AP and disable NTP.
    */
    if(wifi_connect_startup() != 0)
    {
        wifi_ok = 0;
        NTP_on = 0;
    }
    setup_server();
    uart_flush();
    
    while(1) 
    {
        /*
            If new serial data has been received (over interrupt) then:
            Read out data and recognize if "GET /?" or "GET /" has been received
            GET /? --> New variables waiting
                Read out variables and save them.
                TODO: first wi-fi network config
                Send website
            GET / --> Only website requested
                Send website
        */
        if(uart_available() > 0)
        {
            if(uart_wait_for("+IPD,",5,5000) == 0)
            {
                uart_gets(serial_data, 100, 1000);
                if(strstr(serial_data, "GET /?"))
                {
                    if(strstr(serial_data, "ssid=")) //Wifi Settings
                    {
                        char *pch;
                        pch = strstr(serial_data, "ssid=");
                        char ssid[32];
                        uint8_t i = 0;
                        while(pch[i+5] != '&')
                        {
                            ssid[i] = pch[i+5];
                            i++;
                        }
                        pch = strstr(serial_data, "pw=");
                        char pw[32];
                        i = 0;
                        while(pch[i+3] != ' ')
                        {
                            pw[i] = pch[i+3];
                            i++;
                        }
                        if(wifi_connect(ssid, pw) == 0) wifi_ok = 1;
                        else wifi_ok = 0;
                    }
                    else    //Clock Settings
                    {
                        if(strstr(serial_data, "how=man"))
                        {
                            NTP_on = 0;
                        }
                        else 
                        {
                            NTP_on = 1;
                            NTP_check = 1;
                        }
                        
                        if(strstr(serial_data, "RTC=Y"))
                        {
                            RTC_on = 1;
                        }
                        else RTC_on = 0;
                        
                        char *pct;
                        pct = strstr(serial_data, "gmt=");
                        gmt = pct[4] - '0';
     
                        if(!NTP_on)
                        {      
                            char *pch;                     
                            pch = strstr(serial_data, "h=");
                            char number[2] = "00";
                            if((pch[3] < '0') || (pch[3] > '9')) number[1] = pch[2];
                            else
                            {
                                number[0] = pch[2];
                                number[1] = pch[3];
                            }
                            dateTime.hour = atoi(number);

                            pch = strstr(serial_data, "m=");
                            strncpy(number, "00", 2);
                            if((pch[3] < '0') || (pch[3] > '9')) number[1] = pch[2];
                            else
                            {
                                number[0] = pch[2];
                                number[1] = pch[3];
                            }                        
                            dateTime.minute = atoi(number);
                            dateTime.second = 0;
                            changed = 1;
                        }
                    }
                    load_website(serial_data[0], NTP_on, RTC_on, wifi_ok, dateTime.hour, dateTime.minute, gmt);
                }
                else if(strstr(serial_data, "GET /"))
                {
                    load_website(serial_data[0], NTP_on, RTC_on, wifi_ok, dateTime.hour, dateTime.minute, gmt);
                }
            }
        }
        
        /*
            If NTP is enabled connect to NTP server and get time. Save them
            if connection was succesfull.
            TODO: Do this every hour...
        */
        if(NTP_on && NTP_check)
        {        
            uint8_t values[3];
            if(connect_NTP(values) == 0)
            {
                if(values[1] == dateTime.minute-1)                      //If difference is less than 1 minute ahead
                {                                                       //adapt slowly, such that it does not
                    dateTime.second = 0;                                //rotate to the right number only for a
                    NTP_check = 1;                                      //short time.
                }
                else                                                    
                {                                                       
                    dateTime.second = values[2];                        
                    dateTime.minute = values[1]; 
                    dateTime.hour =  (values[0]+gmt)%24;   //GMT
                    //dateTime.day = 0; 
                    //dateTime.weekday = 0; 
                    //dateTime.month = 0; 
                    //dateTime.year = 0;
                    NTP_check = 0;
                }
                changed = 1;
            }            
        }
        
        /*
            If RTC is enabled and the values have been changed previously 
            save new values to RTC.
            If the values have not been changed read out time according to RTC
            and set changed.
        */
        if(RTC_on)
        {
            if(changed)
            {
                PCF_SetDateTime(&dateTime);
            }
            else
            {
                PCF_GetDateTime(&dateTime);
                if(dateTime.hour != old_h || dateTime.minute != old_m) changed = 1;
                if(dateTime.minute == 59 && dateTime.second == 0) NTP_check = 1;
            }
        }
        
        /*
            If the values have been changed during the loop set the display 
            to the new positions and clear the changed flag.
        */
        if(changed)
        {
            set_positions(dateTime.hour, dateTime.minute);
            old_h = dateTime.hour;
            old_m = dateTime.minute;
            changed = 0;   
        }

    }

    return 0;               
}

// x |= (1 << Bitnummer);  Hiermit wird ein Bit in x gesetzt
// x &= ~(1 << Bitnummer); Hiermit wird ein Bit in x geloescht 
