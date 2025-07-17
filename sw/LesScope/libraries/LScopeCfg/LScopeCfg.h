/*
 * @file    LScopeCfg.h
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   declaration-file used in LesScope-project.
 * @version 1.1
 * @date    2025-07-15
 * @copyright Copyright (c) 2025
 *
 *  This file is part of LesScope.
 *
 *  LesScope is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LesScope is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with LesScope.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef _LSCOPECFG_H_
#define _LSCOPECFG_H_

/* Enable c-linkage */
#if defined (__cplusplus)
  extern "C" {
#endif

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/************** ENCODER-Pins *************************/
#define ENCODER_A      2
#define ENCODER_B      4
#define ENCODER_CLICK 12

/*************** OLED Display **************************
 * with SPI Interface
 ****************************/
#define OLED_SPI_DC     8
#define OLED_SPI_RESET  9
#define OLED_SPI_CS    10

#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT  64 // OLED display height, in pixels

#define SET_AMP_LEVEL_1  1
#define SET_AMP_LEVEL_2  2
#define SET_AMP_LEVEL_3  3
#define SET_AMP_LEVEL_4  4
#define SET_OFF          false
#define SET_ON           true
#define SET_TRIG_OFF     0
#define SET_TRIG_AUTO_P  1
#define SET_TRIG_AUTO_N  2
#define SET_TRIG_NORM_P  3
#define SET_TRIG_NORM_N  4
#define SET_TRIG_LEVEL_INTERN  0
#define SET_TRIG_LEVEL_EXTERN  1

//menu option values
#define SET_OPT_SINGLE       1
#define SET_OPT_DUAL         2
#define SET_OPT_DUAL_PLUGGED 3
#define SET_OPT_FREQU        4
#define SET_OPT_TUNING       5

#define TRIGGER_TIMEOUT_VALUE_MSEC 2000

//TIMERSCALE values
#define TIMER2_SAMPLE   100 //50usec sample-time
#define TIMER2_50USEC     0
#define TIMER2_01MSEC     1
#define TIMER2_02MSEC     3
#define TIMER2_05MSEC     9
#define TIMER2_1MSEC     19
#define TIMER2_2MSEC     39
#define TIMER2_5MSEC     99
#define TIMER2_10MSEC   199
#define TIMER2_20MSEC   399
#define TIMER2_50MSEC   999
#define TIMER2_100MSEC 1999

//sample-data memory-size
#define SAMPLE_DATA_SIZE  128

//////////////////////////////////////////
//sw-revision of LesScope
#define STR_MENU_REVISION     "0.2"
//////////////////////////////////////////

// value-check for range
#define range(checkit,low,high) ((checkit)<(low)?(low):((checkit)>(high)?(high):(checkit)))

typedef struct channel_val {
  uint8_t status;    //0:= channel off, 1:= on
  uint8_t amplifier;
  uint8_t time;
  int8_t  offset;
  uint8_t trigger_mode;
  uint8_t trigger_level;
  uint8_t option;
  bool     sample_draw;
  bool     sample_start;
} channel_val_t;

//typedef enum channel_nr_e {
typedef enum channel_nr_e {
  eChannel_nr1 = 0,
  eChannel_nr2
} channel_nr_t;

#define CHANNEL_NUMBERS    (uint8_t)2
typedef struct cfg {
  channel_val_t chan[CHANNEL_NUMBERS];
  channel_nr_t  selected_channel;
} cfg_t;

// EEPROM Addresses
#define ADDR_CHECKSUM            0
#define ADDR_CFG_DATA_BASE       2

// global used config.data for exchange between LScopeMenu and LSsample
extern cfg_t g_cfg;

#if defined (__cplusplus)
} //extern "C"
#endif

#endif //end _LSCOPECFG_H_
