/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "SimpleSensors.h"
#include "buttons.h"
#include "board.h"
#include "vibro.h"
#include "beeper.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "kl_adc.h"
#include "kl_i2c.h"
#include "pill.h"
#include "pill_mgr.h"

#if 1 // ======================== Variables and defines ========================
App_t App;

// EEAddresses
#define EE_ADDR_DEVICE_ID       0
#define EE_ADDR_HEALTH_STATE    8

static const PinInputSetup_t DipSwPin[DIP_SW_CNT] = { DIP_SW6, DIP_SW5, DIP_SW4, DIP_SW3, DIP_SW2, DIP_SW1 };
static uint8_t GetDipSwitch();
static void ReadAndSetupMode();
static uint8_t ISetID(int32_t NewID);
Eeprom_t EE;
void ReadIDfromEE();

// ==== Periphery ====
Vibro_t Vibro {VIBRO_PIN};
#if BEEPER_ENABLED
Beeper_t Beeper {BEEPER_PIN};
#endif

LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };

// ==== Timers ====
static TmrKL_t TmrEverySecond {MS2ST(144), EVT_EVERY_SECOND, tktPeriodic};
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
//    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();
    App.InitThread();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN);
    ReadIDfromEE();
    Uart.Printf("\r%S %S; ID=%u\r", APP_NAME, BUILD_TIME, App.ID);
    Clk.PrintFreqs();

    Led.Init();
    Vibro.Init();
    Vibro.StartOrRestart(vsqBrrBrr);
#if BEEPER_ENABLED // === Beeper ===
    Beeper.Init();
    Beeper.StartOrRestart(bsqBeepBeep);
    chThdSleepMilliseconds(702);    // Let it complete the show
#endif
#if BTN_ENABLED
    PinSensors.Init();
#endif
//    Adc.Init();

#if PILL_ENABLED // === Pill ===
    i2c1.Init();
    PillMgr.Init();
#endif

    // ==== Time and timers ====
    TmrEverySecond.InitAndStart();

    // ==== Radio ====
    if(Radio.Init() == OK) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(1008);

    // Main cycle
    App.ITask();
}

__noreturn
void App_t::ITask() {
    while(true) {
        __unused eventmask_t Evt = chEvtWaitAny(ALL_EVENTS);
        if(Evt & EVT_EVERY_SECOND) {
            ReadAndSetupMode();
        }

#if BTN_ENABLED
        if(Evt & EVT_BUTTONS) {
            Uart.Printf("Btn\r");
            Vibro.StartOrRestart(vsqBrr);
            Beeper.StartOrRestart(bsqBeepBeep);
        }
#endif

        if(Evt & EVT_RX) {
        }

        if(Evt & EVT_RXCHECK) {
        }


#if PILL_ENABLED // ==== Pill ====
        if(Evt & EVT_PILL_CONNECTED) {
            Uart.Printf("Pill: %d\r", PillMgr.Pill.TypeInt32);
        }

        if(Evt & EVT_PILL_DISCONNECTED) {
            Uart.Printf("Pill Discon\r");
        }
#endif

#if 0 // ==== Vibro seq end ====
        if(Evt & EVT_VIBRO_END) {
            // Restart vibration (or start new one) if needed
            if(pVibroSeqToPerform != nullptr) Vibro.StartSequence(pVibroSeqToPerform);
        }
#endif
#if 0 // ==== Led sequence end ====
        if(Evt & EVT_LED_SEQ_END) {
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
} // App_t::ITask()

__unused
void ReadAndSetupMode() {
    static uint8_t OldDipSettings = 0xFF;
    uint8_t b = GetDipSwitch();
    if(b == OldDipSettings) return;
    // Something has changed
    OldDipSettings = b;
    Uart.Printf("Dip: %02X\r", b);
}


#if UART_RX_ENABLED // ================= Command processing ====================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(OK);
    }

    else if(PCmd->NameIs("GetID")) PShell->Reply("ID", App.ID);

    else if(PCmd->NameIs("SetID")) {
        if(PCmd->GetNextInt32(&dw32) != OK) { PShell->Ack(CMD_ERROR); return; }
        uint8_t r = ISetID(dw32);
        PShell->Ack(r);
    }

#if PILL_ENABLED // ==== Pills ====
    else if(PCmd->NameIs("PillRead32")) {
        int32_t Cnt = 0;
        if(PCmd->GetNextInt32(&Cnt) != OK) { PShell->Ack(CMD_ERROR); return; }
        uint8_t MemAddr = 0, b = OK;
        PShell->Printf("#PillData32 ");
        for(int32_t i=0; i<Cnt; i++) {
            b = PillMgr.Read(MemAddr, &dw32, 4);
            if(b != OK) break;
            PShell->Printf("%d ", dw32);
            MemAddr += 4;
        }
        Uart.Printf("\r\n");
        PShell->Ack(b);
    }

    else if(PCmd->NameIs("PillWrite32")) {
        uint8_t b = CMD_ERROR;
        uint8_t MemAddr = 0;
        // Iterate data
        while(true) {
            if(PCmd->GetNextInt32(&dw32) != OK) break;
//            Uart.Printf("%X ", Data);
            b = PillMgr.Write(MemAddr, &dw32, 4);
            if(b != OK) break;
            MemAddr += 4;
        } // while
        Uart.Ack(b);
    }
#endif

    else PShell->Ack(CMD_UNKNOWN);
}
#endif

#if 1 // =========================== ID management =============================
void ReadIDfromEE() {
    App.ID = EE.Read32(EE_ADDR_DEVICE_ID);  // Read device ID
    if(App.ID < ID_MIN or App.ID > ID_MAX) {
        Uart.Printf("\rUsing default ID\r");
        App.ID = ID_DEFAULT;
    }
}

uint8_t ISetID(int32_t NewID) {
    if(NewID < ID_MIN or NewID > ID_MAX) return FAILURE;
    uint8_t rslt = EE.Write32(EE_ADDR_DEVICE_ID, NewID);
    if(rslt == OK) {
        App.ID = NewID;
        Uart.Printf("New ID: %u\r", App.ID);
        return OK;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return FAILURE;
    }
}
#endif

// ====== DIP switch ======
uint8_t GetDipSwitch() {
    uint8_t Rslt = 0;
    for(int i=0; i<DIP_SW_CNT; i++) PinSetupInput(DipSwPin[i].PGpio, DipSwPin[i].Pin, DipSwPin[i].PullUpDown);
    for(int i=0; i<DIP_SW_CNT; i++) {
        if(!PinIsHi(DipSwPin[i].PGpio, DipSwPin[i].Pin)) Rslt |= (1 << i);
        PinSetupAnalog(DipSwPin[i].PGpio, DipSwPin[i].Pin);
    }
    return Rslt;
}
