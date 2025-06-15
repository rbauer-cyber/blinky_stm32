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
#include "QFsys.h"
#include "bsp.hpp"
#include "qp_port.hpp"

#include "multiLed.hpp"
#include "digitalOut.hpp"
#include "console.h"

volatile uint16_t g_appReady = 0;
extern CMultiLed g_multiLed;

volatile uint32_t s_lastTime = 0;
volatile uint32_t s_elapsedTime = 0;


void appSysTickHandler()
{
	// Use this variable to communicate with QV::onIdle
	// to indicate that a critical interrupt from the app
	// has occurred and needs to be service.
	// uint32_t currentTime = getMicros();
	// s_elapsedTime = currentTime - s_lastTime;
	// s_lastTime = currentTime;
	if ( g_appReady )
	{
		Q_SysTick_Handler();

		if ( !QF_getSysAppEvent() )
			QF_setSysAppEvent();
	}
}

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

