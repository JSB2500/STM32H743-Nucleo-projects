#include <stdio.h>
#include <string.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "usbd_cdc_if.h"
#include "tim.h"
#include "go.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  BSP_LED_Toggle(LED_BLUE);
}

static char USBBuffer[2048];

void Go()
{
  int32_t Counter = 0;

  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);

  HAL_TIM_Base_Start_IT(&htim2);

  while(1)
  {
    if (BSP_PB_GetState(BUTTON_USER))
    {
      BSP_LED_On(LED_GREEN);
      sprintf(USBBuffer, "Counter value: %ld\n", Counter);
      CDC_Transmit_FS((uint8_t *)USBBuffer, strlen(USBBuffer));
    }
    else
      BSP_LED_Off(LED_GREEN);

    ++Counter;

    HAL_Delay(10);
  }
}
