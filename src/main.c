/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32l152c_discovery.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

// Command received from USART
uint8_t ordenes=0;
uint8_t ordenes_ant=9;
// Character received from the PC
uint8_t texto[7] = "       ";
// Messages sent to the PC
uint8_t mensaje_stop[7]       = "Stop\r\n";
uint8_t mensaje_forward[10]   = "Forward\r\n";
uint8_t mensaje_backwards[12] = "Backwards\r\n";
uint8_t mensaje_right[8]      = "Right\r\n";
uint8_t mensaje_left[7]       = "Left\r\n";
uint8_t mensaje_auto[12]      = "Automatic\r\n";
uint8_t mensaje_linea[17]     = "Line detected!\r\n";
uint8_t mensaje_vel1[10]      = "Speed 1\r\n";
uint8_t mensaje_vel2[10]      = "Speed 2\r\n";
uint8_t mensaje_vel3[10]      = "Speed 3\r\n";
uint8_t mensaje_vel4[10]      = "Speed 4\r\n";

// Automatic mode flag
unsigned char autom = 0;

// Sensor and buzzer state variables
unsigned char sensor1 = 0;
unsigned char sensor2 = 0;
unsigned char prueba = 0;
// Start condition variables
unsigned char ready = 0;
int go = 0;
// Potentiometer reading
int potentiometer = 0;

// Maximum speed value
unsigned char speed = 0;

