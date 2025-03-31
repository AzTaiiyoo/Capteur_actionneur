#ifndef HCSR04_H
#define HCSR04_H
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"


extern void hcsr04_init(GPIO_TypeDef *trigger_port, uint16_t trigger_pin, GPIO_TypeDef *echo_port, uint16_t echo_pin);
extern void hcsr04_start(void);
extern void hcsr04_stop(void);
#endif  /* HCSR04_H */