#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define USART_BAUDRATE 38400
#define F_CPU 8000000UL
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

static volatile uint8_t UART_TxBuf[UART_TX_BUFFER_SIZE];
static volatile uint8_t UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile uint16_t UART_TxHead;
static volatile uint16_t UART_TxTail;
static volatile uint16_t UART_RxHead;
static volatile uint16_t UART_RxTail;
static volatile uint8_t UART_LastRxError;

void uart_init(void)
{
    UART_TxHead = 0;
	UART_TxTail = 0;
	UART_RxHead = 0;
	UART_RxTail = 0;
	
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t) UBRR_VALUE;
	
	// Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UCSR0B = _BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	sei();
	timeout_init();
}

void uart_putc(uint8_t data)
{
	uint16_t tmphead;

	tmphead  = (UART_TxHead + 1) & UART_TX_BUFFER_MASK;

	while ( tmphead == UART_TxTail ) {
		;/* wait for free space in buffer */
	}

	UART_TxBuf[tmphead] = data;
	UART_TxHead = tmphead;

	/* enable UDRE interrupt */
	UCSR0B    |= _BV(UDRIE0);

} /* uart_putc */

void uart_puts(const char *s )
{
	while (*s) {
		uart_putc(*s++);
	}

} /* uart_puts */

uint16_t uart_getc(void)
{
	uint16_t tmptail;
	uint8_t data;

	if ( UART_RxHead == UART_RxTail ) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART_RxBuf[tmptail];

	return (UART_LastRxError << 8) + data;

} /* uart0_getc */

uint16_t uart_available(void)
{
	return (UART_RX_BUFFER_SIZE + UART_RxHead - UART_RxTail) & UART_RX_BUFFER_MASK;
} /* uart0_available */

void uart_flush(void)
{
	UART_RxHead = UART_RxTail;
} /* uart0_flush */

uint16_t uart_peek(void)
{
	uint16_t tmptail;
	uint8_t data;

	if ( UART_RxHead == UART_RxTail ) {
		return UART_NO_DATA;   /* no data available */
	}

	tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;

	/* get data from receive buffer */
	data = UART_RxBuf[tmptail];

	return (UART_LastRxError << 8) + data;

} /* uart0_peek */

void timeout_init(void) {	/* call at startup, let it run until needed */
	TCCR1A = 0;		/* "Normal" mode, */
	TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10); /* prescale /1024 */
	return;
}
#define	TICKS_PER_MSEC (F_CPU/1024/1000)	/* ticks/millisec with prescale /1024 */
#define	reset_timeout() do { TCNT1 = 0; } while (0)
#define	timeout_event(TIMEOUT_MS) (TCNT1 >= TIMEOUT_MS*TICKS_PER_MSEC)

uint8_t uart_wait_for(const char* string, uint8_t size, int timeout) //timeout ms
{
    uint8_t i = 0;
    reset_timeout();
    
    while(!timeout_event(timeout))
    {
        if(uart_available() > 0)
        {
            uint16_t receive;
            receive = uart_getc();
            if((receive & 0x00FF) == string[i]) i++;
            else i = 0;
            if(i == size) return 0;
        }
    }
    return 1;
}

uint8_t uart_gets(char* string, uint8_t size, int timeout)
{
    uint8_t i = 0;
    reset_timeout();
    
    while(!timeout_event(timeout))
    {
        if(uart_available() > 0)
        {
            uint16_t receive;
            receive = uart_getc();
            string[i] = (char)(receive & 0x00FF);
            i++;
            if(i == size-1) 
            {
                string[i] = '\0';
                return 0;
            }
        }
    }
    return 1;
}

ISR(USART_RX_vect)
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;
 
    /* read UART status register and UART data register */ 
    usr  = UCSR0A;
    data = UDR0;
    
    /* */
    lastRxError = (usr & (_BV(FE0)|_BV(DOR0)) );
        
    /* calculate buffer index */ 
    tmphead = ( UART_RxHead + 1) & UART_RX_BUFFER_MASK;
    
    if ( tmphead == UART_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART_RxHead = tmphead;
        /* store received data in buffer */
        UART_RxBuf[tmphead] = data;
    }
    UART_LastRxError = lastRxError;   
}

ISR(USART_UDRE_vect)
{
    uint16_t tmptail;

    if ( UART_TxHead != UART_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART_TxTail + 1) & UART_TX_BUFFER_MASK;
        UART_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UDR0 = UART_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UCSR0B &= ~_BV(UDRIE0);
    }
}
