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

volatile uint reset_state = 0;
//0-9
static uint numbers_to_display[10] = {219,144,93,213,150,199,207,145,223,215};
static uint button_locations[6] = {16,21,17,20,18,19};
static uint light_locations[6] = {10,11,6,7,8,9};

uint8_t numbermask(uint8_t number, bool dot) {
  return numbers_to_display[number] + (dot ? 32 :0);
}

void gpio_callback(uint gpio, uint32_t event_mask) {
  if(reset_state > 0) return;
  reset_state = 1; //triggered
  for(int x = 0; x< 6; x++) {
    if (gpio == button_locations[x]) {
      gpio_put(light_locations[x], false);        
    };
  }              
}

void setgpios () {
  //relais
  for(uint i = 6; i< 12; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_OUT);        
    //gpio_set_pulls(i, true, false);
  };
  //input buttons
  for(uint i = 16; i< 22; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_IN);
    gpio_set_pulls(i, true, false);    
    gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  };

  // //displays
  // for(uint8_t i = 3; i< 5; i++) {
  //   gpio_init(i);
  //   gpio_set_dir(i, GPIO_IN);
  //   gpio_set_pulls(i, true, false);
  // };
  
}



void drawnumber(ShiftRegister reg, uint number) {
  uint8_t d_seconds, d_seconds_l, d_seconds_h, d_minutes = 0;
  
  d_minutes = number/60;
  d_seconds = number- (d_minutes*60);
  d_seconds_l = (d_seconds % 10);
  d_seconds_h = (d_seconds % 100-d_seconds_l)/10;
    
  shift_register_write_bitmask(&reg, numbermask(d_seconds_l, 0));
  shift_register_write_bitmask(&reg, numbermask(d_seconds_h, 0));
  shift_register_write_bitmask(&reg, numbermask(d_minutes, 1));

  shift_register_flush(&reg);
}


void triggered_button() {      
  while(true) {    
    sleep_ms(1000);
    //triggered state:
    printf("triggered state %i\n", reset_state);
    if(reset_state == 1 && (gpio_get(button_locations[0]) && gpio_get(button_locations[1]))) {      
      
      sleep_ms(3000);
      reset_state = 2;
      
    }
    //released state:
    if(reset_state == 2 && (!gpio_get(button_locations[0]) && !gpio_get(button_locations[1]))) {      
      printf("released state %i\n", reset_state);
      sleep_ms(1000);
      reset_state = 3;
    } 
    //dubble pressed state   
    if(reset_state == 3 && (gpio_get(button_locations[0]) || gpio_get(button_locations[1]))) {
      //if released by one party, exit.
      printf("dubble pressed state %i\n", reset_state);
      gpio_put(light_locations[0], true);
      gpio_put(light_locations[1], true);
      sleep_ms(100);
      reset_state = 0;      
      return;      
    }
  }      
}

void main()
{
  stdio_init_all();
  setgpios();
  ShiftRegister reg = shift_register_new((PinConfig){
       .SERIAL_PIN = 3,
       .SHIFT_REGISTER_CLOCK_PIN = 14,
       .STORAGE_REGISTER_CLOCK_PIN = 15});

  int switch_on = 0;
  for(int x =6; x<12; x++) {
      gpio_put(x, 1);
  }


  while (true)
  {                
   
    for(uint i = 0; i<599; i++) {                          
     
      drawnumber(reg, i);            
      sleep_ms(1000);              

      if(reset_state > 0) {
        triggered_button();
        i = 0;        
        
      }
    }           
  }
}