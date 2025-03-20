/*
 * @file    LesScope.ino
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   setup- and loop-part of project: LesScope.
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
 */
/*
 ****************************************************************************
 * LesScope is a Dual Channel Scope designed as eurorack-modul used
 *  in modular synthesizers.
 * Only the +12Volt/45mA from standard bus-connection is used.
 * This connection is diode-protected against misleading.
 *
 * Features:
 *  - Arduino Nano V3.X as dual-channel sampler and controller.
 *  - 1.3inch OLED display with SPI interface.
 *  - rotary encoder with push-button for data-selection and -change.
 *  - standard patchcabel can be used on input-jacks.
 *  - Any input is interconnected to a second jack per channel.
 *  - AC/DC coupling selectable for both channels on two switches.
 *  - extra input for Trigger-signal (normalized with Channel1-signal).
 *
 ****************************************************************************
 * Rotaryencoder/Button:
 *  encoder turn  left/right: menu activating, selection and changing data.
 *  button short  press     : mark and store selected data.
 *  button double press     : back to previous menu (only if menu is active).
 *  button long   press     : not currently used
 *
 * The signal-draw is automatic active again after 5 seconds without any
 *  encoder and/or button action.
 ****************************************************************************
 * Required libraries:
 *  - Frequency measure: http://www.pjrc.com/teensy/td_libs_FreqMeasure.html
 *  - Encoder          : http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * Further infos and links:
 *  - mzuelch Display  :
 *      https://github.com/mzuelch/CATs-Eurosynth/tree/main/Modules/HAGIWO/Display
 *
 ****************************************************************************
 */

#include "LScopeCfg.h"
#include "LScopeMenu.h"
#include "LScopeSample.h"
#include "LScopeSetHW.h"

CMenu LSMenu(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_SPI_DC,
                        OLED_SPI_RESET, OLED_SPI_CS);

void setup() {
  //serial deactivated, needs ca. 1kByte Flash and 100Byte ram.
//  Serial.begin(9600);

  //hardware init
  hw_init();

  //display setting
  LSMenu.begin(0, true);
  // uncomment //#define SH110X_NO_SPLASH
  //   in file:Adafruit_SH110X/Adafruit_SH110X.h
  // if no Adafruit-logo is wanted

  //init signal-sampling
  sample_init();
  LSMenu.updateMenu();
};

void loop() {
  CRotaryEncoder::Button_t bstate = LSMenu.rotaryencoder.getButtonState();

  LSMenu.CheckMenuTimeout();
  LSMenu.Drawupdate();

  if (LSMenu.rotaryencoder.update()) {
    LSMenu.updateSelection();
  }
  if (bstate != CRotaryEncoder::Open)
  {
    switch (bstate)
    {
      case CRotaryEncoder::HoldOn:
        // not yet used
        break;
      case CRotaryEncoder::Released:
        break;
      case CRotaryEncoder::Pushed:
        LSMenu.updateMenu();
        break;
      case CRotaryEncoder::DoubleClicked:
        LSMenu.SetPrevMenu();
        break;
    }
  }
};
