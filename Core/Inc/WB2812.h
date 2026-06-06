/*
 * WB2812.h
 *
 *  Created on: May 27, 2026
 *      Author: viet.lv
 */

#ifndef INC_WB2812_H_
#define INC_WB2812_H_

//------------------------------------------------------------------------------------//

#include <stdint.h>
#include "stm32f1xx_hal.h"

//------------------------------------------------------------------------------------//

#define WB2812_LUT_GAMMA_LENGTH 														256
#define WB2812_NUM_COLOR																3

#define WB2812_MAX_BALANCE																1
#define WB2812_MIN_BALANCE																0

#define WB2812_MAX_GAMMA																3
#define WB2812_MIN_GAMMA																1.5

#define WB2812_COLOR_BYTE																3
#define WB2812_BIT																		8

#define WB2812_RESET_BIT																50
//------------------------------------------------------------------------------------//

typedef enum{
	WB2812_RED_COLOR_INDEX,
	WB2812_GREEN_COLOR_INDEX,
	WB2812_BLUE_COLOR_INDEX
} WB2812ColorIndexEnum;

typedef enum{
	WB2812_RED_COLOR_INDEX_FORMAT_WB2812,
	WB2812_GREEN_COLOR_INDEX_FORMAT_WB2812,
	WB2812_BLUE_COLOR_INDEX_FORMAT_WB2812
} WB2812ColorIndexFormatWB2812Enum;

//------------------------------------------------------------------------------------//

typedef struct{
	uint8_t *imageByte;
	uint32_t length;
} WB2812Image_t;

typedef struct{
	uint8_t *dutyArray;
	uint32_t length;
} WB2812Duty_t;

//------------------------------------------------------------------------------------//
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void WB2812_Stop();
void WB2812DutyInit(WB2812Duty_t *wb2812Duty);
void WB2812ImageInit(WB2812Image_t *wb2812Image);
void WB2812_SetSize(uint32_t width, uint32_t heigth);
void WB2812_ConvertDutyFormImage(WB2812Image_t *wb2812Image, WB2812Duty_t *wb2812Duty);
static void WB2812_CalculatorLutGamma(WB2812ColorIndexEnum index, float balance, float gamma);
static float WB2812_GetGammaValid(float gamma);
static float WB2812_GetColorBalanceValid(float balance);
void WB2812_SetLutGamma(float greenBalance, float redBalance, float blueBalance, float gamma);
void WB2812_SetTimAndChannel(TIM_HandleTypeDef *tim, uint32_t channel);
void WB2812_SetDutyCode(uint32_t dutyCode0, uint32_t dutyCode1);
void WB2812_Show(WB2812Duty_t *duty);
#endif /* INC_WB2812_H_ */
