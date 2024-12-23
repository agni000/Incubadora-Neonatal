#include <xc.h> // include processor files - each processor file is guarded.  

#define DHT11_Data_Pin   PORTCbits.RC2
#define DHT11_Data_Pin_Direction  TRISCbits.TRISC2

unsigned char checkBit, tempByte1, tempByte2, rhByte1, rhByte2;
unsigned char temper, RH, sumation;

#define _XTAL_FREQ 8000000 //***Define a frequencia do clock, 8Mhz neste caso

void dht11_init(){
    DHT11_Data_Pin_Direction = 0; 
    DHT11_Data_Pin = 0;
    __delay_ms(18);
    DHT11_Data_Pin = 1; 
    __delay_us(25);
    DHT11_Data_Pin_Direction = 1;
 }

void find_response(){
    checkBit = 0;
    __delay_us(20);  // Wait for at least 20 탎 before detecting the response
    if (DHT11_Data_Pin == 0) {  // Check if the line is pulled low by the sensor
        __delay_us(80);         // Wait for the sensor to pull the line low for 80 탎
        if (DHT11_Data_Pin == 1) {  // Check if the line is pulled high after the 80 탎 low pulse
            checkBit = 1;  // DHT11 is ready, response detected
        }
        __delay_us(50);  // Wait for 50 탎 before continuing
    }
}
char read_dht11() {
    char data = 0;
    for (char i = 0; i < 8; i++) {
        unsigned int timeout = 1000;  // Timeout counter
        while (!DHT11_Data_Pin && --timeout);  // Wait for the line to go high
        if (timeout == 0) return 0xFF;  // Return error if timeout
        
        __delay_us(30);  // Wait 30 탎
        if (DHT11_Data_Pin == 0)
        {
            data &= ~(1 << (7 - i));  // Bit is 0
        }
        else 
        {
            data |= (1 << (7 - i));   // Bit is 1
            timeout = 1000;          // Reset timeout
            while (DHT11_Data_Pin && --timeout);  // Wait for the high pulse to end
            if (timeout == 0) return 0xFF;  // Return error if timeout
        }
    }
    return data;
}
