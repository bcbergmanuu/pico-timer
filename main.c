#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include "hardware/spi.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include <stdbool.h>
#include <math.h>

#include <string.h>

#include "rp2040_shift_register.h"

//0-9
uint8_t numbers_to_display[10] = {219,144,93,213,150,199,207,145,223,215};

uint8_t numbermask(uint8_t number, bool dot) {
  return numbers_to_display[number] + (dot ? 32 :0);
}

void drawnumber(ShiftRegister reg, uint16_t number) {
  uint8_t d_ones, d_tens, d_hundreds, d_tenths = 0;
  d_tenths = number % 10;
  d_ones = (number % 100-d_tens)/10;
  d_tens = (number % 1000-d_tenths-d_ones)/100;
  d_hundreds = (number % 10000-d_tenths-d_tens-d_ones)/1000;

  if(number < 1000) {
    shift_register_write_bitmask(&reg, numbermask(d_tenths, 0));
  }
    shift_register_write_bitmask(&reg, numbermask(d_ones, 1));
    shift_register_write_bitmask(&reg, numbermask(d_tens, 0));
  if(number > 999) {
    shift_register_write_bitmask(&reg, numbermask(d_hundreds, 0));
  }

  shift_register_flush(&reg);
}

void main()
{
  stdio_init_all();
  ShiftRegister reg = shift_register_new((PinConfig){
      .SERIAL_PIN = 9,
      .SHIFT_REGISTER_CLOCK_PIN = 11,
      .STORAGE_REGISTER_CLOCK_PIN = 10});

  int switch_on = 0;

  while (true)
  {                
    for(uint16_t i = 0; i<10000; i+= (i < 1000 ? 1 : 10)) {                          
      drawnumber(reg, i);
      
      if(i < 1000) {
        sleep_ms(100);
      } else {
        sleep_ms(1000);
      }      
    }     
  }
}