#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define _SET(type, name, bit)   type ## name  |= _BV(bit)
#define _GET(type, name, bit)   ((type ## name >> bit) &  1)
#define _CLEAR(type, name, bit) type ## name  &= ~ _BV(bit)

#define OUTPUT(pin) _SET(DDR, pin)
#define INPUT(pin)  _CLEAR(DDR, pin)
#define HIGH(pin)   _SET(PORT, pin)
#define LOW(pin)    _CLEAR(PORT, pin)
#define READ(pin)   _GET(PIN, pin)

// PORT_NAME, PIN_NUMBER_ON_THAT_PORT
#define MY_SCL B, 3
#define MY_SDA B, 4

#define I2C_DEBUG_DELAY 4

void i2c_start(void) {
    OUTPUT(MY_SCL);
    OUTPUT(MY_SDA);

    // i2c start signal: SDA goes LOW before SCL goes LOW
    HIGH(MY_SCL);
    HIGH(MY_SDA);
    LOW(MY_SDA);
    LOW(MY_SCL);
}

void i2c_stop(void) {
    OUTPUT(MY_SCL);
    OUTPUT(MY_SDA);

    LOW(MY_SCL);
    LOW(MY_SDA);
    HIGH(MY_SCL);
    HIGH(MY_SDA);
}

void i2c_write(uint8_t data) {
    {
        OUTPUT(MY_SDA);

        for (int i = 0; i < 8; ++i) {
            if ((data << i) & 0x80) {
                HIGH(MY_SDA);
            } else {
                LOW(MY_SDA);
            }

            HIGH(MY_SCL);
            LOW(MY_SCL);
        }
    }

    {
        // INPUT(MY_SDA);

        HIGH(MY_SDA);
        HIGH(MY_SCL);
        LOW(MY_SDA); // technically we should read here
        LOW(MY_SCL);
    }
}

// uint8_t i2c_read(void) {
//     INPUT(MY_SDA);

//     uint8_t data = 0;

//     for (int i = 0; i < 8; ++i) {
//         HIGH(MY_SCL);

//         if (!READ(SDA)) {
//             data |= 1;
//         }

//         LOW(MY_SCL);

//         data >>= 1;
//     }

//     {
//         HIGH(MY_SCL);
//         HIGH(MY_SDA);
//         LOW(MY_SCL);
//     }
// }

// SSD1306 128x64 monochrome display interface
// I2C address
#define SSD1306_ADDR   0x78

// SSD1306 supports two types of packages: commands and data
// data represents the portion of GDDR memory, storing a bitmap to be displayed
// commands control the behaviour of a display
// communication protocol is as following: START, SSD1306 ADDRESS (0x78) + R/W bit, CONTROL COMMAND (0x40, where the D/C# bit = 1, meaning writing data)
void ssd1306_start_command(void) {
    i2c_start();
    i2c_write(SSD1306_ADDR);
    i2c_write(0x00); // C0 = 0 (data), D/C = 0 (command)
}

void ssd1306_start_data(void) {
    i2c_start();
    i2c_write(SSD1306_ADDR);  // Slave address, R/W(SA0)=0 - write
    i2c_write(0x40);           // C0 = 0 (data), D/C = 1 (data)
}

void ssd1306_data(uint8_t byte) {
    i2c_write(byte);
}

void ssd1306_stop(void) {
    i2c_stop();
}

