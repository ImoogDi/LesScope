/*
 * @file    rotaryencoder.h
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   declaration-file for 'rotaryencoder.cpp'.
 * @version 1.1
 * @date    2025-07-10
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

/*************************************************************************
**  Device: rotary encoder                                              **
*************************************************************************/


#ifndef _ROTARY_ENCODER_h
#define _ROTARY_ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//rotary encoder include
  // This following optional setting causes Encoder to use more optimized code
  // but give us linking-problems (only 1 instance alowed)
  //#define  ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#define ENC_DEBOUNCE_DELAY     5  // check encoder and button every x milliseconds
#define ENC_DOUBLECLICKTIME  600  // second click within 600ms
#define ENC_HOLDTIME        1250  // long time pressure value 1.25s


/*!
 *
 * name: CRotaryEncoder
 * @brief  Class that handles rotary encoder turns and pushes.
 * @param  none
 * @return none
 *
 */
class CRotaryEncoder : public Encoder
{
  public:
    typedef enum Button_e {
      Open = 0,
      HoldOn,
      Released,
      Pushed,
      DoubleClicked
    } Button_t;
  public:
    CRotaryEncoder(uint8_t port_ENC_A, uint8_t port_ENC_B, uint8_t port_ENC_CLICK = 99) ;
    void begin(void);
    bool update();
    Button_t getButtonState(void);
    bool up(void)  { bool rtn_value=_upstate; _upstate=false; return rtn_value; }
    bool down(void){ bool rtn_value=_downstate; _downstate=false; return rtn_value; }

  private:
    uint8_t _port_ENC_PUSH;
    unsigned long _sampleTime;
    int32_t _oldEncPosition{-999};
    int32_t _newEncPosition{-999};
    bool _button_pushed{false};
    bool _button_pushed_prev{false};
    bool _upstate{false};
    bool _downstate{false};
    int     _keyDownTicks;
    uint8_t _doubleClickTicks;
    volatile Button_t _button;
};

#endif

