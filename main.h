/*
 * main.h
 *
 *  Created on: 6 ����� 2018 �.
 *      Author: Kreyl
 */

#pragma once

#define ID_MIN                  1
#define ID_MAX                  15
#define ID_DEFAULT              ID_MIN
extern int32_t ID;

enum AppMode_t {appmRx, appmTx};

extern AppMode_t AppMode;
