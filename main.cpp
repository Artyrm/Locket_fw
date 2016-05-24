/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
 *      Author: g.kruglov
 */

#include "main.h"
#include "SimpleSensors.h"
#include "buttons.h"
#include "board.h"
#include "led.h"
#include "vibro.h"
#include "beeper.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "pill_mgr.h"

App_t App;
static TmrKL_t TmrCheckPill {MS2ST(999), EVT_PILL_CHECK, tktPeriodic};
//static TmrKL_t TmrOff      {MS2ST(254), EVT_OFF, tktOneShot};
//static PinInput_t Btn {BTN_PIN};

//Vibro_t Vibro {VIBRO_PIN};
Beeper_t Beeper {BEEPER_PIN};
LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
//    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();
    App.InitThread();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN);
    Uart.Printf("\r%S %S\r", APP_NAME, BUILD_TIME);
//    if(Sleep::WasInStandby()) {
//        Uart.Printf("WasStandby\r");
//        Sleep::ClearStandbyFlag();
//    }
    Clk.PrintFreqs();

    i2c1.Init();
    PillMgr.Init();

//    uint8_t addr=0;
//    uint8_t Buf[16];
//    i2c1.WriteRead(0x50, &addr, 1, Buf, 16);
//    Uart.Printf("%A\r", Buf, 16, ' ');

    Led.Init();
//    Vibro.Init();
//    Vibro.StartSequence(vsqBrr);
//    PillMgr.Init();

    Beeper.Init();
//    Beeper.StartSequence(BeepPillOk);

//    Btn.Init();
//    Radio.Init();
//    TmrCheckBtn.InitAndStart(chThdGetSelfX());
//    TmrOff.InitAndStart(chThdGetSelfX());
    TmrCheckPill.InitAndStart(chThdGetSelfX());

//    PinSensors.Init();
//    if(Radio.Init() != OK) {
//        Led.StartSequence(lsqFailure);
//        chThdSleepMilliseconds(2700);
//    }
//    else
//    Led.StartSequence(lsqOn);

    // Main cycle
    App.ITask();
}

__noreturn
void App_t::ITask() {
    while(true) {
        __unused eventmask_t Evt = chEvtWaitAny(ALL_EVENTS);
#if BTN_REQUIRED // ==== Button ====
        if(Evt & EVT_BUTTONS) {
            if(Btn.IsHi() and !WasHi) {    // Pressed
                WasHi = true;
                TmrOff.Stop();
                Beeper.StartSequence(bsqOn);
                Led.SetColor(clGreen);
            }
            else if(!Btn.IsHi() and WasHi) {
                WasHi = false;
                Beeper.Off();
                Led.SetColor(clBlack);
                TmrOff.Start();
            }
        } // EVTMSK_BTN_PRESS
#endif

#if 1 // ==== Pill ====
        if(Evt & EVT_PILL_CHECK) {
            PillMgr.Check();
            switch(PillMgr.State) {
                case pillJustConnected: Uart.Printf("Pill Conn\r"); break;
                case pillJustDisconnected: Uart.Printf("Pill Discon\r"); break;
                default: break;
            }
        }
#endif


//        if(Evt & EVT_OFF) {
////            Uart.Printf("Off\r");
//            chSysLock();
//            Sleep::EnableWakeup1Pin();
//            Sleep::EnterStandby();
//            chSysUnlock();
//        }

#if UART_RX_ENABLED
        if(Evt & EVT_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }
#endif

    } // while true
}

//void ProcessButton(PinSnsState_t *PState, uint32_t Len) {
//    if(*PState == pssRising) {
//        Led.StartSequence(lsqOn);   // LED on
//        Vibro.Stop();
//        App.MustTransmit = true;
//        Uart.Printf("\rTx on");
//        App.TmrTxOff.Stop();    // do not stop tx after repeated btn press
//    }
//    else if(*PState == pssFalling) {
//        Led.StartSequence(lsqOff);
//        App.TmrTxOff.Start();
//    }
//}

#if UART_RX_ENABLED // ======================= Command processing ============================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(OK);
    }

    else PShell->Ack(CMD_UNKNOWN);
}
#endif

