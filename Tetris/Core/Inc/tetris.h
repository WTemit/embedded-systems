#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define BOARD_WIDTH       10
#define BOARD_HEIGHT      20
#define BLOCK_SIZE        3
#define BOARD_OFFSET_X    5
#define BOARD_OFFSET_Y    2

typedef enum {
    CMD_NONE = 0,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_ROTATE,
    CMD_DOWN,
    CMD_HARD_DROP,
    CMD_RESTART,
    CMD_PAUSE
} GameCommand_t;

typedef enum {
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
} GameState_t;

// Внешние синхронизирующие объекты FreeRTOS
extern QueueHandle_t xQueueInput;
extern SemaphoreHandle_t xI2CMutex;
extern SemaphoreHandle_t xFrameReadySem;

void Tetris_Init(void);
void Tetris_Update(GameCommand_t cmd);
void Tetris_Draw(void);

#endif
