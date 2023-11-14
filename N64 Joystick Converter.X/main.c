/*
 * File:   main.c
 * Author: Gunnar
 *
 * Created on August 9, 2020, 11:43 AM
 */

#include "PICCONFIG.h"
#include "ADC.h"
#include <xc.h>
#include <stdint.h>

char rotate_right[4] = {0b01,0b11,0b00,0b10};
char rotate_left[4] = {0b10,0b00,0b11,0b01};

static int16_t Map(int x, int inMin, int inMax, int outMin, int outMax) {
    // Map a single value onto a different range
    return (((x - inMin) * (outMax - outMin)) / (inMax - inMin)) +outMin;
}

void PIC_SETUP(){
    
    CPUDOZEbits.IDLEN = 0;      //clear idle mode
    VREGCON = 0b00000011;       //low power sleep mode
    
    //IO setup
    TRISA = 0xFF;
    TRISC = 0xFF;
    ANSELA = 0;
    ANSELC = 0;
    ANSELAbits.ANSA4 = 1;  
    ANSELCbits.ANSC4 = 1;
    
    ODCONAbits.ODCA5 = 0;
    ODCONCbits.ODCC5 = 0;
    TRISAbits.TRISA5 = 0;
    TRISCbits.TRISC5 = 0;
    
    ODCONCbits.ODCC2 = 0;
    ODCONCbits.ODCC3 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
}

#define SX_MIN 36
#define SX_MAX 213
#define SY_MIN 31
#define SY_MAX 208
#define dead_zone 19

int16_t get_y(int16_t y) {
    y = readADC(ADCRC4);   
    
    if(y < SY_MIN) y = SY_MIN;
    else if(y > SY_MAX) y = SY_MAX;   
    y = Map(y, SY_MIN, SY_MAX, 37, 218);    
    if( (y < (127 + dead_zone)) && (y > (127 - dead_zone)) ) y = 127;    
    return y;
}

int16_t get_x(int16_t x) {
    x = readADC(ADCRA4);
    
    if(x < SX_MIN) x = SX_MIN;
    else if(x > SX_MAX) x = SX_MAX;   
    x = Map(x, SX_MIN, SX_MAX, 37, 218);   
    if( (x < (127 + dead_zone)) && (x > (127 - dead_zone)) ) x = 127;    
    return x;
}

void main() {
	
	PIC_SETUP();
	
	int16_t x = 0;
	int16_t y = 0;
    
    y = get_y(y);
    x = get_x(x);
    
    int16_t x_prev = x;
    int16_t y_prev = y;
	
	int16_t x_steps = 0;
	int16_t y_steps = 0;
	
	char x_out = 0;
	char y_out = 0;
    
    LATAbits.LATA5 = y_out;
    LATCbits.LATC5 = y_out>>1;   
    LATCbits.LATC2 = x_out;
    LATCbits.LATC3 = x_out>>1;    
	
	while(1) {
        
        CLRWDT(); 
        
		//measure x and y adc
        y = (int16_t) get_y(y);
        x = (int16_t) get_x(x);
		
		x_steps = (int16_t) x - x_prev;
		y_steps = (int16_t) y - y_prev;

		x_prev = x;
		y_prev = y;

		while((x_steps != 0) || (y_steps != 0)) {
                
			if(x_steps > 0) {
				x_out = rotate_right[x_out];
				x_steps--;
			}
			if(x_steps < 0) {
				x_out = rotate_left[x_out];
				x_steps++;
			}
			
			if(y_steps > 0) {
				y_out = rotate_right[y_out];
				y_steps--;
			}
			if(y_steps < 0) {
				y_out = rotate_left[y_out];
				y_steps++;
			}		
			
			//set output
            LATAbits.LATA5 = y_out;
            LATCbits.LATC5 = y_out>>1;
            
            LATCbits.LATC2 = x_out;
            LATCbits.LATC3 = x_out>>1;
			
		}
	}
}