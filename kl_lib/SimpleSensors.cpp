/*
 * SimpleSensors.cpp
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: Kreyl
 */

#include "SimpleSensors.h"

#if SIMPLESENSORS_ENABLED

SimpleSensors_t PinSensors;

// ==== Sensors Thread ====
static THD_WORKING_AREA(waPinSnsThread, 64);
__attribute__((noreturn))
static void SensorsThread(void *arg) {
    chRegSetThreadName("PinSensors");
    PinSensors.ITask();
}

void SimpleSensors_t::Init() {
    // Init pins
    for(uint32_t i=0; i < PIN_SNS_CNT; i++) {
        PinSns[i].Init();
        States[i] = pssLo;
    }
    // Create and start thread
    chThdCreateStatic(waPinSnsThread, sizeof(waPinSnsThread), (tprio_t)90, (tfunc_t)SensorsThread, NULL);
}

void PrintStackSz() {
    thread_t *PThd = chThdGetSelfX();
    port_intctx *r13 = (port_intctx *)__get_PSP();
    int32_t StkSz = PThd->p_stklimit - (stkalign_t *)(r13-1);
    Uart.PrintfI("StackSz=%d\r", StkSz);
}


#define DSZ     sizeof(waPinSnsThread)

__attribute__((noreturn))
void SimpleSensors_t::ITask() {
//    PrintStackSz();
//    Uart.Printf("DSZ=%u\r", DSZ);
//    uint32_t dCnt = 108;
    while(true) {
//        if(dCnt++ >= 99) {
//            dCnt = 0;
//            PrintStackSz();
////            Uart.PrintfNow("%A\r", waPinSnsThread, DSZ, ' ');
//        }

        chThdSleepMilliseconds(SNS_POLL_PERIOD_MS);
        ftVoidPSnsStLen PostProcessor = PinSns[0].Postprocessor;
        uint32_t GroupLen = 0;
        PinSnsState_t *PStates = &States[0];
        // ==== Iterate pins ====
        uint32_t i=0;
        while(i < PIN_SNS_CNT) {
            // Check pin
            if(PinSns[i].IsHi()) {
                if(States[i] == pssLo or States[i] == pssFalling) States[i] = pssRising;
                else States[i] = pssHi;
            }
            else { // is low
                if(States[i] == pssHi or States[i] == pssRising) States[i] = pssFalling;
                else States[i] = pssLo;
            }
            GroupLen++;
            // Call postprocessor if this was last pin in group (or last at all)
            i++;
            if((i >= PIN_SNS_CNT) or (PinSns[i].Postprocessor != PostProcessor)) {
                if(PostProcessor != nullptr) PostProcessor(PStates, GroupLen);
                // Prepare for next group
                PostProcessor = PinSns[i].Postprocessor;
                GroupLen = 0;
                PStates = &States[i];
            }
        } // while i
    } // while true
}

#endif
