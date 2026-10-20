#include "pca9538_keypad.h"

#define REG_INPUT        0x00
#define REG_OUTPUT       0x01
#define REG_POLARITY     0x02
#define REG_CONFIG       0x03

void Keypad_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t config = 0xF0;
    HAL_I2C_Mem_Write(hi2c, PCA9538_KEYPAD_ADDR, REG_CONFIG, 1, &config, 1, 50);

    uint8_t out = 0xFF;
    HAL_I2C_Mem_Write(hi2c, PCA9538_KEYPAD_ADDR, REG_OUTPUT, 1, &out, 1, 50);
}

uint8_t Keypad_Scan(I2C_HandleTypeDef *hi2c) {
    for (uint8_t row = 0; row < 4; row++) {
        uint8_t row_mask = (uint8_t)~(1 << row);
        if (HAL_I2C_Mem_Write(hi2c, PCA9538_KEYPAD_ADDR, REG_OUTPUT, 1, &row_mask, 1, 10) != HAL_OK) {
            continue;
        }

        uint8_t in_val = 0xFF;
        if (HAL_I2C_Mem_Read(hi2c, PCA9538_KEYPAD_ADDR, REG_INPUT, 1, &in_val, 1, 10) != HAL_OK) {
            continue;
        }

        for (uint8_t col = 0; col < 3; col++) {
            if (!(in_val & (1 << (col + 4)))) {
                return (row * 3 + col + 1);
            }
        }
    }

    return 0;
}
