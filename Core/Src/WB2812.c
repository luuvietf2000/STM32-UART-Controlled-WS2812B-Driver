/*
 * WB2812.c
 *
 *  Created on: May 27, 2026
 *      Author: viet.lv
 */


#include "WB2812.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

static uint32_t _dutyCode0 = 0;
static uint32_t _dutyCode1 = 0;
static TIM_HandleTypeDef *_tim = NULL;
static uint32_t _channel = 0;
static uint8_t _lutGammaTable[WB2812_NUM_COLOR][WB2812_LUT_GAMMA_LENGTH] = {0};
static uint32_t _wb2812Size = 0;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	if (htim == _tim) {
		WB2812_Stop();
	}
}

const uint8_t wb2812RgbFormatIndex[WB2812_COLOR_BYTE] =
{	WB2812_GREEN_COLOR_INDEX_FORMAT_WB2812,
	WB2812_RED_COLOR_INDEX_FORMAT_WB2812,
	WB2812_BLUE_COLOR_INDEX_FORMAT_WB2812
};

void WB2812DutyInit(WB2812Duty_t *wb2812Duty){
	wb2812Duty->dutyArray = malloc(sizeof(uint8_t) *( _wb2812Size * WB2812_BIT * WB2812_COLOR_BYTE + WB2812_RESET_BIT));
	wb2812Duty->length = _wb2812Size * WB2812_BIT * WB2812_COLOR_BYTE + WB2812_RESET_BIT;
	for(uint32_t i = wb2812Duty->length - WB2812_RESET_BIT; i < wb2812Duty->length; i++)
		*(wb2812Duty->dutyArray + i) = 0;
}

void WB2812ImageInit(WB2812Image_t *wb2812Image){
	wb2812Image->imageByte = malloc(sizeof(uint8_t) * _wb2812Size * WB2812_COLOR_BYTE);
	wb2812Image->length = _wb2812Size * WB2812_COLOR_BYTE;
	for (uint32_t i = 0; i < wb2812Image->length; i++)
		wb2812Image->imageByte[i] = i % 3 == 1 ? 0x64 : 0;
}

void WB2812_SetSize(uint32_t width, uint32_t heigth){
	_wb2812Size = width * heigth;
}

static inline WB2812ColorIndexFormatWB2812Enum WB2812_GetIndexDuty(WB2812ColorIndexEnum index){
	return wb2812RgbFormatIndex[index];
}

static inline uint8_t WB2812_GetPixelByte(WB2812Image_t *wb2812Image, uint32_t pixelIndex, WB2812ColorIndexEnum colorIndex){
	return wb2812Image->imageByte[pixelIndex * WB2812_COLOR_BYTE + colorIndex];
}

static inline uint8_t WB2812_GetLutGamma(WB2812Image_t *wb2812Image, uint8_t pixelIndex, WB2812ColorIndexEnum colorIndex){
	return _lutGammaTable[colorIndex][WB2812_GetPixelByte(wb2812Image, pixelIndex, colorIndex)];
}
void WB2812_ConvertDutyFormImage(WB2812Image_t *wb2812Image, WB2812Duty_t *wb2812Duty){
	uint32_t indexDuty, gamma, index;
	for(uint32_t pixelIndex = 0 ; pixelIndex < _wb2812Size; pixelIndex++)
		for(WB2812ColorIndexEnum colorIndex = 0; colorIndex < WB2812_COLOR_BYTE; colorIndex++){
			gamma = WB2812_GetLutGamma(wb2812Image, pixelIndex, colorIndex);
			for(uint8_t bitIndex = 0; bitIndex < WB2812_BIT; bitIndex++){
				index = WB2812_BIT - bitIndex - 1;
				indexDuty = (pixelIndex * WB2812_COLOR_BYTE * WB2812_BIT) + (WB2812_GetIndexDuty(colorIndex) * WB2812_BIT) + index;
				wb2812Duty->dutyArray[indexDuty] = (gamma & 1 << index) == 0 ? _dutyCode0 : _dutyCode1;
			}
		}
}

static void WB2812_CalculatorLutGamma(WB2812ColorIndexEnum index, float balance, float gamma){
	float normalized;
	for(uint32_t i = 0; i < WB2812_LUT_GAMMA_LENGTH; i++){
		normalized = (float)i / (WB2812_LUT_GAMMA_LENGTH - 1);
        float value = balance * powf(normalized, gamma) * (WB2812_LUT_GAMMA_LENGTH - 1);
        _lutGammaTable[index][i] = (uint8_t)roundf(value);
	}
}

static float WB2812_GetGammaValid(float gamma){
	if(gamma < WB2812_MIN_GAMMA)
		return WB2812_MIN_GAMMA;
	else if(gamma > WB2812_MAX_GAMMA)
		return WB2812_MAX_GAMMA;
	return gamma;
}


static float WB2812_GetColorBalanceValid(float balance){
	if(balance < WB2812_MIN_BALANCE)
		return WB2812_MIN_BALANCE;
	else if(balance > WB2812_MAX_BALANCE)
		return WB2812_MAX_BALANCE;
	return balance;
}

void WB2812_SetLutGamma(float greenBalance, float redBalance, float blueBalance, float gamma){
	greenBalance = WB2812_GetColorBalanceValid(greenBalance);
	redBalance = WB2812_GetColorBalanceValid(redBalance);
	blueBalance = WB2812_GetColorBalanceValid(blueBalance);
	gamma = WB2812_GetGammaValid(gamma);
	WB2812_CalculatorLutGamma(WB2812_RED_COLOR_INDEX, redBalance, gamma);
	WB2812_CalculatorLutGamma(WB2812_GREEN_COLOR_INDEX, greenBalance, gamma);
	WB2812_CalculatorLutGamma(WB2812_BLUE_COLOR_INDEX, blueBalance, gamma);
}

void WB2812_SetTimAndChannel(TIM_HandleTypeDef *tim, uint32_t channel){
	_tim = tim;
	_channel = channel;
}

void WB2812_SetDutyCode(uint32_t dutyCode0, uint32_t dutyCode1){
	_dutyCode0 = dutyCode0;
	_dutyCode1 = dutyCode1;
}

void WB2812_Show(WB2812Duty_t *duty){
	HAL_StatusTypeDef result = HAL_TIM_PWM_Start_DMA(_tim, _channel, (uint32_t *)duty->dutyArray, duty->length);
}

void WB2812_Stop(){
	HAL_TIM_PWM_Stop_DMA(_tim, _channel);
}
