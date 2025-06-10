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

// === Global Variables ===
unsigned char eprom_addr = 0x00;

// === Delay Function ===
void delay(unsigned int k) {
    unsigned int i, j;
    for(i = 0; i < k; i++)
        for(j = 0; j < 1275; j++);
}

// === LCD Functions ===
void lcd_cmd(unsigned char cmd) {
    rs = 0;
    LCD = cmd;
    en = 1;
    delay(2);
    en = 0;
    delay(2);
}

void lcd_data(unsigned char daata)
	{
    rs = 1;
    LCD = daata;
    en = 1;
    delay(2);
    en = 0;
    delay(2);
  }

void lcd_str(unsigned char *str) {
    while(*str)
        lcd_data(*str++);
}

void lcd_init(void) {
    lcd_cmd(0x38);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    lcd_cmd(0x80);
}

// === I2C Functions ===
void i2c_start(void) {
    SDA = 1; SCL = 1; delay(2);
    SDA = 0; delay(2);
    SCL = 0; delay(2);
}

void i2c_stop(void) {
    SDA = 0; SCL = 0; delay(2);
    SCL = 1; delay(2);
    SDA = 1; delay(2);
}

void i2c_data_write(unsigned char daata) {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        SDA = (daata & 0x80) ? 1 : 0;
        SCL = 1; delay(2);
        SCL = 0;
        daata <<= 1;
    }
    SDA = 1;
    SCL = 1;
    while(SDA);
    SCL = 0;
}

unsigned char i2c_read_data(void) {
    unsigned char i, daata = 0;
    bit read_bit;

    for(i = 0; i < 8; i++) {
        delay(2);
        SCL = 1;
        read_bit = SDA;
        daata = (daata << 1) | read_bit;
        SCL = 0;
    }
    SDA = 1;
    SCL = 1; 
		delay(2);
    SCL = 0;
    return daata;
}

// === SPI ADC Function ===
unsigned int spi_adc_temp(void) {
    unsigned int temp = 0;
    int i;

    cs = 0; clk = 0;

    // Start & config bits
    din = 1; clk = 1; delay(1); clk = 0;
    din = 1; clk = 1; delay(1); clk = 0;
    din = 1; clk = 1; delay(1); clk = 0; // D2
    din = 0; clk = 1; delay(1); clk = 0; // D1
    din = 0; clk = 1; delay(1); clk = 0; // D0
    din = 1; clk = 1; delay(1); clk = 0; // TS
    din = 1; clk = 1; delay(1); clk = 0; // NULL bit

    for(i = 11; i >= 0; i--) {
        clk = 1;
        delay(1);
        if(dout) temp |= (1 << i);
        clk = 0;
        delay(1);
    }

    cs = 1;
    return temp;
}

// === Display Temperature on LCD ===
float display_temp(int adc_val) {
    char str[16];
    float tempc;

    tempc = ((float)adc_val * 500.0) / 4095.0 - 0.3;
    sprintf(str, "Temp: %.1f C", tempc);
    lcd_cmd(0xC0);
    lcd_str(str);
    return tempc;
}

// === INT0 ISR - Display Last 5 Temp Data from EEPROM ===
void switch_ext_int0(void) interrupt 0 {
    unsigned char i, addr;
    unsigned char temp_val;
    float tempc;
    char str[16];

    lcd_cmd(0x01);
    lcd_cmd(0x80);
    lcd_str("Last 51 Temp Data");

    for(i = 0; i < 5; i++) {
        // Calculate circular address
        //if(eprom_addr >= 5)
          //  addr = eprom_addr - 5 + i %256;
        //else
          //  addr = (256 + eprom_addr - 5 + i) % 256;

				addr = (eprom_addr - 5 + i ) %256 ;
        i2c_start();
        i2c_data_write(0xA0);
        i2c_data_write(addr);
        i2c_start();
        i2c_data_write(0xA1);
        temp_val = i2c_read_data();
        i2c_stop();
				
				tempc = (float)temp_val / 10.0 ;
					
        //tempc = ((float)temp_val * 500.0) / 4095.0;
        //tempc -= 0.3;

        sprintf(str, "T%d: %.1fC", i + 1, tempc);

      if(i < 2) 
			 {
         lcd_cmd(0xC0 + (i * 8));  // First line of second row
       } 
			else 
			  {
          lcd_cmd(0x80 + ((i-2) * 8));  // Wrap to first line if more than 2
        }
      lcd_str(str);
				

    }
	lcd_cmd(0x01);
}

// === MAIN FUNCTION ===
void main(void) {
    unsigned int adc_val;
    float temp_c;

    lcd_init();
    lcd_str("DATA LOGGER ");
    delay(10);
    lcd_cmd(0x01);
    //lcd_str("DATA LOGGER");
	
	

    IT0 = 1;  // Edge triggered INT0
    EX0 = 1;  // Enable INT0
    EA = 1;   // Enable global interrupts

    while(1) {
			unsigned char scaled;
        adc_val = spi_adc_temp();
        temp_c = display_temp(adc_val);
			   
			  

        // Store 8-bit scaled value into EEPROM
        scaled = (unsigned char)((temp_c + 0.3) * 4095.0 / 500.0);

        i2c_start();
        i2c_data_write(0xA0);
        i2c_data_write(eprom_addr);
        i2c_data_write(scaled);
        i2c_stop();

        eprom_addr++;
        if(eprom_addr > 0xFF)
            eprom_addr = 0;

        delay(100);
    }
}
