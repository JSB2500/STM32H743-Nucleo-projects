#include "string.h"
#include "stm32h7xx_nucleo.h"
#include "stm32h7xx_hal.h"
#include "tim.h"
#include "usbd_cdc_if.h"
#include "go.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  BSP_LED_Toggle(LED_BLUE);
}

///////////////////////////////////////////////////////////////////////////////
// MCU stuff:

#define __D2_32ByteAligned __attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned(32)))

uint32_t GetAPB1TimerClockFrequency()
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  uint32_t Result;

  Result = 2 * HAL_RCC_GetPCLK1Freq();

  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInit);

// This needs updating from F7 to H7:
//  if (PeriphClkInit.TIMPresSelection == RCC_TIMPRES_ACTIVATED)
//    Result *= 2;

  return Result;
}

///////////////////////////////////////////////////////////////////////////////

int wrap(int Value, int Size)
{
  if (Value < 0)
    Value += Size;
  else if (Value >= Size)
    Value -= Size;

  return Value;
}

///////////////////////////////////////////////////////////////////////////////

#define CalculateFrequency_NumSamples (4096)
#define CalculateFrequency_NumReadSamples (10)
#define CalculateFrequency_MinFrequency_StopThreshold (10.0f)
#define CalculateFrequency_MinFrequency_StartThreshold (12.0f)

static __D2_32ByteAligned uint32_t CalculateFrequency_Samples[CalculateFrequency_NumSamples];
static uint8_t InputStopped = 0;
static float CalculateFrequency_TimerCountFrequency = 0.0f;

void CalculateFrequency_Reset()
{
}

uint8_t CalculateFrequency(float *o_pFrequency)
{
  uint32_t ReadIndex, WriteIndex;
  uint32_t AverageTimePerCycle_InCounterCycles;
  uint32_t CurrentTimerCount, CurrentSampleCount, NumCountsSinceLastSample;
  float Frequency, TimeSinceLastSample;

  *o_pFrequency = 0.0f;

  SCB_InvalidateDCache_by_Addr(CalculateFrequency_Samples, sizeof(CalculateFrequency_Samples));

  WriteIndex = CalculateFrequency_NumSamples - __HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC1]);
  if (WriteIndex >= CalculateFrequency_NumSamples)
    return 0;
  ReadIndex = wrap(WriteIndex - 1, CalculateFrequency_NumSamples);

  CurrentTimerCount = __HAL_TIM_GetCounter(&htim2);
  CurrentSampleCount = CalculateFrequency_Samples[ReadIndex];

  NumCountsSinceLastSample = CurrentTimerCount - CurrentSampleCount;
  TimeSinceLastSample = (float)NumCountsSinceLastSample / (float)CalculateFrequency_TimerCountFrequency;

  AverageTimePerCycle_InCounterCycles = (CurrentSampleCount - CalculateFrequency_Samples[wrap(ReadIndex - CalculateFrequency_NumReadSamples, CalculateFrequency_NumSamples)]) / CalculateFrequency_NumReadSamples;
  Frequency = (float)CalculateFrequency_TimerCountFrequency / (float)AverageTimePerCycle_InCounterCycles;

  if (InputStopped)
  {
    if (Frequency >= CalculateFrequency_MinFrequency_StartThreshold)
      InputStopped = 0;
  }
  else
  {
    if ((Frequency < CalculateFrequency_MinFrequency_StopThreshold) || (TimeSinceLastSample >= 1.0f / CalculateFrequency_MinFrequency_StopThreshold))
      InputStopped = 1;
  }

  if (InputStopped)
    Frequency = 0.0f;

  *o_pFrequency = Frequency;

  return 1;
}

///////////////////////////////////////////////////////////////////////////////

void Go()
{
  float InputFrequency;
  char S[2048];

  CalculateFrequency_TimerCountFrequency = (float)GetAPB1TimerClockFrequency() / ((float)htim2.Instance->PSC + 1.0f);

  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

  if (HAL_TIM_Base_Start(&htim2))
    Error_Handler();
  if (HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, CalculateFrequency_Samples, CalculateFrequency_NumSamples) != HAL_OK)
    Error_Handler();

  CalculateFrequency_Reset();

  while(1)
  {
    if (CalculateFrequency(&InputFrequency))
    {
      // if (BSP_PB_GetState(BUTTON_USER))
      {
        sprintf(S, "Input frequency: %0.2f\r\n", InputFrequency);
        CDC_Transmit_FS((uint8_t *)S, strlen(S));
      }
    }

    HAL_Delay(1);
  }
}
