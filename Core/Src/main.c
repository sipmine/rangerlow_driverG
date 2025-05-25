/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include <stdbool.h>
#include <string.h>

#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "../Tarantul/motor_controller.h"
#include "../Tarantul/uart_protocol.h"

#define SIZE 4
#define PACKET_SIZE 8
void SystemClock_Config(void);

uint8_t msg[PACKET_SIZE] = {0};

uint8_t core_id = 0x41;

void createEncMessage(const uint8_t CMD,  int32_t count, uint8_t* tx_data)
{
  protocol msg;
  msg.core_id = core_id;
  msg.cmd = CMD;
  uint8_t bytes[SIZE] = {0};
  int_32_to_bytes(count, bytes);
  memcpy(msg.data, bytes,SIZE);
  create_msg(msg, tx_data);
}



/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();

  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

  MotorController_t m1;
  m1.id = M1_CMD_PWM;
  m1.enc_htim = TIM1;
  m1.motor_htim = &htim3;
  m1.forward_ch = TIM_CHANNEL_4;
  m1.backward_ch = TIM_CHANNEL_3;
  m1.counter = 0;

  MotorController_t m2;
  m1.id = M2_CMD_PWM;
  m1.enc_htim = TIM4;
  m1.motor_htim = &htim3;
  m1.forward_ch = TIM_CHANNEL_1;
  m1.backward_ch = TIM_CHANNEL_2;
  m1.counter = 0;


  HAL_UART_Receive_IT(&huart3, msg, PACKET_SIZE);
  if (HAL_GPIO_ReadPin(ADDR_X0_GPIO_Port, ADDR_X0_Pin) == 1)
  {
    core_id = 0x42;
  }
  if (HAL_GPIO_ReadPin(ADDR_X1_GPIO_Port, ADDR_X1_Pin) == 1)
  {
    core_id = 0x43;
  }
  uint8_t csum =0 ;
  uint8_t tx_data[PACKET_SIZE] = {0xA5, core_id, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0};
  for (int i = 0; i < 5; ++i) csum ^= tx_data[i];
  tx_data[PACKET_SIZE-1] = csum;

  bool success = false;

  HAL_UART_Transmit_IT(&huart3, tx_data, PACKET_SIZE);
  while (1)
  {
    if (!success)
    {

      HAL_UART_Transmit_IT(&huart3, tx_data, PACKET_SIZE);
      if (huart3.RxXferCount == 0)
      {
        HAL_UART_Receive_IT(&huart3, msg, PACKET_SIZE);
        if (memcmp(tx_data, msg, PACKET_SIZE) == 0) success = true;
      }

    } else
    {
      MotorController_UpdateEnc(&m1);
      MotorController_UpdateEnc(&m2);
      uint8_t tx_buffer_enс_1[PACKET_SIZE] = {0};
      uint8_t tx_buffer_enс_2[PACKET_SIZE] = {0};

      createEncMessage(M1_CMD_ENC, MotorController_GetCounter(&m1), tx_buffer_enс_1);
      createEncMessage(M2_CMD_ENC, MotorController_GetCounter(&m2), tx_buffer_enс_2);

      HAL_UART_Transmit(&huart3, tx_buffer_enс_1, PACKET_SIZE, 100);
      HAL_UART_Transmit(&huart3, tx_buffer_enс_2, PACKET_SIZE, 100);

      if (huart3.RxXferCount == 0) // Reception complete
      {
        protocol data;
        parse_msg(&data, msg);

        if (data.core_id == core_id )
        {
          if (data.cmd == m1.id)
          {
            MotorController_SetSpeed(&m1, bytes_to_float(data.data));
          }
          else if (data.cmd == m2.id)
          {
            MotorController_SetSpeed(&m2, bytes_to_float(data.data));
          }
        }


        // Restart reception
        HAL_UART_Receive_IT(&huart3, msg, 6);
      }
    }
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
