#include <reg51.h>
#include <stdio.h>

// === LCD PINS ===
#define LCD P1
sbit rs = P3^6;
sbit en = P3^7;

// === I2C PINS ===
sbit SDA = P3^1;
sbit SCL = P3^0;

// === SPI PINS ===
sbit clk = P2^3;
sbit din = P2^2;
sbit dout = P2^1;
sbit cs = P2^0;

sbit sw = P2^5;
sbit buzzer = P3^4;
#define Temp_Threshold 40.0

// === Global Variables ===
unsigned char eprom_addr = 0x00;
volatile bit update_required = 0;
volatile bit lcd_disp = 0 ; 

// === Delay Function ===
void delay(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 1275; j++);
}

// === LCD Functions ===
void lcd_cmd(unsigned char cmd) {
    rs = 0;
    LCD = cmd;
    en = 1;
    delay(2);
    en = 0;
}

void lcd_data(unsigned char daata) {
    rs = 1;
    LCD = daata;
    en = 1;
    delay(2);
    en = 0;
}

void lcd_str(unsigned char *str) {
    while(*str)
        lcd_data(*str++);
}

void lcd_init(void) {
    lcd_cmd(0x38); // 2-line, 5x7 matrix
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    lcd_cmd(0x01); // Clear Display
    lcd_cmd(0x80); // Cursor to line 1
}

// === I2C Functions ===
void i2c_start(void) {
    SDA = 1; SCL = 1; delay(2);
    SDA = 0; delay(2);
    SCL = 0;
}

void i2c_stop(void) {
    SDA = 0; SCL = 1; delay(2);
    SDA = 1; delay(2);
}

void i2c_write(unsigned char daata) {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        SDA = (daata & 0x80) ? 1 : 0;
        SCL = 1; delay(1);
        SCL = 0;
        daata <<= 1;
    }
    SDA = 1; // ACK
    SCL = 1; delay(1);
    SCL = 0;
}

unsigned char i2c_read(void) {
    unsigned char i, daata = 0;
    SDA = 1; // Release line for input
    for(i = 0; i < 8; i++) {
        SCL = 1; delay(1);
        daata = (daata << 1) | SDA;
        SCL = 0;
        delay(1);
    }
    return daata;
}

// === SPI ADC Function ===
unsigned int read_adc(void) {
    unsigned int temp = 0;
    int i;

    cs = 0; clk = 0;

    // Start & config bits for MCP3208 or similar
    din = 1; clk = 1; delay(1); clk = 0;
    din = 1; clk = 1; delay(1); clk = 0;
    din = 1; clk = 1; delay(1); clk = 0; // D2
    din = 0; clk = 1; delay(1); clk = 0; // D1
    din = 0; clk = 1; delay(1); clk = 0; // D0
    din = 1; clk = 1; delay(1); clk = 0; // TS
    din = 1; clk = 1; delay(1); clk = 0; // NULL

    for(i = 11; i >= 0; i--) {
        clk = 1; delay(1);
        if(dout) temp |= (1 << i);
        clk = 0; delay(1);
    }

    cs = 1;
    return temp;
}

// === Display Temperature on LCD ===
float display_temp(unsigned int adc_val) {
    float temp_c = ((float)adc_val * 500.0) / 4095.0 - 0.3;
    char str[16];
    sprintf(str, "Temp: %.1f C", temp_c);
    lcd_cmd(0xC0);
    lcd_str(str);
    return temp_c;
}




void ext_int0_isr (void) interrupt 0 
{
	update_required = 1 ;
}	

// === MAIN FUNCTION ===
void main(void) 
{
    unsigned int adc_val;
    float temp_c;
    unsigned char scaled;

    lcd_init();
    lcd_str("DATA LOGGER");
    delay(100);
    lcd_cmd(0x01);

    // Enable INT0 (P3.2)
    IT0 = 1;  // Edge triggered
    EX0 = 1;  // Enable external interrupt 0
    EA = 1;   // Global interrupt enable

    while (1) 
   {
		 

    if (temp_c >= Temp_Threshold) 
			{
				char str[10];
				buzzer = 1;  // Active buzzer turns ON

				lcd_cmd(0x01);             // Clear display once
				delay(2);                  // Short delay after clear
				lcd_cmd(0x80);             // Line 1 start
				lcd_str("EMERGENCY ALERT");

				lcd_cmd(0xC0);             // Line 2
				sprintf(str, "Temp: %.1f C", temp_c);
				lcd_str(str);
				lcd_disp = 1 ; 

			  delay(10);               // Show alert and sound for 1 sec
				buzzer = 0;                // Turn OFF buzzer after beep
				

			} 
			else          // clearing display after Alert
			{
					if (lcd_disp == 1) 
				{
					lcd_cmd(0x01);  // Clear only once when temp drops
					delay(2);
					lcd_cmd(0x80);
					lcd_str("Temp Normal");
					delay(1000);
					lcd_cmd(0x01);
					lcd_disp = 0;
				}
			}

    buzzer = 0;

	

		 
     if (update_required == 1) 
		 {
        unsigned char i, addr, temp_val;
        float temp_c;
        char str[16];

        lcd_cmd(0x01);               // Clear LCD
        delay(10);
			 //lcd_cmd(0x80);               // Set cursor to beginning
        lcd_str("Last 5 Temp Data"); // Title

        for (i = 0; i < 5; i++)
		    	{
            addr = (eprom_addr - 5 + i + 256) % 256;

            i2c_start();
            i2c_write(0xA0);        // EEPROM write address
            i2c_write(addr);        // Memory location
            i2c_start();
            i2c_write(0xA1);        // EEPROM read address
            temp_val = i2c_read();  // Read value
            i2c_stop();

            temp_c = (float)temp_val / 10.0;

            // Format: T1:24.5C (max 8 chars)
            sprintf(str, "T%d : %.1fC", i + 1, temp_c);

            if (i == 0) lcd_cmd(0x80);      // Line 1
						else if (i == 1) lcd_cmd(0xC0); // Line 2
						else if (i == 2) lcd_cmd(0x94); // Line 3
						else if (i == 3) lcd_cmd(0xD4); // Line 4
						else lcd_cmd(0x80 + 10);        // Continue on line 1 (or scroll)


						lcd_str(str);
        }

        update_required = 0;        // ? Clear flag after use
        delay(1000);                // Give time to read output
        lcd_cmd(0x01);              // Clear screen again
		  	}

    // (Main loop continues...)
		  


				
     adc_val = read_adc();
     temp_c = display_temp(adc_val);

     // Scale temperature and store in EEPROM
      scaled = (unsigned char)(temp_c * 10); // e.g., 25.6C ? 256


     i2c_start();
     i2c_write(0xA0);            // EEPROM write address
     i2c_write(eprom_addr);      // Write address
     i2c_write(scaled);          // Data
     i2c_stop();

     eprom_addr++;
     if(eprom_addr > 0xFF) eprom_addr = 0;

     delay(1000);
	 } 
}

