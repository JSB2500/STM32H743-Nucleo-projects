#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "go.h"

#define ADC_FullRangeVoltage (3.3f / 0.5f) /* Includes effect of 2:1 potential divider */
#define ADC_FullRangeCount (65536.0f)

#define LoadResistance (5.0f)

static float IIR_Integral = 0.0f;
static float IIR_Output = 0.0f;

static float BatteryVoltage = 0.0f;
static float BatteryCurrent = 0.0f;

#define Sample_TimerPeriod (5)
#define Samples_MaxNum ((10 * 60 * 60) / Sample_TimerPeriod)
typedef struct
{
  float Time;
  float BatteryVoltage;
  float BatteryCurrent;
} Sample_t;
static Sample_t Samples[Samples_MaxNum] = {};
static uint32_t Samples_Num = 0;
static float Time = 0.0f;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

  float IIR_Input = HAL_ADC_GetValue(hadc);
  float Current = IIR_Input - IIR_Output;
  IIR_Integral += Current;
  IIR_Output = IIR_Integral / 1000.0f; // 10 Hz filter.

  BatteryVoltage = (ADC_FullRangeVoltage / ADC_FullRangeCount) * IIR_Output;
  BatteryCurrent = BatteryVoltage / LoadResistance;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    if (Samples_Num != Samples_MaxNum)
    {
      Sample_t Sample;

      Sample.Time = Time;
      Sample.BatteryVoltage = BatteryVoltage;
      Sample.BatteryCurrent = BatteryCurrent;
      Samples[Samples_Num] = Sample;

      ++Samples_Num;
    }

    Time += Sample_TimerPeriod;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Go()
{
  uint8_t UserButtonAlreadyPressed = 0;
  char S[200];

  Samples_Num = 0;

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  HAL_ADC_Start_IT(&hadc1);
  HAL_TIM_Base_Start(&htim3); // ADC sample timer.

  sprintf(S, "JSB lithium battery tester. Assumes:\r\n(1) 5 ohm discharge resistor.\r\n(2) 2:1 potential divider.\r\n(3) Input to ADC1 IN5.\r\n(4) Battery has internal discharge protection.\r\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
  sprintf(S, "Press user button at any time to dump measurements so far.\r\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);

  HAL_Delay(5000); // Wait for IIR to settle.

  HAL_TIM_Base_Start_IT(&htim2); // Measurement store timer.

  while(1)
  {
    if  (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin))
    {
      if (!UserButtonAlreadyPressed)
      {
        UserButtonAlreadyPressed = 1;

        sprintf(S, "ElapsedTime Voltage Current\r\n");
        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);

        for (int SampleIndex = 0; SampleIndex < Samples_Num; ++SampleIndex)
        {
          Sample_t Sample = Samples[SampleIndex];

          sprintf(S, "%0.0f %0.3f %0.3f\r\n", round(Sample.Time), Sample.BatteryVoltage, Sample.BatteryCurrent);
          HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
        }
      }
    }
    else
    {
      UserButtonAlreadyPressed = 0;
    }
  }
}
