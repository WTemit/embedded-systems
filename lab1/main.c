#include "main.h"
#include "tm1637.h"
#include "keyboard.h"

volatile uint32_t tickCount;
uint32_t last_display_update;
uint16_t counter;
char lastKey;
uint32_t lastScanTime;

void osSystickHandler(void) {
  tickCount++;
}

void initGPIO() {
  // Включаем тактирование GPIOA и GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

  // Настраиваем PA5 как выход
  GPIOA->MODER = (GPIOA->MODER & ~(3 << 10)) | (1 << 10);
  GPIOA->OTYPER &= ~(1 << 5);
  GPIOA->OSPEEDR |= (1 << 10);
}

void initUSART2() {
  // Включаем тактирование USART2
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // Настраиваем PA2 и PA3 в альтернативный режим
  GPIOA->MODER = (GPIOA->MODER & ~(0xF << 4)) | (0xA << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF << 8)) | (1 << 8) | (1 << 12);

  // Настраиваем USART2
  USART2->BRR = 417; // 48MHz/115200
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void initSysTick() {
  SysTick->LOAD = 47999; // 1ms при 48MHz
  SysTick->VAL = 0;
  SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}

int _write(int file, uint8_t *ptr, int len) {
  for (int i = 0; i < len; i++) {
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ptr[i];
  }
  return len;
}

// Отображение знака операции на TM1637
void show_op(char op) {
  tm1637_clear();
  uint8_t code = 0;
  switch (op) {
    case '+': code = 0x73; break;
    case '-': code = 0x40; break;
    case '*': code = 0x63; break;
    case '/': code = 0x5E; break;
  }
  tm1637_display_digit(3, code);
}

// Отображение результата (с поддержкой отрицательных чисел)
void show_result(int res) {
  if (res < 0) {
    int abs_res = -res;
    tm1637_clear();
    tm1637_display_digit(2, 0x40); // знак '-'
    tm1637_display_digit(3, digit_codes[abs_res % 10]);
  } else {
    tm1637_display_number(res);
  }
}

void show_error(void) {
  tm1637_clear();
  tm1637_display_digit(1, 0x79); // E
  tm1637_display_digit(2, 0x50); // r
  tm1637_display_digit(3, 0x50); // r
}

typedef enum {
  CALC_STATE_NUM1 = 0,
  CALC_STATE_OP,
  CALC_STATE_NUM2,
  CALC_STATE_DONE
} CalcState;

int main(void) {
  initGPIO();
  initUSART2();
  initSysTick();
  initKeyboard();
  tm1637_init();

  printf("Calculator initialized.\n");

  GPIOA->ODR |= (1 << 5); // Включаем LED

  CalcState state = CALC_STATE_NUM1;
  int num1 = 0;
  int num2 = 0;
  int op_idx = 0;
  const char ops[] = {'+', '-', '*', '/'};
  char prevKey = '\0';

  while (1) {
    scanKeyboard();

    // Обработка фронта нажатия новой клавиши
    if (lastKey != '\0' && lastKey != prevKey) {
      char key = lastKey;
      prevKey = lastKey;

      if (key >= '0' && key <= '9') {
        int val = key - '0';
        if (state == CALC_STATE_NUM1 || state == CALC_STATE_DONE) {
          num1 = val;
          state = CALC_STATE_NUM1;
          tm1637_display_number(num1);
          printf("Num1 = %d\n", num1);
        } else if (state == CALC_STATE_OP || state == CALC_STATE_NUM2) {
          num2 = val;
          state = CALC_STATE_NUM2;
          tm1637_display_number(num2);
          printf("Num2 = %d\n", num2);
        }
      } else if (key == '*') {
        if (state == CALC_STATE_NUM1 || state == CALC_STATE_DONE) {
          op_idx = 0; // Начинаем с '+'
          state = CALC_STATE_OP;
        } else if (state == CALC_STATE_OP || state == CALC_STATE_NUM2) {
          op_idx = (op_idx + 1) % 4; // Циклический перебор операций
          state = CALC_STATE_OP;
        }
        show_op(ops[op_idx]);
        printf("Operation: %c\n", ops[op_idx]);
      } else if (key == '#') {
        if (state == CALC_STATE_NUM2) {
          int res = 0;
          switch (ops[op_idx]) {
            case '+':
              res = num1 + num2;
              printf("%d + %d = %d\n", num1, num2, res);
              show_result(res);
              num1 = res;
              break;
            case '-':
              res = num1 - num2;
              printf("%d - %d = %d\n", num1, num2, res);
              show_result(res);
              num1 = res;
              break;
            case '*':
              res = num1 * num2;
              printf("%d * %d = %d\n", num1, num2, res);
              show_result(res);
              num1 = res;
              break;
            case '/':
              if (num2 == 0) {
                printf("Error: Division by zero!\n");
                show_error();
              } else {
                res = num1 / num2;
                printf("%d / %d = %d\n", num1, num2, res);
                show_result(res);
                num1 = res;
              }
              break;
          }
          state = CALC_STATE_DONE;
        }
      }
    } else if (lastKey == '\0') {
      prevKey = '\0';
    }
  }

  return 0;
}