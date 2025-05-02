/*
 * altMain.cpp
 *
 *  Created on: Apr 5, 2025
 *      Author: rbauer
 */
#define USE_QUANTUM

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "bsp.hpp"

#include "multiLed.hpp"
#include "digitalOut.hpp"
#include "console.h"

void appSysTickHandler()
{
	Q_SysTick_Handler();
}

static uint8_t s_pins[] =
{
		kDigitalPin08, // PA9
		kDigitalPin09, // PC7
		kDigitalPin10, // PB6
		kDigitalPin11, // PA7
		kDigitalPin12, // PA6
};

static uint8_t s_numPins = sizeof(s_pins) / sizeof(s_pins[0]);
CMultiLed g_multiLed( s_pins, s_numPins );

void BSP_SetLed(uint8_t index, unsigned int state)
{
	g_multiLed.SetLed(index, state);
}

void altMain()
{
	while ( 1 )
	{
		//consoleDisplay("Invoking bspMain\r\n");
		bspMain();
	}
}

