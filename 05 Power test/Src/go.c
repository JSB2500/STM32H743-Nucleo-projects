
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include "main.h"

void Go()
{
  volatile double A = 1.234;
  volatile double B = 2.345;
  volatile double C, D;

  while(1)
  {
    C = pow(A,B);
    D = sqrt(C);

//    HAL_GPIO_WritePin(JSB_PE0_GPIO_Port, JSB_PE0_Pin, Q & 1);
//    HAL_GPIO_WritePin(JSB_PE1_GPIO_Port, JSB_PE1_Pin, Q & 2);
//    HAL_GPIO_WritePin(JSB_PE2_GPIO_Port, JSB_PE2_Pin, Q & 4);
//    HAL_GPIO_WritePin(JSB_PE3_GPIO_Port, JSB_PE3_Pin, Q & 8);
//    HAL_GPIO_WritePin(JSB_PE4_GPIO_Port, JSB_PE4_Pin, Q & 0x10);
  }
}

