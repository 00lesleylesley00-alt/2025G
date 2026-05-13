#include "lcd.h"

typedef struct {
  uint16_t Head;                     // 头部指针（读指针）
  uint16_t Tail;                     // 尾部指针（写指针）
  uint16_t Length;                   // 缓冲区长度
  uint8_t Ring_data[RINGBUFFER_LEN]; // 缓冲区数据
} RingBuffer_t;

RingBuffer_t ringBuffer; // 定义一个全局的环形缓冲区实例
uint8_t RxBuffer[1];     // 定义一个接收缓冲区，用于存储从串口接收到的数据

void initRingBuffer(void) {
  // 初始化相关信息
  ringBuffer.Head = 0;
  ringBuffer.Tail = 0;
  ringBuffer.Length = 0;
  return;
}

void write1ByteToRingBuffer(uint8_t data) {
  if (ringBuffer.Length >= RINGBUFFER_LEN) // 判断缓冲区是否已满
  {
    return;
  }
  ringBuffer.Ring_data[ringBuffer.Tail] = data;
  ringBuffer.Tail = (ringBuffer.Tail + 1) % RINGBUFFER_LEN; // 防止越界非法访问
  ringBuffer.Length++;
  return;
}

void deleteRingBuffer(uint16_t size) {
  if (size >= ringBuffer.Length) {
    initRingBuffer();
    return;
  }
  for (int i = 0; i < size; i++) {
    ringBuffer.Head =
        (ringBuffer.Head + 1) % RINGBUFFER_LEN; // 防止越界非法访问
    ringBuffer.Length--;
  }
  return;
}

uint8_t read1ByteFromRingBuffer(uint16_t position) {
  uint16_t realPosition = (ringBuffer.Head + position) % RINGBUFFER_LEN;

  return ringBuffer.Ring_data[realPosition];
}

uint16_t getRingBufferLength(void) { return ringBuffer.Length; }

uint8_t isRingBufferOverflow(void) {
  return ringBuffer.Length < RINGBUFFER_LEN;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

  if (huart->Instance == USART3) {
    write1ByteToRingBuffer(RxBuffer[0]);
    HAL_UART_Receive_IT(&huart3, RxBuffer, 1); // 重新使能串口3接收中断
  }
  return;
}

uint8_t mode = 0, vpp = 0, index = 0;
uint32_t fre = 0, v = 0;
uint64_t f = 0;
float av[30] = {5.12, 4.88, 4.52, 4.24, 3.92, 3.64, 3.36,  3.12,  2.89,  2.69,
                2.5,  2.32, 2.16, 2.04, 1.92, 1.8,  1.68,  1.6,   1.51,  1.43,
                1.36, 1.28, 1.21, 1.15, 1.09, 1.04, 0.984, 0.952, 0.904, 0.856};

void process(void) {
  if (usize >= 8) {
    Uart_printf(&huart1, "MATCH! Output: %x,%x,%x,%x,%x,%x,%x,%x\r\n", u(0),
                u(1), u(2), u(3), u(4), u(5), u(6), u(7));
    if (u(0) == 0x0a && u(6) == 0xFF && u(7) == 0xFF) {
      mode = u(1);
      if (mode == 1) {
        fre = u(2) + u(3) * 256 + u(4) * 65536;
        f = fre * 100;
        v = 1023;
      } else if (mode == 2) {
        fre = u(2) + u(3) * 256 + u(4) * 65536;
        f = fre * 100;
        index = fre - 1;
        vpp = u(5);
        v = (uint32_t)((float)(vpp * 1023) / (3.3f * av[index]));
      }
    }
    udelete(8);
  }
  Write_frequence(0, f);
  Write_Amplitude(0, v);
}
