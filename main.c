#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"



#define FOSC 7372800UL// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

void USART_Init( unsigned int ubrr){
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1 << RXEN) | (1 << TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
}

unsigned char USART_Receive( void ){

	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

void USART_Transmit( unsigned char data ){
	
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR = data;
	
}

void USART_Transmits(char data[] ) {
	int i;

	for(i = 0; i < strlen(data); i++) {
		USART_Transmit(data[i]);
		//_delay_ms(1000);
	}
}

void debounce() {
	_delay_ms(200);
	GIFR = _BV(INTF0) | _BV(INTF1);
}

ISR(INT0_vect) {
	PORTB |= 0x0f;
	
	lcd_clrscr();
	lcd_puts("ALARM\nDEACTIVATED");
	debounce();
}

int main( void ){
	
	// Time to connect to services
	//_delay_ms(30000);
	PORTB = 0x0f;
	DDRB = 0x0f;
	
	
	DDRA = 0xfe;
	PORTA = 0xff;
	
	DDRC = _BV(0);
	PORTC = _BV(0);
	
	UDR = 0x00;
	
	DDRD = _BV(4);

	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 128;
	
	//interrupts
	MCUCR = _BV(ISC01) | _BV(ISC11);
	GICR = _BV(INT0);
	sei();

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	USART_Init ( MYUBRR );
	
	
	// First 13 to clear old data
	// Last 13 to submit message
	char call[] = {13, 'A', 'T', 'D', '0', '9', '9', '8', '4', '4', '3', '0', '5', '6', 13};
	char call_2[] = "\rATD0998443056\r";
	int flag = 0;
	int flag_sens = 0;
	
	lcd_puts("ARMED");
	
	while(1) {
		
		while(bit_is_clear(PINB, 4)){
			
		}
		
		if (!flag_sens) {
			USART_Transmits(call_2);
			flag_sens = 1;
			lcd_clrscr();
			lcd_puts("CALLING\nOWNER");
		}
		
		char tmp = USART_Receive();
		char tmp2 = USART_Receive();
		
		
		if((tmp == 'S' && tmp2 == 'T') || (tmp == 'T' || tmp2 == 'T') || (tmp == '9' || tmp2 == '9')){
			flag = 1;
			lcd_clrscr();
			lcd_puts("IDENTITY WELCOME\nCONFIRMED HOME");
		}
		
		if(((tmp == 'N' && tmp2 == 'O') || (tmp == 'O' && tmp2 == ' ')) && !flag){
			lcd_clrscr();
			lcd_puts("INTRUDER\nDETECTED !!!");
			PORTB = 0x00;
			flag = 0;
		}
		
		_delay_ms(1);
	}

	
	return 0;
}

/*
OK

+SIND: 5,1

+SIND: 2

NO CARRIER

+SIND: 6,1
*/


/*

+SIND: 9

OK

+STIN:9

NO CARRIER

+SIND: 6,1

*/