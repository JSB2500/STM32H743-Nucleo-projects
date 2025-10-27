#include <string.h>
#include <math.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "stm32h7xx_nucleo_144.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "JSB_General.h"
#include "JSB_DWT.h"
#include "go.h"

#define TxPacket_Data_NumBytes (2048)
typedef struct
{
  GUID_t FormatID;
  uint8_t Data[TxPacket_Data_NumBytes];
} TxPacket_t;

static TxPacket_t TxPacket;
static uint32_t NumPacketsSent = 0;

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint32_t TimerCallNumber = 0;

static uint8_t USBLinkBroken = 0;

// static float MinTransmissionTime = 1.0f;
// static float MaxTransmissionTime = 0.0f;

//void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
//{
//  char S[200];
//  sprintf(S, "HAL_PCD_SuspendCallback() called.\r\n");
//  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
//}
//
//void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
//{
//  char S[200];
//  sprintf(S, "HAL_PCD_ResumeCallback() called.\r\n");
//  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  char S[200];
  float AbsoluteTime = HAL_GetTick() / 1000.0f;

  if (htim == &htim3)
  {
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
    {
      sprintf(S, "Device not configured.\r\n");
      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
      return;
    }

    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)(hUsbDeviceFS.pClassData);
    if (!hcdc)
    {
      sprintf(S, "hCDC is zero!\r\n");
      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
      return;
    }

    if ((hcdc->TxState != 0) && (hcdc->TxState != 1))
    {
      sprintf(S, "TxState is %08lx!\r\n", hcdc->TxState);
      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
    }

    if (hcdc->TxState) // Abort if busy.
    {
//      sprintf(S, "%0.3f Busy!\r\n", AbsoluteTime);
//      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
    }
    else if (!BSP_PB_GetState(BUTTON_USER))
    {
      if (hcdc->TxState)
      {
        uint32_t StartTime_Cycles = DWT_GetNumProcessorClockCycles();
        while (hcdc->TxState) {} // Wait for transmission to complete.
        uint32_t EndTime_Cycles = DWT_GetNumProcessorClockCycles();
        float WaitTime = DWT_GetCycleTime() * (EndTime_Cycles - StartTime_Cycles);

        sprintf(S, "%0.3f Waited for %0.6f seconds.\r\n", AbsoluteTime, WaitTime);
        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
      }

      uint32_t StartTime_Cycles = DWT_GetNumProcessorClockCycles();
      while (CDC_Transmit_FS((void *)&TxPacket, sizeof(TxPacket)) == USBD_BUSY) {}

//!!!
      // Wait for transmission to complete. If too long then assume link is broken.
      while (hcdc->TxState)
      {
        uint32_t CurrentTime_Cycles = DWT_GetNumProcessorClockCycles();
        float TransmissionTime = DWT_GetCycleTime() * (CurrentTime_Cycles - StartTime_Cycles);
        if (TransmissionTime > 1.0)
        {
          USBLinkBroken = 1;
          return;
        }
      }


//!!!

//        while (hcdc->TxState) {} // Wait for transmission to complete.
//        uint32_t EndTime_Cycles = DWT_GetNumProcessorClockCycles();
//        float TransmissionTime = DWT_GetCycleTime() * (EndTime_Cycles - StartTime_Cycles);
//        MinTransmissionTime = Min_float(MinTransmissionTime, TransmissionTime);
//        MaxTransmissionTime = Max_float(MaxTransmissionTime, TransmissionTime);
//        sprintf(S, "Transmission time: %0.6f MinTransmissionTime: %0.6f MaxTransmissionTime: %0.6f.\r\n", TransmissionTime, MinTransmissionTime, MaxTransmissionTime);
//        HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);

      //    sprintf(S, "Num packets sent: %u.\r\n", (unsigned int)NumPacketsSent);
      //    HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);

      ++NumPacketsSent;
    }
  }

  ++TimerCallNumber;
}

void Go()
{
  char S[200];

  Enable_DWT_Delay();

  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
  BSP_LED_Init(LED_GREEN);

  StringToGUID("{399D6D71-8C66-45C6-893A-FCB678909954}", &TxPacket.FormatID);
  for (int DataByteIndex = 0; DataByteIndex < TxPacket_Data_NumBytes; ++ DataByteIndex)
    TxPacket.Data[DataByteIndex] = 64 + (DataByteIndex % 32);

  sprintf(S, "Waiting for user button to be pressed.\r\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
  while (!BSP_PB_GetState(BUTTON_USER)) {}
  sprintf(S, "User button pressed.\r\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);

  HAL_TIM_Base_Start_IT(&htim3);

  while (1)
  {
    // if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
    if (USBLinkBroken)
    {
      USBLinkBroken = 0;
      sprintf(S, "USB link assumed broken.\r\n");
      sprintf(S, "Reinitializing USB.\r\n");
      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);

//      hUsbDeviceFS.pClass->DeInit(&hUsbDeviceFS, (uint8_t)hUsbDeviceFS.dev_config);
//      HAL_Delay(1000); //!!!JSB
//      hUsbDeviceFS.pClass->Init(&hUsbDeviceFS, (uint8_t)hUsbDeviceFS.dev_config);

      MX_USB_DEVICE_Init();
    }


//    float CurrentTime = DWT_GetElapsedTime();
//
//    while (CDC_Transmit_FS((void *)&TxPacket, sizeof(TxPacket)) == USBD_BUSY)
//    {
//      sprintf(S, "Waiting to send packet at time %0.3f.\r\n", CurrentTime - StartTime);
//      HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
//    }
//
//    sprintf(S, "Packet %d sent.\r\n", (int)NumPacketsSent);
//    HAL_UART_Transmit(&huart3, (uint8_t *)S, strlen(S), 0xFFFF);
//
//    ++NumPacketsSent;
  }
}