// PWM duty cycle value
short DC=9;
// Line correction flag
short avoidance = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// TIM4 interrupt handler
void TIM4_IRQHandler(void) {
	if ((TIM4->SR & 0x0008)!=0){ // Channel 3 interrupt flag
		// A value of 0 means white surface
		sensor1 = (GPIOC->IDR & 0x00000002);
		sensor2 = (GPIOC->IDR & 0X00000004);
		if (ready != 0 && go < 20){
			go = go + 1;
		}
		// Read potentiometer value
		potentiometer = ADC1->DR; // 12-bit ADC value
		if(potentiometer > 3100){ // Maximum power
			speed = 9;
		}else if((potentiometer > 2600) & (potentiometer < 3100)){ // Medium power range
			speed = 8;
		}else if((potentiometer < 2600) & (potentiometer > 280)){
			speed = 7;
		}else if(potentiometer < 280){
			speed = 5;
		}
		if((ready != 0) && (go >= 20) && (autom != 0)){ // Start automatic mode after the safety delay
			// Stop backward movement before entering automatic forward control
			GPIOC->BSRR = (1<<7)<<16;
			GPIOC->BSRR = (1<<9)<<16;
			DC = speed;
			if(prueba==0){
				prueba = 1;
				if((sensor1 != 0) && (sensor2 == 0)){// Sensor 1 detects black, sensor 2 detects white
					GPIOB->BSRR = (1<<8);
					TIM3->CCR1 = DC;
					TIM3->CCR3 = 0;
					HAL_UART_Transmit_IT(&huart1, mensaje_linea, 17);
				}else if((sensor1 == 0) && (sensor2 != 0)){
					GPIOB->BSRR = (1<<8);
					TIM3->CCR1 = 0;
					TIM3->CCR3 = DC;
					HAL_UART_Transmit_IT(&huart1, mensaje_linea, 17);
				}else if((sensor1 == 0) && (sensor2 == 0)){
					GPIOB->BSRR = (1<<8)<<16;
					TIM3->CCR1 = DC;
					TIM3->CCR3 = DC;
				}else if((sensor1 != 0) && (sensor2 != 0)){
					GPIOB->BSRR = (1<<8);
					TIM3->CCR1 = 0;
					TIM3->CCR3 = 0;
				}
			}else{
				prueba = 0;
				if((sensor1 != 0) && (sensor2 == 0)){
					GPIOB->BSRR = (1<<8)<<16;
					TIM3->CCR1 = DC;
					TIM3->CCR3 = 0;
				}else if((sensor1 == 0) && (sensor2 != 0)){
					GPIOB->BSRR = (1<<8)<<16;
					TIM3->CCR1 = 0;
					TIM3->CCR3 = DC;
				}else if((sensor1 == 0) && (sensor2 == 0)){
					GPIOB->BSRR = (1<<8)<<16;
					TIM3->CCR1 = DC;
					TIM3->CCR3 = DC;
				}else if((sensor1 != 0) && (sensor2 != 0)){
					GPIOB->BSRR = (1<<8);
					TIM3->CCR1 = 0;
					TIM3->CCR3 = 0;
				}
			}
		}
		TIM4->CNT = 0;
		TIM4->SR = 0x0000;// Clear interrupt flags
	}
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM4_Init();
  MX_ADC_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  texto[1] = 0;
  HAL_UART_Receive_IT(&huart1, &(texto[1]), 1);// Receive one character by interrupt

  // TIM4 channel 3: periodic interrupt and buzzer timing

    TIM4->CR1 = 0x0000;
    TIM4->CR2 = 0x0000;
    TIM4->SMCR = 0x0000;

    TIM4->PSC = 31999;     // Prescaler = 32000 -> f_cnt = 1000 steps/s
    TIM4->CNT = 0;
    TIM4->ARR = 0xFFFF;

    TIM4->CCR3 = 100;     // Trigger interrupt every 0.1 s
    TIM4->DIER = 0x0008;  // Channel 3 interrupt enable

    TIM4->CCMR2 &= ~(0x00FF) ; // Clear channel 3 configuration in CCMR2
    TIM4->CCMR2 |= 0x0030;     // CC3S = 0 -> output compare mode
    						   // OC3M = 011 -> toggle mode
    						   // OC3PE = 0 -> no preload

    TIM4->CCER &= ~(0x0F00); // Clear channel 3 configuration in CCER
    TIM4->CCER |= 0x0100;    // Enable channel 3 output

    TIM4->EGR |= 0x0001;  // UG = 1 -> generate update event
    TIM4->SR = 0;         // Clear counter flags
    TIM4->CR1 |= 0x0001;  // Start counter


    NVIC->ISER[0] |= (1 << 30); // Enable TIM4 interrupt in NVIC



    // TIM3 PWM configuration for motor control
     TIM3->CR1 = 0x0080;     // ARPE = 1 -> enable auto-reload preload
     TIM3->CR2 = 0x0000;
     TIM3->SMCR =0x0000;

     // Counter configuration
     TIM3->PSC= 31999;
     TIM3->CNT= 0;
     TIM3->ARR= 9;        // PWM frequency: 100 Hz, 10 duty-cycle steps
     TIM3->CCR1=DC;      // Channels 1 and 3 use variable duty cycle
     TIM3->CCR3=DC;

     TIM3->DIER=0x0000;  // Timer interrupt disabled for PWM
     // Output compare mode
     TIM3->CCMR1=0x0068; // Channel 1
     	 	 	 	 	 // PWM output mode
                         // OCxM = 110 -> PWM mode 1
                         // OCxPE = 1 -> preload enabled
     TIM3->CCMR2=0x0068; // Channel 3
     	 	 	 	 	 // PWM output mode
                         // OCxM = 110 -> PWM mode 1
                         // OCxPE = 1 -> preload enabled

     TIM3->CCER= 0x0101; // Active high polarity
                         // Enable hardware output on channels 1 and 3

     // Enable counter
     TIM3->CR1 |=0x0001; // CEN = 1 -> start counter
     TIM3->EGR |=0x0001; // UG = 1 -> generate update event
     TIM3->SR =0;         // Clear counter flags

    // ADC configuration
    ADC1->CR2&= ~(0x00000001); // ADON = 0 -> ADC disabled
    ADC1->CR1 = 0x00000000;    // OVRIE = 0 -> overrun interrupt disabled
                               // RES = 00 -> 12-bit resolution
                               // SCAN = 0 -> scan mode disabled
                               // EOC interrupt disabled

     ADC1->CR2=0x0000412;       // EOCS = 1 -> EOC after each conversion
                                // DELS = 000 -> no delay
     ADC1->SMPR1=0;
     ADC1->SMPR2=0;
     ADC1->SMPR3=0;
     // CONT = 0 -> single conversion mode
     ADC1->SQR1=0x00000000;     // One channel in the conversion sequence
     ADC1->SQR5=0x00000005;     // Selected channel: AIN5
     ADC1->CR2|=0x00000001;     // ADON = 1 -> ADC enabled

     // Start ADC conversion
     while((ADC1->SR&0x0040)==0); // Wait until ADC is ready

     ADC1->CR2 |= 0x40000000;     // Start conversion with SWSTART
     //---------------------------------------------------------------------------------------


    // GPIO configuration

    // Line sensor 1 on PC1
    GPIOC->MODER &= ~(1 << (2*1));
    GPIOC->MODER |= (0 << (1*2));

    // Line sensor 2 on PC2
    GPIOC->MODER &= ~(1 << (2*2));
    GPIOC->MODER |= (0 << (2*2));

    // Potentiometer on PA5 as analog input
    GPIOA->MODER |=0x00000C00;

    // Buzzer on PB8

	 GPIOB->MODER |= (1 << 8*2);
	 GPIOB->MODER &= ~(1 << (8*2 + 1));
     // AF2 -> TIM4
     GPIOB->AFR[1] |= (0x02 << (2*1));

    // Motor pins

     // Forward motor control
     // IN1
	 GPIOC->MODER |=0x00000001<<(2*6 +1);
	 GPIOC->MODER &=~(0x00000001<<(2*6));
	 // AF2 -> TIM3
	 GPIOC->AFR[0] |= (0x02 << (2*6));

     // IN3
     GPIOC->MODER |=0x00000001<<(2*8 +1);
     GPIOC->MODER &=~(0x00000001<<(2*8));
     // AF2 -> TIM3
     GPIOC->AFR[1] |= (0x02 << (2*0));

     // Backward motor control
     // IN2
     GPIOC->MODER |= (1 << 7*2);
     GPIOC->MODER &= ~(1 << (7*2 + 1));
     // IN4
     GPIOC->MODER |= (1 << 9*2);
     GPIOC->MODER &= ~(1 << (9*2 + 1));

     // User button on PA0 as input
     GPIOA->MODER &=~(1<<(0*2 +1));
     GPIOA->MODER &=~(1<<(0*2));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(ordenes!=ordenes_ant){
      switch(ordenes){
        case '0': // Stop
          HAL_UART_Transmit_IT(&huart1, mensaje_stop, 7);
          autom = 0;
          TIM3->CCR1 = 0;
          TIM3->CCR3 = 0;
          GPIOC->BSRR = (1<<7)<<16;
          GPIOC->BSRR = (1<<9)<<16;
          ordenes_ant = ordenes;
          break;
        case '1': // Forward
          autom = 0;
          HAL_UART_Transmit_IT(&huart1, mensaje_forward, 10);
          TIM3->CCR1 = DC;
          TIM3->CCR3 = DC;
          GPIOC->BSRR = (1<<7)<<16;
          GPIOC->BSRR = (1<<9)<<16;
          ordenes_ant = ordenes;
          break;
        case '2': // Backwards
          autom = 0;
          HAL_UART_Transmit_IT(&huart1, mensaje_backwards, 12);
          TIM3->CCR1 = 0;
          TIM3->CCR3 = 0;
          GPIOC->BSRR = (1<<7);
          GPIOC->BSRR = (1<<9);
          ordenes_ant = ordenes;
          break;
        case '3': // Right
          autom = 0;
          HAL_UART_Transmit_IT(&huart1, mensaje_right, 8);
          TIM3->CCR1 = 0;
          TIM3->CCR3 = DC;
          GPIOC->BSRR = (1<<7)<<16;
          GPIOC->BSRR = (1<<9)<<16;
          ordenes_ant = ordenes;
          break;
        case '4': // Left
          autom = 0;
          HAL_UART_Transmit_IT(&huart1, mensaje_left, 7);
          TIM3->CCR1 = DC;
          TIM3->CCR3 = 0;
          GPIOC->BSRR = (1<<7)<<16;
          GPIOC->BSRR = (1<<9)<<16;
          ordenes_ant = ordenes;
          break;
        case '5': // Automatic mode
          HAL_UART_Transmit_IT(&huart1, mensaje_auto , 12);
          GPIOC->BSRR = (1<<7)<<16;
          GPIOC->BSRR = (1<<9)<<16;
          TIM3->CCR1 = 0;
          TIM3->CCR3 = 0;
          autom = 1;
          ordenes_ant = ordenes;
          break;
        case '6': // Speed level 1
          DC = speed * 0.6;
          ordenes = ordenes_ant;
          ordenes_ant = 6;
          HAL_UART_Transmit_IT(&huart1, mensaje_vel1 , 10);
          break;
        case '7': // Speed level 2
          DC = speed * 0.7;
          ordenes = ordenes_ant;
          ordenes_ant = 7;
          HAL_UART_Transmit_IT(&huart1, mensaje_vel2 , 10);
          break;
        case '8': // Speed level 3
          DC = speed * 0.8;
          ordenes = ordenes_ant;
          ordenes_ant = 8;
          HAL_UART_Transmit_IT(&huart1, mensaje_vel3 , 10);
          break;
        case '9': // Maximum speed
          DC = speed;
          ordenes = ordenes_ant;
          ordenes_ant = 9;
          HAL_UART_Transmit_IT(&huart1, mensaje_vel4 , 10);
          break;
      }

	  }
	  if(ready == 0){
		  TIM3->CCR1 = 0;
		  TIM3->CCR3 = 0;
	  }
	  if((GPIOA->IDR&0x00000001)!=0){
		  ready = 1;
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC1 PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC7 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

 HAL_UART_Receive_IT(huart, &(texto[1]), 1); // Re-enable UART reception after receiving one character
 ordenes = texto[1];
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
