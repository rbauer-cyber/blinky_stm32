//============================================================================
// Blinky example, NUCLEO-C031C6 board, QV kernel
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"

#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "blinky.hpp"            // Blinky Application interface
#include "bsp.hpp"               // Board Support Package
#include "console.h"
#include "multiLed.hpp"
#include "stm32f4xx_hal.h"
// add other drivers if necessary...

extern uint16_t g_appReady;

#define USE_LOCAL_CRITICAL_SECTIONS

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

//#define SHOW_BLINK
//#define USE_HAL

extern CMultiLed g_multiLed;

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
constexpr std::uint32_t LD4_PIN     {5U};
constexpr std::uint32_t B1_PIN      {13U};

//Q_DEFINE_THIS_FILE

} // unnamed local namespace

// Define functions for enabling/disabling HAL interrupts for critical sections
// and for setting/detecting Q system events.
#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_LOCAL_CRITICAL_SECTIONS
void QF_int_disable_(void)
{
	//HAL_SuspendTick();
}

void QF_int_enable_(void)
{
	//HAL_ResumeTick();
}

void QF_crit_entry_(void)
{
	//HAL_SuspendTick();
}

void QF_crit_exit_(void)
{
	//HAL_ResumeTick();
}

volatile static uint16_t s_sysAppInterrupt = 0;

volatile void QF_setSysAppEvent()
{
	s_sysAppInterrupt = 1;
}

volatile void QF_clearSysAppEvent()
{
	s_sysAppInterrupt = 0;
}

volatile uint16_t QF_getSysAppEvent()
{
	return s_sysAppInterrupt;
}
#endif

#ifdef __cplusplus
}
#endif

//============================================================================
// Error handler and ISRs...

extern "C" {

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the assertion handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U);
    while (1) {}

#ifndef NDEBUG
    // light up the user LED
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif

    NVIC_SystemReset();
}
//............................................................................
// assertion failure handler for the STM32 library, including the startup code
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs used in the application ==============================================
#if 0
void Q_SysTick_Handler(void); // prototype
void Q_SysTick_Handler(void) {
    QP::QTimeEvt::TICK_X(0U, nullptr); // process time events at rate 0
    QV_ARM_ERRATUM_838869();
}
#endif

} // extern "C"

//============================================================================
namespace BSP {

void init() {
#ifdef NO_HAL
	// Configure the MPU to prevent NULL-pointer dereferencing ...
    MPU->RBAR = 0x0U                          // base address (NULL)
                | MPU_RBAR_VALID_Msk          // valid region
                | (MPU_RBAR_REGION_Msk & 7U); // region #7
    MPU->RASR = (7U << MPU_RASR_SIZE_Pos)     // 2^(7+1) region
                | (0x0U << MPU_RASR_AP_Pos)   // no-access region
                | MPU_RASR_ENABLE_Msk;        // region enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk       // enable background region
                | MPU_CTRL_ENABLE_Msk;        // enable the MPU
    __ISB();
    __DSB();
#endif
    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

#ifdef NO_HAL
    // enable GPIOA clock port for the LED LD4
    RCC->IOPENR |= (1U << 0U);

    // set all used GPIOA pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~(3U << 2U*LD4_PIN);
    GPIOA->MODER   |=  (1U << 2U*LD4_PIN);
    GPIOA->OTYPER  &= ~(1U <<    LD4_PIN);
    GPIOA->OSPEEDR &= ~(3U << 2U*LD4_PIN);
    GPIOA->OSPEEDR |=  (1U << 2U*LD4_PIN);
    GPIOA->PUPDR   &= ~(3U << 2U*LD4_PIN);

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);
#endif
}
//............................................................................
void start() {
    // initialize event pools
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[20];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // instantiate and start AOs/threads...

    consoleDisplay("BSP: starting blinky\r\n");
    static QP::QEvtPtr blinkyQueueSto[20];
    APP::AO_Blinky->start(
        1U,                         // QP prio. of the AO
        blinkyQueueSto,              // event queue storage
        Q_DIM(blinkyQueueSto),       // queue length [events]
        nullptr, 0U,                 // no stack storage
        nullptr);                    // no initialization param
}

#ifdef SHOW_BLINK
//static char s_outBuf[80];
#endif

//............................................................................
void ledOn() {
#ifdef SHOW_BLINK
	consoleDisplay("BSP:: turning led2 on\r\n");
#endif
    //GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    g_multiLed.SetLed(g_multiLed.MAX_LEDS, 1);
}
//............................................................................
void ledOff() {
#ifdef SHOW_BLINK
	consoleDisplay("BSP:: turning led2 off\r\n");
#endif
	//GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LED off
    //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    g_multiLed.SetLed(g_multiLed.MAX_LEDS, 0);
}
//............................................................................
void terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

} // namespace BSP

//============================================================================
namespace QP {
#if 0
// QF callbacks...
void QF::onStartup() {
    // set up the SysTick timer to fire at BSP::TICKS_PER_SEC*2 rate, 2ms
    SysTick_Config((SystemCoreClock / TICKS_PER_SEC)*2000);

    // assign all priority bits for preemption-prio. and none to sub-prio.
    // NOTE: this might have been changed by STM32Cube.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
}
//............................................................................
void QF::onCleanup() {
}
#endif
//............................................................................
void QV::onIdle() {

    // toggle an LED on and then off (not enough LEDs, see NOTE02)
    //GPIOA->BSRR = (1U << LD4_PIN);         // turn LED[n] on
    //GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LED[n] off
	QF_INT_DISABLE();

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    // !!!CAUTION!!!
    // QV_CPU_SLEEP() contains the WFI instruction, which stops the CPU
    // clock, which unfortunately disables the JTAG port, so the ST-Link
    // debugger can no longer connect to the board. For that reason, the call
    // to QV_CPU_SLEEP() has to be used with CAUTION.
    //
    // NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    // reset the board, then connect with ST-Link Utilities and erase the part.
    // The trick with BOOT(0) is it gets the part to run the System Loader
    // instead of your broken code. When done disconnect BOOT0, and start over.
    //
    //QV_CPU_SLEEP(); // atomically go to sleep and enable interrupts
    QF_INT_ENABLE(); // for now, just enable interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
    // g_sysAppInterrupt is set by events in QP indicating system state
    // has changed and the ready queue must be re-examined for active objects
    // ready to run. Continually disabling and enabling interrupts is
    // very inefficient and interferes with the event loop.
    while ( !QF_getSysAppEvent() ) {}
    QF_clearSysAppEvent();
#endif
}


} // namespace QP

//============================================================================
// NOTE0:
// The QV_onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV_onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE1:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call any other QF/QV services.
// These ISRs are "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call ANY QF/QV services. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
