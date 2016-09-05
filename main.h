/*
 * main.h
 *
 *  Created on: 15 ����. 2014 �.
 *      Author: g.kruglov
 */

#pragma once

#include "ch.h"
#include "kl_lib.h"
#include "uart.h"
#include "evt_mask.h"
#include "board.h"
#include "pill.h"

// ==== Constants and default values ====
#define ID_MIN                  1
#define ID_MAX                  21
#define ID_DEFAULT              ID_MIN

enum Mode_t {modeNone, modeDetectorTx, modeDetectorRx, modeBinding};

class App_t {
private:
    thread_t *PThread;
public:
    Mode_t Mode = modeNone;
    uint8_t ID;
    // Eternal methods
    void InitThread() { PThread = chThdGetSelfX(); }
    void SignalEvt(eventmask_t Evt) {
        chSysLock();
        chEvtSignalI(PThread, Evt);
        chSysUnlock();
    }
    void SignalEvtI(eventmask_t Evt) { chEvtSignalI(PThread, Evt); }
    void OnCmd(Shell_t *PShell);
    // Inner use
    void ITask();
};

extern App_t App;
