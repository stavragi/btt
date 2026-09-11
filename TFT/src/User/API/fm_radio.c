#include "stm32f10x.h"
#include "delay.h"

// Define GPIO Pins (Repurposed RS232 Pins)
#define RADIO_GPIO   GPIOA
#define PIN_SDA      GPIO_Pin_9  // TX pin
#define PIN_SCL      GPIO_Pin_10 // RX pin
#define TEA5767_ADDRESS 0x60 // Default I2C Address

void Radio_SetFrequency(float frequency) {
    uint8_t data[5];
    // Calculate frequency data word for TEA5767
    uint32_t frequency_word = (uint32_t)((frequency * 1000000 + 225000) / 8192);
    
    data[0] = (frequency_word >> 8) & 0x3F; // First byte (MUTE/Search bits cleared)
    data[1] = frequency_word & 0xFF;        // Second byte
    data[2] = 0xB0;                         // Third byte (Stereo, HLSI)
    data[3] = 0x10;                         // Fourth byte (XTAL adjustment)
    data[4] = 0x00;                         // Fifth byte
    
    I2C_Start();
    I2C_WriteByte(TEA5767_ADDRESS << 1); // Write mode
    for (int i = 0; i < 5; i++) {
        I2C_WriteByte(data[i]);
    }
    I2C_Stop();
}



void Radio_I2C_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Configure pins as Open-Drain Outputs
    GPIO_InitStructure.GPIO_Pin = PIN_SDA | PIN_SCL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RADIO_GPIO, &GPIO_InitStructure);
    
    GPIO_SetBits(RADIO_GPIO, PIN_SDA | PIN_SCL);
}

void I2C_Delay(void) {
    Delay_us(5);
}

void I2C_Start(void) {
    GPIO_SetBits(RADIO_GPIO, PIN_SDA);
    GPIO_SetBits(RADIO_GPIO, PIN_SCL);
    I2C_Delay();
    GPIO_ResetBits(RADIO_GPIO, PIN_SDA);
    I2C_Delay();
    GPIO_ResetBits(RADIO_GPIO, PIN_SCL);
}

void I2C_Stop(void) {
    GPIO_ResetBits(RADIO_GPIO, PIN_SDA);
    GPIO_SetBits(RADIO_GPIO, PIN_SCL);
    I2C_Delay();
    GPIO_SetBits(RADIO_GPIO, PIN_SDA);
    I2C_Delay();
}

void I2C_WriteByte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80)
            GPIO_SetBits(RADIO_GPIO, PIN_SDA);
        else
            GPIO_ResetBits(RADIO_GPIO, PIN_SDA);
        byte <<= 1;
        I2C_Delay();
        GPIO_SetBits(RADIO_GPIO, PIN_SCL);
        I2C_Delay();
        GPIO_ResetBits(RADIO_GPIO, PIN_SCL);
    }
    // Release SDA for ACK check
    GPIO_SetBits(RADIO_GPIO, PIN_SDA);
    I2C_Delay();
    GPIO_SetBits(RADIO_GPIO, PIN_SCL);
    I2C_Delay();
    GPIO_ResetBits(RADIO_GPIO, PIN_SCL);
}
