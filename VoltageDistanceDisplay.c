// Kevin Nelson
// Embedded Systems - ADC Voltage & Angle Distance Display
// Spring 2021

#include "stm32f4xx.h"
#include <stdio.h>

#define RS 0x20     							// PA5 mask for reg select
#define RW 0x40     							// PA6 mask for read/write
#define EN 0x80     							// PA7 mask for enable

void delayMs(int n);
void LCD_command(unsigned char command);
void LCD_data(char datawrite);
void LCD_init(void);
void Port_init(void);
void Send_A_String(char *StringOfCharacters);

int main (void) {
    uint16_t result;
		float voltage, distance;
		char Voltage_Buffer[18], Distance_Buffer[18];
		Port_init();
		LCD_init();

    while (1) {
				ADC1->CR2 |= 0x40000000;        															// start a conversion
				while(!(ADC1->SR & 2)) {}       															// wait for conv complete
				result = ADC1->DR;              															// read conversion result
				voltage = result *(3.3/4095.0);  															// convert ADC output to voltage
				distance = (-94.74*voltage)+308;															// convert to angle the potentiometer has travelled
				sprintf(Voltage_Buffer, "Voltage: %.2f V", voltage);		
				Send_A_String(Voltage_Buffer);																// Display Voltage
				LCD_command(0xC0);
				sprintf(Distance_Buffer, "Distance: %.1f\337", distance);
				Send_A_String(Distance_Buffer);																// Display angle
				delayMs(500);
				LCD_command(0x02);
		}
}

// Port Initializations for LCD. PA5-R/S, PA6-R/W, PA7-EN, PB0-PB7 for D0-D7, respectively.
void Port_init(void) {
		//Enable Clocks
		RCC->AHB1ENR = 3;	            	// enable GPIOA/B clocks
		
		// Enable Modes
		GPIOA->MODER = 0;    						// clear pin mode
		GPIOA->MODER = 0x5555555C;    	// set pins output mode, PA1 to analog
    GPIOA->BSRR  = 0x00C00000;      // turn off EN and R/W
    GPIOB->MODER = 0;    						// clear pin mode
    GPIOB->MODER = 0x55555555;    	// set pins output mode
	
		// setup ADC1 
    RCC->APB2ENR |= 0x00000100;     // enable ADC1 clock
    ADC1->CR2 = 0;                  // SW trigger
    ADC1->SQR3 = 1;                 // conversion sequence starts at ch 1
    ADC1->SQR1 = 0;                 // conversion sequence length 1
    ADC1->CR2 |= 1;                 // enable ADC1
		ADC1->SMPR2 = 0xFFFFFFFF;				// set sample rate at max # of cycles
}

// initialize port pins then initialize LCD controller
void LCD_init(void) {
    delayMs(30);            // initialization sequence
    LCD_command(0x30);
    delayMs(10);
    LCD_command(0x30);
    delayMs(1);
    LCD_command(0x30);

    LCD_command(0x38);      // set 8-bit data, 2-line, 5x7 font
    LCD_command(0x06);      // move cursor right after each char
    LCD_command(0x01);      // clear screen, move cursor to home
    LCD_command(0x0F);      // turn on display, cursor blinking
}

// Send command to LCD
void LCD_command(unsigned char command) {
    GPIOA->BSRR = (RS | RW) << 16;  // RS = 0, R/W = 0
    GPIOB->ODR = command;           // put command on data bus
    GPIOA->BSRR = EN;               // pulse E high
    delayMs(0);
    GPIOA->BSRR = EN << 16;         // clear E
    if (command < 4)
        delayMs(2);         				// command 1 and 2 needs up to 1.64ms
    else
        delayMs(1);         				
}

// Write data to the LCD
void LCD_data(char datawrite) {
    GPIOA->BSRR = RS;               // RS = 1
    GPIOA->BSRR = RW << 16;         // R/W = 0
    GPIOB->ODR = datawrite;         // put data on data bus
    GPIOA->BSRR = EN;               // pulse E high
    delayMs(0);
    GPIOA->BSRR = EN << 16;         // clear E
    delayMs(1);
}

// Sending a String of Characters
void Send_A_String(char *StringOfCharacters){
	while(*StringOfCharacters > 0){
		LCD_data(*StringOfCharacters++);
	}
}

// Delay timer
void delayMs(int n) {
    int i;

    // Configure SysTick
    SysTick->LOAD = 16000;  // reload with number of clocks per millisecond
    SysTick->VAL = 0;       // clear current value register
    SysTick->CTRL = 0x5;    // Enable the timer

    for(i = 0; i < n; i++) {
        while((SysTick->CTRL & 0x10000) == 0) // wait until the COUNTFLAG is set
            { }
    }
    SysTick->CTRL = 0;      // Stop the timer (Enable = 0)
}
