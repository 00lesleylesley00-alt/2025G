#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "usart.h"
#include <stdio.h>
#include "AD9959.h"
#include "math.h"
#include "stdio.h"

#define RINGBUFFER_LEN	(500)

void initRingBuffer(void);
void write1ByteToRingBuffer(uint8_t data);
void deleteRingBuffer(uint16_t size);
uint8_t read1ByteFromRingBuffer(uint16_t position);
uint16_t getRingBufferLength(void);
uint8_t isRingBufferOverflow(void);

#define udelete(x) deleteRingBuffer(x)
#define u(x) read1ByteFromRingBuffer(x)
#define usize getRingBufferLength()

void process(void);
extern uint8_t RxBuffer[1];

#endif
