/*
 * @file    rotaryencoder.cpp
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   returns current state of encoder-data and encoder-switch.
 *           update() has to be called cyclic.
 * @version 1.0
 * @date    2025-19-03
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
**************************************************************************/

#include "rotaryencoder.h"

/*!
 *
 * name: class::CRotaryEncoder
 * @param : uint8_t port_ENC_A, port for encoder-pin A
 *          uint8_t port_ENC_B, port for encoder-pin B
 *          uint8_t port_ENC_PUSH, port for push-sw
 * @return: none
 *
 */
CRotaryEncoder::CRotaryEncoder(uint8_t port_ENC_A, uint8_t port_ENC_B, uint8_t port_ENC_PUSH)
  : Encoder(port_ENC_A, port_ENC_B)
{
  _port_ENC_PUSH = port_ENC_PUSH;
  _keyDownTicks = 0;
  _button_pushed = false;
  _button_pushed_prev = false;

  _button=CRotaryEncoder::Open;
  _doubleClickTicks=0;
}

//************************************************************************
/*!
 *
 * name: begin()
 * @brief  has to be called before using.
 * @param: void
 * @return:void
 *
 */
void CRotaryEncoder::begin(void)
{
  // set up Click-port only if it is required
  if (_port_ENC_PUSH != 99) {
    pinMode(_port_ENC_PUSH, INPUT_PULLUP);
  }
  _sampleTime = millis();
}

/*!
 *
 * name: update()
 * @brief  has to be called cyclic in main-loop.
 * @param: void
 * @return:True if new values available, else false
 */
bool CRotaryEncoder::update(void)
{
  bool rtn_value = false;
  // elapsed time > debounceDelay ? -> action
  if(millis() >= (_sampleTime + (unsigned long)ENC_DEBOUNCE_DELAY))
  {
    // Updates sample Time
    _sampleTime = millis();
    // read current encoder-value and check it for up/down to previous
    _newEncPosition = this->read();
    if ( (_newEncPosition-3) > _oldEncPosition) {
      _oldEncPosition = _newEncPosition;
      rtn_value = true;
      _upstate=true;
      _downstate=false;
    }
    if ( (_newEncPosition+3) < _oldEncPosition ) {
      _oldEncPosition = _newEncPosition;
      rtn_value = true;
      _upstate=false;
      _downstate=true;
    }
    // encoder push-switch status handling
    if (_port_ENC_PUSH != 99) {
      // read port, if Low then button:=pushed
      _button_pushed = !(bool)digitalRead(_port_ENC_PUSH);
      if (_button_pushed == _button_pushed_prev) {
        if ( _button_pushed ) {
          _keyDownTicks++;
          if (_keyDownTicks > (ENC_HOLDTIME / ENC_DEBOUNCE_DELAY)) {
            _button = HoldOn;
          }
        } //end if button_pushed

        if ( ! _button_pushed ) {
          if (_keyDownTicks) {
            if (_button == HoldOn) {
              _button = Released;
              _doubleClickTicks = 0;
            } else {
              if (_doubleClickTicks > 1) {
                if (_doubleClickTicks < (ENC_DOUBLECLICKTIME / ENC_DEBOUNCE_DELAY)) {
                  _button = DoubleClicked;
                  _doubleClickTicks = 0;
                }
              } else {
                _doubleClickTicks = (ENC_DOUBLECLICKTIME / ENC_DEBOUNCE_DELAY);
              }
            }
          }
          _keyDownTicks = 0;
        } //end if not button_pushed
      } //end if (_button_pushed == _prev

      if (_doubleClickTicks > 0) {
        _doubleClickTicks--;
        if (--_doubleClickTicks == 0) {
          _button = Pushed;
        }
      }
      _button_pushed_prev = _button_pushed;
    }  // end button-check required
  } //end if(millis()...
  if (rtn_value == false) {
    _upstate=false;
    _downstate=false;
  }
  return rtn_value;
}

/*!
 *
 * name: getButtonState
 * @param  none
 * @return Button_t  current button-state.
 *
 */
CRotaryEncoder::Button_t CRotaryEncoder::getButtonState(void)
{
  CRotaryEncoder::Button_t ret = _button;
  if (_button != CRotaryEncoder::HoldOn) {
    _button = CRotaryEncoder::Open; // reset
  }
  return ret;
}
