
#include <Arduino.h>

#include "stm32f4xx.h"
//#include "stm32f429i_discovery.h"
#include "tft.h"
//#include "touchpad.h"

#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif

void hal_setup(void)
{
	pinMode(PD12, OUTPUT);
	digitalWrite(PD12, HIGH);

	// HAL_Init();

	// /* Configure the system clock to 180 MHz */
	// SystemClock_Config();

	// /* Start up indication */
	// BSP_LED_Init(LED3);
	// for (uint8_t i = 0; i < 8; i++) { BSP_LED_Toggle(LED3); delay(50); }

	tft_init();
	//touchpad_init();
}

// void SysTick_Handler(void)
// {
// 	HAL_IncTick();
// 	HAL_SYSTICK_IRQHandler();

// 	lv_tick_inc(1);

// #ifdef USE_RTOS_SYSTICK
// 	osSystickHandler();
// #endif
// }

void hal_loop(void)
{
	//while (1)
	{
		delay(5);
		lv_task_handler();
	}
}
