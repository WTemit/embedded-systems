// pca9538_keypad.h
#ifndef PCA9538_KEYPAD_H
#define PCA9538_KEYPAD_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define PCA9538_KEYPAD_ADDR  0xE2

void Keypad_Init(I2C_HandleTypeDef *hi2c);
uint8_t Keypad_Scan(I2C_HandleTypeDef *hi2c);

#endif