// initialization sequence
void ssd1306_init(void) {
    ssd1306_start_command();

    // ssd1306_data(0xAF); // display ON

    // // ssd1306_stop();

    // ssd1306_data(0x20); // set memory addressing mode
    // ssd1306_data(0x00); // memory addressing mode: 00 - horizontal, 01 - vertical, 10 - page

    // ssd1306_data(0x40 | 0x00); // set start line address = 0

    // ssd1306_data(0x21); // set column address
    // ssd1306_data(0x00); // column address start
    // ssd1306_data(0x7f); // column address end

    // ssd1306_data(0x22); // set page address
    // ssd1306_data(0x00); // page address start
    // ssd1306_data(0x3f); // page address end

    // // ssd1306_start_command();

    // ssd1306_data(0xA4); // entire display ON (resume) - output GDDR to display

// old:

    ssd1306_data(0xAF);           // Set Display ON/OFF - AE=OFF, AF=ON
    ssd1306_data(0xD5);
    // ssd1306_data(0xF0);     // Set display clock divide ratio/oscillator frequency, set divide ratio
    // ssd1306_data(0xA8);
    // ssd1306_data(0x3F);     // Set multiplex ratio (1 to 64) ... (height - 1)
    // ssd1306_data(0xD3);
    ssd1306_data(0x00);     // Set display offset. 00 = no offset
    ssd1306_data(0x40 | 0x00);    // Set start line address, at 0.
    ssd1306_data(0x8D);
    ssd1306_data(0x14);     // Charge Pump Setting, 14h = Enable Charge Pump
    ssd1306_data(0x20);
    ssd1306_data(0x00);     // Set Memory Addressing Mode - 00=Horizontal, 01=Vertical, 10=Page, 11=Invalid
    ssd1306_data(0xA0 | 0x01);    // Set Segment Re-map
    ssd1306_data(0xC8);           // Set COM Output Scan Direction
    ssd1306_data(0xDA);
    ssd1306_data(0x12);     // Set COM Pins Hardware Configuration - 128x32:0x02, 128x64:0x12
    ssd1306_data(0x81);
    ssd1306_data(0x3F);     // Set contrast control register
    ssd1306_data(0xD9);
    ssd1306_data(0x22);     // Set pre-charge period (0x22 or 0xF1)
    ssd1306_data(0xDB);
    // ssd1306_data(0x20);     // Set Vcomh Deselect Level - 0x00: 0.65 x VCC, 0x20: 0.77 x VCC (RESET), 0x30: 0.83 x VCC
    ssd1306_data(0xA4);           // Entire Display ON (resume) - output RAM to display
    // ssd1306_data(0xA6);           // Set Normal/Inverse Display mode. A6=Normal; A7=Inverse
    ssd1306_data(0x2E);           // Deactivate Scroll command
    ssd1306_data(0xAF);           // Set Display ON/OFF - AE=OFF, AF=ON
    //
    ssd1306_data(0x22);
    ssd1306_data(0x00);
    ssd1306_data(0x3f);   // Set Page Address (start,end)
    ssd1306_data(0x21);
    ssd1306_data(0x00);
    ssd1306_data(0x7f);

    ssd1306_stop();
}

void ssd1306_gotoxy(uint8_t x, uint8_t y) {
    ssd1306_start_command();

    ssd1306_data(0x21); // select column (horizontal coordinate) start and end address
    ssd1306_data(x & 0x7f); // start: only lowest 7 bits are considered
    ssd1306_data(0x7f); // end: 0x7f

    ssd1306_data(0x22); // select page (vertical coordinate) start and end address
    ssd1306_data(y & 0x07); // only lowest 3 bits are considered
    ssd1306_data(0x07); // end: 0x07

    ssd1306_stop();
}

void ssd1306_clear(void) {
    ssd1306_gotoxy(0, 0);

    ssd1306_start_data();   // Initiate transmission of data

    for (uint16_t i = 0; i < 128 * 8; i++) {
        ssd1306_data(0);
        ssd1306_data(0);
        ssd1306_data(0);
        ssd1306_data(0);
    }

    ssd1306_stop();
}

void ssd1306_hello_world(void) {
    ssd1306_gotoxy(0, 0);

    ssd1306_start_data();

    // H
    ssd1306_data(0x00);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0x18);
    ssd1306_data(0x18);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0x00);

    // E
    ssd1306_data(0x00);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0xdb);
    ssd1306_data(0xdb);
    ssd1306_data(0xc3);
    ssd1306_data(0xc3);
    ssd1306_data(0x00);

    // L
    ssd1306_data(0x00);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0x00);

    // L
    ssd1306_data(0x00);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0x00);

    // O
    ssd1306_data(0x00);
    ssd1306_data(0x3c);
    ssd1306_data(0x7e);
    ssd1306_data(0xc3);
    ssd1306_data(0xc3);
    ssd1306_data(0x7e);
    ssd1306_data(0x3c);
    ssd1306_data(0x00);

    // SPC
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);
    ssd1306_data(0x00);

    // W
    ssd1306_data(0xff);
    ssd1306_data(0x7f);
    ssd1306_data(0x20);
    ssd1306_data(0x30);
    ssd1306_data(0x30);
    ssd1306_data(0x20);
    ssd1306_data(0x7f);
    ssd1306_data(0xff);

    // O
    ssd1306_data(0x00);
    ssd1306_data(0x3c);
    ssd1306_data(0x7e);
    ssd1306_data(0xc3);
    ssd1306_data(0xc3);
    ssd1306_data(0x7e);
    ssd1306_data(0x3c);
    ssd1306_data(0x00);

    // R
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0x13);
    ssd1306_data(0x13);
    ssd1306_data(0x33);
    ssd1306_data(0x7b);
    ssd1306_data(0xce);
    ssd1306_data(0x84);

    // L
    ssd1306_data(0x00);
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0xc0);
    ssd1306_data(0x00);

    // D
    ssd1306_data(0xff);
    ssd1306_data(0xff);
    ssd1306_data(0xc3);
    ssd1306_data(0xc3);
    ssd1306_data(0xc3);
    ssd1306_data(0xe7);
    ssd1306_data(0x7e);
    ssd1306_data(0x18);

    ssd1306_stop();
}

int main(void) {
    _delay_ms(40);
    ssd1306_init();

    while (1) {
        ssd1306_clear();
        ssd1306_hello_world();
        _delay_ms(2000);
    }

    return 0;
}
