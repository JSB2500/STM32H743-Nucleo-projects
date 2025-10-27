#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "go.h"

#define ADCValues_MaxNum 10240
static uint32_t ADCValues[ADCValues_MaxNum] = {};
static uint32_t ADCValues_Num = 0;

static float IIR_Integral = 0.0f;
static float IIR_Output = 0.0f;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

  float Input = HAL_ADC_GetValue(hadc);

  if (ADCValues_Num != ADCValues_MaxNum)
  {
    ADCValues[ADCValues_Num] = Input;
    ++ADCValues_Num;
  }

  float Current = Input - IIR_Output;
  IIR_Integral += Current;
  IIR_Output = IIR_Integral / 100.0f; // 100 Hz filter.
}

///////////////////////////////////////////////////////////////////////////////
// Window:

#define Window_NumPoints 100
static volatile float Window_Points[Window_NumPoints];

static const double Window_a0 = 0.35875;
static const double Window_a1 = 0.48829;
static const double Window_a2 = 0.14128;
static const double Window_a3 = 0.01168;

static void Window_Create()
{
  for (uint32_t Index = 0; Index < Window_NumPoints; ++Index)
  {
    double Theta = ((2.0 * M_PI * Index) / (Window_NumPoints - 1));
    float w = Window_a0 - Window_a1 * cos(Theta) + Window_a2 * cos(2.0 * Theta) - Window_a3 * cos(3.0 * Theta);
    Window_Points[Index] = w;
  }
}

//static float Window_Apply(uint32_t *pData)
//{
//  float Total = 0.0f;
//
//  for (uint32_t Index = 0; Index < Window_NumPoints; ++Index)
//    Total += Window_Points[Index] * pData[Index];
//
//  return Total / (Window_a0 * 65536.0f * Window_NumPoints); // 24027920.000000;
//}

///////////////////////////////////////////////////////////////////////////////

void Go()
{
  char S[200];

  Window_Create();

  ADCValues_Num = 0;

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  HAL_ADC_Start_IT(&hadc1);
  HAL_TIM_Base_Start(&htim2);

  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

//  while(1)
//  {
//    sprintf(S, "%0.0f %0.4fV\r\n", IIR_Output, IIR_Output * (3.3f / 65535.0f));
//    HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
//    HAL_Delay(10);
//  }

  while(1)
  {
    if (ADCValues_Num == ADCValues_MaxNum)
    {
      HAL_ADC_Stop_IT(&hadc1);
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

//      float Integral = 0.0f;
//      float Output = 0.0f;
//      for (int SampleIndex = 0; SampleIndex < ADCValues_MaxNum; ++SampleIndex)
//      {
//        float Input = ADCValues[SampleIndex];
//        float Current = Input - Output;
//        Integral += Current;
//        Output = Integral / 1000.0f; // 10 Hz filter.
//      }
//
//      sprintf(S, "%0.0f %0.4fV\r\n", Output, Output * (3.3f / 65535.0f));
//      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
//
//      ADCValues_Num = 0;

      // Send raw samples:
      for (int SampleIndex = 0; SampleIndex < ADCValues_MaxNum; ++SampleIndex)
      {
        sprintf(S, "%d\r\n", (int)ADCValues[SampleIndex]);
        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
      }

// Send IIR filtered samples:
//      float Integral = 0.0f;
//      float Output = 0.0f;
//      for (int SampleIndex = 0; SampleIndex < ADCValues_MaxNum; ++SampleIndex)
//      {
//        float Input = ADCValues[SampleIndex];
//        float Current = Input - Output;
//        Integral += Current;
//        Output = Integral / 1000.0f; // 10 Hz filter.
//
//        sprintf(S, "%0.1f\r\n", Output);
//        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
//      }

// Send FIR filtered samples:
//      for (int SampleStartIndex = 0; SampleStartIndex < 1000; ++SampleStartIndex)
//      {
//        float Result = Window_Apply(&ADCValues[SampleStartIndex]);
//        sprintf(S, "%04d %f\r\n", SampleStartIndex, Result);
//        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 1000);
//      }

      return;
    }
  }
}
