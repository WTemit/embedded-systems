#include "tetris.h"
#include "oled.h"
#include "fonts.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const uint16_t TETROMINOES[7][4] = {
    // I
    { 0x0F00, 0x2222, 0x00F0, 0x4444 },
    // J
    { 0x8E00, 0x6440, 0x0E20, 0x44C0 },
    // L
    { 0x2E00, 0x4460, 0x0E80, 0xC440 },
    // O
    { 0x6600, 0x6600, 0x6600, 0x6600 },
    // S
    { 0x6C00, 0x4620, 0x06C0, 0x8C40 },
    // T
    { 0x4E00, 0x4640, 0x0E40, 0x4C40 },
    // Z
    { 0xC600, 0x2640, 0x0C60, 0x4C80 }
};

static uint8_t Board[BOARD_HEIGHT][BOARD_WIDTH];
static int8_t curX, curY;
static uint8_t curPiece, curRot;
static uint8_t nextPiece;
static uint32_t score = 0;
static uint32_t linesCleared = 0;
static GameState_t gameState = STATE_PLAYING;

// Получение бита фигуры 4x4
static inline bool GetPieceBit(uint8_t piece, uint8_t rot, int8_t x, int8_t y) {
    uint16_t mask = 0x8000 >> (y * 4 + x);
    return (TETROMINOES[piece][rot & 3] & mask) != 0;
}

static bool CheckCollision(int8_t testX, int8_t testY, uint8_t testRot) {
    for (int8_t py = 0; py < 4; py++) {
        for (int8_t px = 0; px < 4; px++) {
            if (GetPieceBit(curPiece, testRot, px, py)) {
                int8_t bx = testX + px;
                int8_t by = testY + py;

                if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return true;
                if (by >= 0 && Board[by][bx]) return true;
            }
        }
    }
    return false;
}

static void SpawnPiece(void) {
    curPiece = nextPiece;
    nextPiece = rand() % 7;
    curRot = 0;
    curX = (BOARD_WIDTH - 4) / 2;
    curY = 0;

    if (CheckCollision(curX, curY, curRot)) {
        gameState = STATE_GAME_OVER;
    }
}

static void MergePiece(void) {
    for (int8_t py = 0; py < 4; py++) {
        for (int8_t px = 0; px < 4; px++) {
            if (GetPieceBit(curPiece, curRot, px, py)) {
                int8_t bx = curX + px;
                int8_t by = curY + py;
                if (bx >= 0 && bx < BOARD_WIDTH && by >= 0 && by < BOARD_HEIGHT) {
                    Board[by][bx] = 1;
                }
            }
        }
    }
}

static void ClearLines(void) {
    uint8_t linesFound = 0;
    for (int8_t y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (int8_t x = 0; x < BOARD_WIDTH; x++) {
            if (!Board[y][x]) {
                full = false;
                break;
            }
        }
        if (full) {
            linesFound++;
            for (int8_t moveY = y; moveY > 0; moveY--) {
                for (int8_t x = 0; x < BOARD_WIDTH; x++) {
                    Board[moveY][x] = Board[moveY - 1][x];
                }
            }
            for (int8_t x = 0; x < BOARD_WIDTH; x++) Board[0][x] = 0;
            y++;
        }
    }

    if (linesFound > 0) {
        linesCleared += linesFound;
        static const uint16_t pts[] = {0, 100, 300, 700, 1500};
        score += pts[linesFound <= 4 ? linesFound : 4];
    }
}

void Tetris_Init(void) {
    memset(Board, 0, sizeof(Board));
    score = 0;
    linesCleared = 0;
    gameState = STATE_PLAYING;
    nextPiece = rand() % 7;
    SpawnPiece();
}

