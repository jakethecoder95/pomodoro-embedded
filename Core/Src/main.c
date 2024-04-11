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
#include "stdbool.h"

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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int INCREMENTOR = 5;

enum RotaryRotation {
    DEC_ROTATION = 0, INC_ROTATION = 1, NONE = 2
};

enum PomodoroMode {
    FOCUS_SELECT = 0, REST_SELECT = 1, FOCUSING = 2, RESTING = 3
};

struct RotaryEncoderState {
    uint32_t position;
    uint32_t prev_position;

    bool pressed;
    uint32_t last_pressed;
    uint32_t last_released;
};

struct PomodoroState {
    int focus;
    int rest;
    enum PomodoroMode mode;

    bool paused;
    uint32_t elapsed_time;
    uint32_t prev_time;
};

struct PomodoroState pomodoro;
struct RotaryEncoderState encoder;

bool button_debounce(uint32_t curmilli) {
    uint8_t debounce_time = 10;
    return curmilli - encoder.last_pressed > debounce_time
            && curmilli - encoder.last_released > debounce_time;
}

enum RotaryRotation get_rotate_dir(struct RotaryEncoderState encoder) {
    if (encoder.prev_position == 0 && encoder.position == 65532) {
        return DEC_ROTATION;
    }
    if (encoder.prev_position == 65532 && encoder.position == 0) {
        return INC_ROTATION;
    }
    if (encoder.position > encoder.prev_position) {
        return INC_ROTATION;
    }
    if (encoder.position < encoder.prev_position) {
        return DEC_ROTATION;
    }
    return NONE;
}

void init_pomodoro(int focus, int rest, uint32_t curmilli) {
    pomodoro.focus = focus;
    pomodoro.rest = rest;
    pomodoro.mode = FOCUS_SELECT;
    pomodoro.paused = true;
    pomodoro.elapsed_time = 0;
    pomodoro.prev_time = curmilli;
}

void init_encoder() {
    encoder.position = 0;
    encoder.prev_position = 0;
    encoder.last_pressed = 0;
    encoder.last_released = 0;
    encoder.pressed = false;
}

void on_encoder_rotate(uint32_t position) {
    encoder.position = position;

    if (pomodoro.mode == FOCUSING || pomodoro.mode == RESTING) {
        return;
    }

    enum RotaryRotation dir = get_rotate_dir(encoder);
    if (dir == INC_ROTATION) {
        if (pomodoro.mode == FOCUS_SELECT) {
            pomodoro.focus += INCREMENTOR;
        } else {
            pomodoro.rest += INCREMENTOR;
        }
    } else if (dir == DEC_ROTATION) {
        if (pomodoro.mode == FOCUS_SELECT) {
            pomodoro.focus -= INCREMENTOR;
        } else {
            pomodoro.rest -= INCREMENTOR;
        }
    }

    encoder.prev_position = position;
}

void on_encoder_pressed(uint32_t curmilli) {

    if (curmilli - encoder.last_pressed < 300) {
        init_pomodoro(pomodoro.focus, pomodoro.rest, curmilli);
    } else {

        if (pomodoro.mode == FOCUS_SELECT) {
            pomodoro.mode = REST_SELECT;
        } else if (pomodoro.mode == REST_SELECT) {
            pomodoro.mode = FOCUSING;
            pomodoro.paused = false;
            pomodoro.prev_time = curmilli;
            pomodoro.elapsed_time = 0;
        } else {
            pomodoro.paused = !pomodoro.paused;
            pomodoro.prev_time = curmilli;
        }
    }

    encoder.last_pressed = curmilli;
    encoder.pressed = true;
}

void on_encoder_released(uint32_t curmilli) {
    encoder.last_released = curmilli;
    encoder.pressed = false;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    uint32_t position = __HAL_TIM_GET_COUNTER(htim);
    if (position % 4 == 0) {
        on_encoder_rotate(position);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint32_t curmilli = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_4 && button_debounce(curmilli)) {
        if (!encoder.pressed
                && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) {
            on_encoder_pressed(curmilli);
        }
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    init_encoder();
    init_pomodoro(25, 5, 0);

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */

    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        uint32_t curmilli = HAL_GetTick();

        // Handle Release Button
        if (encoder.pressed
                && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET
                && button_debounce(curmilli)) {
            on_encoder_released(HAL_GetTick());
        }

        // Handle pomodoro timers
        if (!pomodoro.paused
                && (pomodoro.mode == FOCUSING || pomodoro.mode == RESTING)) {

            uint16_t target_time;
            if (pomodoro.mode == FOCUSING) {
//			  target_time = pomodoro.focus * 60 * 1000;
                target_time = pomodoro.focus * 1000;
            } else {
//			  target_time = pomodoro.rest * 60 * 1000;
                target_time = pomodoro.rest * 1000;
            }

            pomodoro.elapsed_time += curmilli - pomodoro.prev_time;
            pomodoro.prev_time = curmilli;

            if (pomodoro.elapsed_time >= target_time) {
                if (pomodoro.mode == FOCUSING) {
                    pomodoro.mode = RESTING;
                } else {
                    pomodoro.mode = FOCUSING;
                }
                pomodoro.elapsed_time = 0;
            }
        }
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

    /* USER CODE BEGIN TIM2_Init 0 */

    /* USER CODE END TIM2_Init 0 */

    TIM_Encoder_InitTypeDef sConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    /* USER CODE BEGIN TIM2_Init 1 */

    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 65535;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 0;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 0;
    if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
            != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */

    /* USER CODE END TIM2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PA4 */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
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
