/*
 * pill.h
 *
 *  Created on: Apr 17, 2013
 *      Author: g.kruglov
 */

#pragma once

#include "uart.h"
#include "ch.h"

#define PILL_I2C_STD_ADDR   0x50    // Standard address start of EEPROM - 0x01010aaa
#define PILL_I2C_ADDR       (PILL_I2C_STD_ADDR | 0) // Only Zero addr
#define PILL_START_ADDR     0x00    // Address of data
// Number of bytes to be written simultaneously. IC dependant, see datasheet.
#define PILL_PAGE_SZ        8

enum PillState_t { pillJustConnected, pillJustDisconnected, pillNoChange };

class PillMgr_t {
private:
    i2c_t *i2c;
    const PinOutput_t PillPwr;
    void Standby();
    void Resume();
    bool IsConnectedNow;
public:
    PillState_t State;
    PillMgr_t(i2c_t *pi2c, const PortPinOutput_t PillPwrPin) :
        i2c(pi2c), PillPwr(PillPwrPin), IsConnectedNow(false), State(pillNoChange) {}
    void Init();
    void Check();
    uint8_t Read (uint8_t MemAddr, void *Ptr, uint32_t Length);
    uint8_t Write(uint8_t MemAddr, void *Ptr, uint32_t Length);
};

extern PillMgr_t PillMgr;
