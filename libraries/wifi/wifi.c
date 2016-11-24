/*
 * wifi.h
 *
 *  Created: 12.10.2016
 *  Author: Jonas Pfaff
 */

#include "wifi.h"
#include "uart/uart.h"
#include <stdlib.h>

void ap_create(void)
{
    uart_flush();
    uart_puts("AT+RST\r\n");
    uart_wait_for("ready", 5, 1000);
    uart_puts("AT+CWMODE=3\r\n");
    uart_wait_for("OK", 2, 1000);
    uart_puts("AT+CWSAP=\"FallBlattUhr\",\"\",6,0\r\n");
    uart_wait_for("OK", 2, 1000);
    uart_puts("AT+CIPAP=\"192.168.4.1\"\r\n");
    uart_wait_for("OK", 2, 1000);
}

uint8_t wifi_connect_startup(void)
{
    uart_flush();
    uart_puts("AT+RST\r\n");
    if(uart_wait_for("WIFI GOT IP",11,8000) == 0)
    {
        uart_puts("AT+CWMODE=1\r\n");
        uart_wait_for("OK", 2, 1000);
        return 0;
    }
    ap_create();
    return 1;
}

uint8_t wifi_connect(char *ssid, char *pw)
{
    uart_flush();
    uart_puts("AT+CWJAP=\"");
    uart_puts(ssid);
    uart_puts("\",\"");
    uart_puts(pw);
    uart_puts("\"\r\n");
    if(uart_wait_for("WIFI GOT IP",11,8000) == 0)
    {
        uart_puts("AT+CWMODE=1\r\n");
        uart_wait_for("OK", 2, 1000);
        return 0;
    }
    ap_create();
    return 1;
}

void setup_server(void)
{
    uart_flush();
    uart_puts("AT+CIPMUX=1\r\n");
    uart_wait_for("OK", 2, 1000);
    uart_puts("AT+CIPSERVER=1,80\r\n");
    uart_wait_for("OK", 2, 1000);
}

void load_website(char ch, uint8_t NTP, uint8_t RTC, uint8_t wifi, uint8_t hour, uint8_t minute, uint8_t gmt)
{
    uart_puts("AT+CIPSENDEX=");
    uart_putc(ch);
    uart_puts(",2048\r\n");
    uart_wait_for(">",1,1000);
    uart_puts("HTTP/1.1 200 OK\r\n\r\n");
    uart_puts("<html><body><form method=\"get\">SSID <input type=\"text\" name=\"ssid\"><br>Password <input type=\"text\" name=\"pw\"><br><br><input type=\"submit\" value=\"Submit WiFi Settings\"></form>");
    if(wifi) uart_puts("<font color=\"green\">WIFI connected</font></br>");
    else uart_puts("<font color=\"red\">No WIFI connection</font></br>");
    uart_puts("<form method=\"get\">");
    if(NTP) uart_puts("<input type=\"radio\" name=\"how\" value=\"man\"> Manual<input type=\"radio\" name=\"how\" value=\"ntp\" checked> NTP<br><br>");
    else uart_puts("<input type=\"radio\" name=\"how\" value=\"man\" checked> Manual<input type=\"radio\" name=\"how\" value=\"ntp\"> NTP<br><br>");
    uart_puts("GMT <select name=\"gmt\">");
    for(uint8_t i=1;i<3;i++){
        if(i == gmt) 
        {
        char str[1];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\" selected>");
        uart_putc('+');
        uart_puts(str);
        uart_puts("</option>");
        }
        else
        {
        char str[2];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\">");
        uart_putc('+');
        uart_puts(str);
        uart_puts("</option>");
        }
    }
    uart_puts("</select><br>");
    uart_puts("<select name=\"h\">");
    for(uint8_t i=0;i<24;i++){
        if(i == hour) 
        {
        char str[2];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\" selected>");
        uart_puts(str);
        uart_puts("</option>");
        }
        else
        {
        char str[2];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\">");
        uart_puts(str);
        uart_puts("</option>");
        }
    }
    uart_putc('\\');
    uart_putc('0');
    uart_wait_for("SEND OK",7,5000);
    uart_puts("AT+CIPSENDEX=");
    uart_putc(ch);
    uart_puts(",2048\r\n");
    uart_wait_for(">",1,1000);
    uart_puts("</select>.<select name=\"m\">");
    for(uint8_t i=0;i<60;i++){
        if(i == minute) 
        {
        char str[2];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\" selected>");
        uart_puts(str);
        uart_puts("</option>");
        }
        else
        {
        char str[2];
        itoa(i,str,10);
        uart_puts("<option value=\"");
        uart_puts(str);
        uart_puts("\">");
        uart_puts(str);
        uart_puts("</option>");
        }
    }
    uart_puts("</select><br><br><input type=\"checkbox\" name=\"RTC\" value=\"Y\"");
    if(RTC) uart_puts(" checked");
    uart_puts("> Start Clock<br><br><input type=\"submit\" value=\"Submit\"></form></body></html>");
    uart_putc('\\');
    uart_putc('0');
    uart_wait_for("SEND OK",7,5000);
    uart_puts("AT+CIPCLOSE=");
    uart_putc(ch);
    uart_puts("\r\n");
    uart_wait_for("CLOSED",6,2000);
}

uint8_t connect_NTP(uint8_t* values)
{
    char min[3];
    char hour[3];
    char sec[3];
    char temp[18];
    uart_flush();
    uart_puts("AT+CIPSTART=4,\"TCP\",\"www.google.ch\",80\r\n");
    uart_wait_for("CONNECT", 7, 3000);
    uart_puts("AT+CIPSENDEX=4,2048\r\n");
    uart_wait_for(">", 1, 1000);
    uart_puts("GET / HTTP/1.1\r\n");
    uart_puts("Host: https://www.google.com\r\n");
    uart_puts("\r\n");
    uart_putc('\\');
    uart_putc('0');
    if(uart_wait_for("Date: ", 6, 5000) == 0)
    {
        uart_gets(temp, 18, 1000);
        uart_gets(hour, 3, 500);
        uart_gets(temp, 2, 1000);
        uart_gets(min, 3, 500);
        uart_gets(temp, 2, 1000);
        uart_gets(sec, 3, 500);
        values[0] = atoi(hour);
        values[1] = atoi(min);
        values[2] = atoi(sec);
    }
    if(uart_wait_for("4,CLOSED", 8, 1000) == 0) 
    {
        uart_flush();
        return 0;
    }
    uart_flush();
    return 1;
}