// Шаг логики при получении команды
void Tetris_Update(GameCommand_t cmd) {
    if (cmd == CMD_RESTART) {
        Tetris_Init();
        return;
    }
    if (cmd == CMD_PAUSE) {
        if (gameState == STATE_PLAYING) gameState = STATE_PAUSED;
        else if (gameState == STATE_PAUSED) gameState = STATE_PLAYING;
        return;
    }

    if (gameState != STATE_PLAYING) return;

    switch (cmd) {
        case CMD_LEFT:
            if (!CheckCollision(curX - 1, curY, curRot)) curX--;
            break;
        case CMD_RIGHT:
            if (!CheckCollision(curX + 1, curY, curRot)) curX++;
            break;
        case CMD_ROTATE:
            if (!CheckCollision(curX, curY, (curRot + 1) & 3)) curRot = (curRot + 1) & 3;
            break;
        case CMD_DOWN:
            if (!CheckCollision(curX, curY + 1, curRot)) {
                curY++;
            } else {
                MergePiece();
                ClearLines();
                SpawnPiece();
            }
            break;
        case CMD_HARD_DROP:
            while (!CheckCollision(curX, curY + 1, curRot)) {
                curY++;
                score += 2;
            }
            MergePiece();
            ClearLines();
            SpawnPiece();
            break;
        default:
            break;
    }
}

static void DrawBlock(int16_t x, int16_t y, int16_t size, OLED_COLOR color) {
    for (int16_t i = x; i < x + size; i++) {
        for (int16_t j = y; j < y + size; j++) {
            oled_DrawPixel(i, j, color);
        }
    }
}

static void DrawFrame(int16_t x, int16_t y, int16_t w, int16_t h, OLED_COLOR color) {
    for (int16_t i = x; i < x + w; i++) {
        oled_DrawPixel(i, y, color);
        oled_DrawPixel(i, y + h - 1, color);
    }
    for (int16_t j = y; j < y + h; j++) {
        oled_DrawPixel(x, j, color);
        oled_DrawPixel(x + w - 1, j, color);
    }
}

// Формирование кадра Тетриса
void Tetris_Draw(void) {
    oled_Fill(Black);

    // Рамка стакана
    DrawFrame(BOARD_OFFSET_X - 1, BOARD_OFFSET_Y - 1,
              BOARD_WIDTH * BLOCK_SIZE + 2, BOARD_HEIGHT * BLOCK_SIZE + 2, White);

    // Упавшие блоки
    for (int8_t y = 0; y < BOARD_HEIGHT; y++) {
        for (int8_t x = 0; x < BOARD_WIDTH; x++) {
            if (Board[y][x]) {
                DrawBlock(BOARD_OFFSET_X + x * BLOCK_SIZE,
                          BOARD_OFFSET_Y + y * BLOCK_SIZE,
                          BLOCK_SIZE - 1, White);
            }
        }
    }

    // Текущая падающая фигура
    if (gameState != STATE_GAME_OVER) {
        for (int8_t py = 0; py < 4; py++) {
            for (int8_t px = 0; px < 4; px++) {
                if (GetPieceBit(curPiece, curRot, px, py)) {
                    int8_t bx = curX + px;
                    int8_t by = curY + py;
                    if (bx >= 0 && bx < BOARD_WIDTH && by >= 0 && by < BOARD_HEIGHT) {
                        DrawBlock(BOARD_OFFSET_X + bx * BLOCK_SIZE,
                                  BOARD_OFFSET_Y + by * BLOCK_SIZE,
                                  BLOCK_SIZE - 1, White);
                    }
                }
            }
        }
    }

    // Текстовая панель справа
    char buf[16];
    oled_SetCursor(40, 2);
    oled_WriteString("TETRIS", Font_7x10, White);

    sprintf(buf, "S:%lu", (unsigned long)score);
    oled_SetCursor(40, 16);
    oled_WriteString(buf, Font_7x10, White);

    sprintf(buf, "L:%lu", (unsigned long)linesCleared);
    oled_SetCursor(40, 30);
    oled_WriteString(buf, Font_7x10, White);

    // Превью следующей фигуры
    DrawFrame(40, 44, 4 * BLOCK_SIZE + 2, 4 * BLOCK_SIZE + 2, White);
    for (int8_t py = 0; py < 4; py++) {
        for (int8_t px = 0; px < 4; px++) {
            if (GetPieceBit(nextPiece, 0, px, py)) {
                DrawBlock(41 + px * BLOCK_SIZE, 45 + py * BLOCK_SIZE,
                          BLOCK_SIZE - 1, White);
            }
        }
    }

    if (gameState == STATE_PAUSED) {
        oled_SetCursor(6, 25);
        oled_WriteString("PAUSE", Font_11x18, White);
    } else if (gameState == STATE_GAME_OVER) {
        oled_SetCursor(4, 25);
        oled_WriteString("OVER!", Font_11x18, White);
    }
}
