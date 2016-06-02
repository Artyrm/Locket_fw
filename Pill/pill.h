/*
 * pill.h
 *
 *  Created on: 22 ��� 2014 �.
 *      Author: g.kruglov
 */

#pragma once

#include "color.h"

enum PillType_t {
    ptVitamin = 101,
    ptCure = 102,
    ptPanacea = 103,
    ptEnergetic = 104,
    ptMaster = 105,
};
#define PILL_TYPE_CNT   5

//struct PillColor_t {
//    PillType_t Type;
//    Color_t Color;
//};
//
//const PillColor_t PillColor[PILL_TYPE_CNT] = {
//        { ptVitamin, clGreen },
//        { ptCure, clYellow },
//        { ptPanacea, clWhite },
//        { ptEnergetic, clBlue },
//        { ptMaster, clRed }
//};

__unused
static const Color_t GetPillColor(PillType_t PillType) {
//    uint32_t i = (uint32_t)PillType - 101;
    switch(PillType) {
        case ptVitamin:   return clGreen; break;
        case ptCure:      return clYellow; break;
        case ptPanacea:   return clWhite; break;
        case ptEnergetic: return clBlue; break;
        case ptMaster:    return clRed; break;
    }
    return clBlack;
}

struct Pill_t {
    union { // Type
        int32_t TypeInt32;      // offset 0
        PillType_t Type;        // offset 0
    };
    // Contains dose value after pill application
    int32_t ChargeCnt;          // offset 4
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)
#define PILL_SZ32   (sizeof(Pill_t) / sizeof(int32_t))

//// Inner Pill addresses
//#define PILL_TYPE_ADDR      0
//#define PILL_CHARGECNT_ADDR 4

//// Data to save in EE and to write to pill
//struct DataToWrite_t {
//    uint32_t Sz32;
//    int32_t Data[PILL_SZ32];
//};
