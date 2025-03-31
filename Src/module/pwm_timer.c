#include "stm32f4xx_hal.h"

void PWM_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_TIM1_PARTIAL();
}

TIM_HandleTypeDef htim1;
TIM_OC_InitTypeDef sConfigOC = {0};

void PWM_Timer_Init(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 99;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);
}

void PWM_Channel_Config(void) {
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 50;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

void PWM_Start(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void Set_PWM_Duty_Cycle(uint32_t duty_cycle) {
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, duty_cycle);
}

int main(void) {
    HAL_Init();
    PWM_GPIO_Init();
    PWM_Timer_Init();
    PWM_Channel_Config();
    PWM_Start();

    while (1) {
        Set_PWM_Duty_Cycle(75);
        HAL_Delay(1000);
        Set_PWM_Duty_Cycle(25);
        HAL_Delay(1000);
    }
}
