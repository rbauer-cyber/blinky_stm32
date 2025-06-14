//$file${.::blinky.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: blinky.qm
// File:  ${.::blinky.cpp}
//
// This code has been generated by QM 7.0.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// Copyright (c) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                 ____________________________________
//                /                                   /
//               /    GGGGGGG    PPPPPPPP   LL       /
//              /   GG     GG   PP     PP  LL       /
//             /   GG          PP     PP  LL       /
//            /   GG   GGGGG  PPPPPPPP   LL       /
//           /   GG      GG  PP         LL       /
//          /     GGGGGGG   PP         LLLLLLL  /
//         /___________________________________/
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open-source software licensed under the GNU
// General Public License (GPL) as published by the Free Software Foundation
// (see <https://www.gnu.org/licenses>).
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
//
//$endhead${.::blinky.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include <iostream>
#include "main.h"
#include "console.h"
#include "multiLed.hpp"
#include "measure.hpp"

using namespace std;
using namespace QP;

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

extern uint16_t g_appReady;

enum { BSP_TICKS_PER_SEC = 2000 };

static uint8_t s_ledIndex = 0;

void BSP_ledOff(void) {
    //consoleDisplay("LED OFF\r\n");
    BSP_SetLed(CMultiLed::MAX_LEDS, 0);
    BSP_SetLed(s_ledIndex++, 0);
    s_ledIndex %= CMultiLed::MAX_LEDS;
}
void BSP_ledOn(void) {
    //consoleDisplay("LED ON\r\n");
    BSP_SetLed(s_ledIndex, 1);
}

static uint8_t s_clockReady = 0;

void Q_SysTick_Handler(void) {
    if ( s_clockReady )
    {
        QP::QTimeEvt::TICK_X(0U, &l_SysTick_Handler); // time events at rate 0
        QV_ARM_ERRATUM_838869();
    }
}

namespace QP {
namespace QF {
void onStartup(void)
{
    if ( s_clockReady == 0 )
    {
        SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
        s_clockReady = 1;
    }
}

void onCleanup(void) {}
void onClockTick(void) {
    QTimeEvt::TICK_X(0U, nullptr);  // QTimeEvt clock tick processing
}
} // QF
} // QP

enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG,
    MAX_SIG
};

//=============== ask QM to declare the Blinky class ==================
//$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace APP {

//${AOs::Blinky} .............................................................
class Blinky : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    static Blinky inst;
    CMeasure m_measure;

public:
    Blinky();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
}; // class Blinky

} // namespace APP
//$enddecl${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpcpp version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::AO_Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace APP {

//${AOs::AO_Blinky} ..........................................................
QP::QActive * const AO_Blinky = &Blinky::inst;

} // namespace APP
//$enddef${AOs::AO_Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

int bspMain() {
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[20];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QF::init(); // initialize the framework

    //consoleDisplay("blinky starting\r\n");
    QF::onStartup();

    static QEvtPtr blinky_queueSto[10];
    APP::AO_Blinky->start(1U, // priority
                     blinky_queueSto, Q_DIM(blinky_queueSto),
                     nullptr, 0U); // no stack

    g_appReady = 1;

    return QF::run(); // run the QF application
}

//================ ask QM to define the Blinky class ==================
//$define${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace APP {

//${AOs::Blinky} .............................................................
Blinky Blinky::inst;

//${AOs::Blinky::Blinky} .....................................................
Blinky::Blinky()
: QActive(Q_STATE_CAST(&Blinky::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//${AOs::Blinky::SM} .........................................................
Q_STATE_DEF(Blinky, initial) {
    //${AOs::Blinky::SM::initial}
    //m_timeEvt.armX(BSP_TICKS_PER_SEC/2, 0);
    (void)e; // unused parameter
    m_measure.Initialize();
    m_measure.Run();
    m_timeEvt.armX(1000, 1000);
    return tran(&off);
}

//${AOs::Blinky::SM::off} ....................................................
Q_STATE_DEF(Blinky, off) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::off}
        case Q_ENTRY_SIG: {
            //m_intervalStartTime = getMicros();
            m_measure.Start();
            //m_timeEvt.armX(1000, 0U);
            BSP_ledOff();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Blinky::SM::off::TIMEOUT}
        case TIMEOUT_SIG: {
            m_measure.UpdateElapsedTime();
            m_measure.DisplayElapsedTimeDelta();
            status_ = tran(&on);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Blinky::SM::on} .....................................................
Q_STATE_DEF(Blinky, on) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::on}
        case Q_ENTRY_SIG: {
            //m_intervalStartTime = getMicros();
            m_measure.Start();
            //m_timeEvt.armX(1000, 0U);
            BSP_ledOn();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Blinky::SM::on::TIMEOUT}
        case TIMEOUT_SIG: {
            m_measure.UpdateElapsedTime();
            m_measure.DisplayElapsedTimeDelta();
            status_ = tran(&off);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

} // namespace APP
//$enddef${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
