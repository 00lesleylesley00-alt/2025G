#include "fun.h"

uint16_t findMax_uint16(uint16_t *array, uint32_t length) {
  uint16_t max = array[0];
  for (uint32_t i = 1; i < length; i++) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  return max;
}

uint16_t findAverage_uint16(uint16_t *array, uint32_t length) {
  uint32_t sum = 0;
  for (uint32_t i = 0; i < length; i++) {
    sum += array[i];
  }

  return (uint16_t)(sum / length);
}

