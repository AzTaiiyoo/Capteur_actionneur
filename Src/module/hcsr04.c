// system includes
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

// user includes
#include "hcsr04.h"
#include "watchdog.h"

static bool hcsr04_trigger_flag = false;

int hcsr04_trigger(void) {

}
void hcsr04_init(GPIO_TypeDef *trigger_port, uint16_t trigger_pin, GPIO_TypeDef *echo_port, uint16_t echo_pin) {

}

void hcsr04_start(watchdog_t *watchdog) {
}