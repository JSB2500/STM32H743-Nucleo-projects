#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "main.h"
#include "usart.h"
#include "adc.h"
#include "tim.h"
#include "go.h"

uint8_t ReadInjector(uint32_t *o_pValue)
{
  if (HAL_ADC_Start(&hadc1)!=HAL_OK) return 0;

  if (HAL_ADC_PollForConversion(&hadc1,0xFFFF)!=HAL_OK) return 0;

  *o_pValue=HAL_ADC_GetValue(&hadc1);

  if (HAL_ADC_Stop(&hadc1)!=HAL_OK) return 0;

  return 1;
}

void Go()
{
  char strSendMessage[200],strInjectorValue[100];
  uint32_t TimeElapsed_us,InjectorValue;
  uint8_t CrankshaftState,CamshaftState;

  while(HAL_ADCEx_Calibration_Start(&hadc1,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED)!=HAL_OK);

 HAL_TIM_Base_Start(&htim2);

  while (1)
  {
    CrankshaftState=HAL_GPIO_ReadPin(GPIOA,CrankshaftPositionSensor_Pin);
    CamshaftState=HAL_GPIO_ReadPin(GPIOA,CamshaftPositionSensor_Pin);
    if (ReadInjector(&InjectorValue))
      sprintf(strInjectorValue, "%d",(int)InjectorValue);
    else
      sprintf(strInjectorValue, "Error");

    TimeElapsed_us=__HAL_TIM_GetCounter(&htim2);

    sprintf(strSendMessage, "%u,%01d,%01d,%s\n",(unsigned int)TimeElapsed_us,CrankshaftState,CamshaftState,strInjectorValue);

    HAL_UART_Transmit(&huart3, (uint8_t *)strSendMessage,strlen(strSendMessage),0xFFFF);
  }
}
