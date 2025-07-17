/*
 * @file    LScopeMenu.cpp
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   Class: 'CMenu' used for all menu-driven data-handling.
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

#include <avr/pgmspace.h>
#include <avr/io.h>//for fast PWM
#include <SPI.h>
#include <Wire.h>
#include "LScopeMenu.h"
#include "LScopeSample.h"
#include "LScopeSetHW.h"
#include <LSFreqMeasure.h>

// global used config.data for exchange between LesScope and Cfg
cfg_t g_cfg;

//reference frequency-table
//  see URL: https://en.wikipedia.org/wiki/MIDI_tuning_standard
//     and : https://musicinformationretrieval.com/midi_conversion_table.html
//  array is stored in program-memory (flash) to save RAM
//
const int g_ref_frequ10[] PROGMEM = {
  2589, // ~ 1% below below C
  2616, // note C
  2772, // note Cis
  2937, // note D
  3111, // note Dis
  3296, // note E
  3492, // note F
  3700, // note Fis
  3920, // note G
  4153, // note Gis
  4400, // note A
  4662, // note Ais
  4939, // note B
  5001  // ~ 1% above abow B
};

/*!
 *
 * name: CMenu::CMenu     constructor
 * @param  uint16_t w     disply-width
 * @param  uint16_t h     disply-height
 * @param  SPIClass *spi  pointer to spi-object
 * @param  uint16_t dc_pin  spi datacontrol pin
 * @param  uint16_t rst_pin spi reset pin
 * @param  uint16_t cs_pin  spi chip-select pin
 * @return none
 *
 */
CMenu::CMenu(uint16_t w, uint16_t h, SPIClass *spi, int16_t dc_pin, int16_t rst_pin, int16_t cs_pin)
            : Adafruit_SH1106G(w, h, spi, dc_pin, rst_pin, cs_pin) , rotaryencoder(ENCODER_A, ENCODER_B, ENCODER_CLICK)
{
  _menutimer.expired = false;
  _menutimer.Timeout  = millis();

  init_cfg();
  _menuctrl.menu_updated = true;
  _menuctrl.rowindex = 1;
  _menuctrl.cursor_x=52;
  _menuctrl.cursor_y=_index2_ypixel(_menuctrl.rowindex);
  _menuctrl.mark_on = false;
  _menuctrl.save_yes = false;
  this->_x_border = this->width() - 1;
  this->_y_border = this->height()- 1;
  CMenu::_draw_channels_running = false;
}

CMenu::~CMenu(void) {}


/*!
 *
 * name: begin
 * @brief  has to be called at first.
 * @param  int addr
 * @param  bool reset
 * @return none
 *
 */
void CMenu::begin(int addr, bool reset)
{
  SPI.begin();
  if(! this->Adafruit_SH1106G::begin(addr, reset) ) {
    this->print(F("CMenu::begin() failed"));
    this->display();
    while (1) {};
  }
  clearDisplay();
  setTextSize(0);
  setTextColor(SH110X_WHITE);
  this->rotaryencoder.begin();
  FreqMeasure.begin();
}

/*!
 *
 * name: displayModulName
 * @brief  first display shows name and revision.
 * @param  none
 * @return none
 *
 */
void CMenu::displayModulName(void) {
  //display Module-name
  setTextColor(SH110X_WHITE);
  setTextSize(2);
  setCursor(10, 4);
  println(STR_MENU_LESSCOPE);
  setTextSize(0);
  setCursor(50, 24);
  // print Revision
  println(STR_MENU_REVISION);
  this->setCursor(12, 39);
  println(STR_MENU_MODUL_TYPE);
  this->setCursor(12, 55);
  println(STR_MENU_OWNER);
  display();
}

/*!
 *
 * name: updateMenu
 * @brief  has to be called whenever botton is pushed.
 * @param  none
 * @return none
 *
 */
void CMenu::updateMenu(void) {

  switch (_menu_state)
  {
    case STARTUP:
      _prev_menu = MENU_DEFAULT;
      _menu_state = INITDEFAULTS;
      // No break statement, continue through next case
    case INITDEFAULTS:
      _InitDisplay();
      _prev_menu = INITDEFAULTS;
      _menu_state = MENU_DEFAULT;
      delay(3000L);
      _menutimer.Timeout = millis();
      init_cfg();
      break;
    case MENU_DEFAULT:
      _prev_menu = MENU_DEFAULT;
      if (_menutimer.expired==false) { // Menu is active, choose selection
        _menu_state = SETTINGS;
        _defaultMenu();
      } else {
        _menu_state = DRAW_SAMPLES;
      }
      break;
    case SETTINGS:
      _prev_menu = SETTINGS;
      _menu_state = SELECT_VALUES;
      _menutimer.Timeout = millis();
      if (_menutimer.expired) {
        _menu_state = DRAW_SAMPLES;
      }
      _menuctrl.mark_on = &(_menuctrl.mark_on);
      break;
    case SELECT_VALUES:
      _prev_menu = SELECT_VALUES;
      _menu_state = SETTINGS;
      _menutimer.Timeout = millis();
      if (_menutimer.expired) {
        _menu_state = DRAW_SAMPLES;
      }
      _menuctrl.mark_on = false;
      break;
    case DRAW_SAMPLES:
      _prev_menu = DRAW_SAMPLES;
      _menu_state = MENU_DEFAULT;
      _menutimer.expired=false;
      break;
    case SAVE_REQUEST:
      _prev_menu = SAVE_REQUEST;
      _menu_state = SAVE_SET;
      _menutimer.expired=false;
      _saveMenu();
      break;
    case SAVE_SET:
      _prev_menu = SAVE_SET;
      _menu_state = SAVE_DATA;
      _menutimer.Timeout = millis();
      if (_menutimer.expired) {
        _menu_state = DRAW_SAMPLES;
      }
      break;
    case SAVE_DATA:
      _prev_menu = SAVE_DATA;
      _menu_state = MENU_DEFAULT;
      if (_menutimer.expired) {
        _menu_state = DRAW_SAMPLES;
      }
      break;
    default:
      break;
  }
  updateSelection(); // Refresh screen
}

/*!
 *
 * name: updateSelection
 * @brief  has to be called whenever encoder is turned or on startup
 * @param  none
 * @return none
 *
 */
void CMenu::updateSelection(void) {
  int newPosition;
  uint8_t select_rowindex;
  bool select_save_yes=_menuctrl.save_yes;

  switch (_menu_state)
  {
    case STARTUP:
    case INITDEFAULTS:
      _InitDisplay();
      delay(3000L);
      // No break statement, continue through next case
    case MENU_DEFAULT:
      _defaultMenu();
      _menutimer.Timeout = millis();
    break;
    case SETTINGS:
      select_rowindex=_menuctrl.rowindex;
      if (this->rotaryencoder.down()) {
        select_rowindex++;
        _menutimer.Timeout = millis();
      }
      if (this->rotaryencoder.up()) {
        select_rowindex--;
        _menutimer.Timeout = millis();
      }
      if (g_cfg.selected_channel == eChannel_nr1) {
        _menuctrl.rowindex = range(select_rowindex,1,7);
      } else {
        _menuctrl.rowindex = range(select_rowindex,1,4);
      }
      _defaultMenu();
      if ( _menuctrl.menu_updated ) {
        //// set parameter for channel1 ////
        //set amplifier
        set_amplifier(eChannel_nr1);
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          //renew counter-values
          set_counter_defaults();
          //set trigger-mode
          set_trigger_mode(eChannel_nr1);
          //set trigger-level
          set_trigger_level();
          //allocate/deallocate memory for channel2 if required
          if ((g_cfg.chan[eChannel_nr1].option == SET_OPT_DUAL) ||
              (g_cfg.chan[eChannel_nr1].option == SET_OPT_DUAL_PLUGGED))
          {
            if(pchannel2 == NULL) {
              pchannel2 = new sample_t;
            }
          } else {
            if(pchannel2 != NULL) {
              delete pchannel2;
              pchannel2 = NULL;
            }
          }
        } //end ATOMIC_BLOCK()

        //// set parameter for channel2 ////
        //set amplifier
        set_amplifier(eChannel_nr2);
        _menuctrl.menu_updated = false;
      }

    break;
    case SELECT_VALUES:
      _defaultMenu();
    break;
    case DRAW_SAMPLES:
      _draw_channels();
    break;
    case SAVE_REQUEST:
      //no break
    case SAVE_SET:
      if (this->rotaryencoder.down()) {
        select_save_yes=false;
        _menutimer.Timeout = millis();
      }
      if (this->rotaryencoder.up()) {
        select_save_yes=true;
        _menutimer.Timeout = millis();
      }
      select_save_yes = range(select_save_yes, false, true);
      _menuctrl.save_yes = select_save_yes;
      _saveMenu(select_save_yes);
    break;
    case SAVE_DATA:
      if(_menuctrl.save_yes) {
        bool write_ok=false;
        //save data to EEPROM
        write_ok=_EEPROM_write_cfg();
        setCursor(10, SAVE_POX_Y+16);
        (write_ok) ? print(STR_OK) : print(STR_FAILED);
        display();
      } else {
        _menutimer.expired = true;
      } //end if(_menuctrl.save_yes)
      _menuctrl.save_yes = false;
    break;
  } //end switch(_menu_state)
  if (_menutimer.expired) {
    _menu_state = DRAW_SAMPLES;
  }
  if (rotaryencoder.update()) {
    switch (_menu_state) {
      case DRAW_SAMPLES :
        _menu_state = MENU_DEFAULT;
        _menutimer.expired=false;
        _menutimer.Timeout= millis();
      break;
      case MENU_DEFAULT :
        _menu_state = SETTINGS;
        _menutimer.expired=false;
        _menutimer.Timeout= millis();
      break;
      default  :
      break;
    }
    updateSelection();
  }
  if (rotaryencoder.getButtonState() == CRotaryEncoder::HoldOn) {
    _menutimer.expired=false;
    _menutimer.Timeout= millis();
    SaveConfigdata();
  }
}

/*!
 *
 * name: CheckMenuTimeout
 * @brief  has to be called cyclic in main-loop.
 * @param  none
 * @return none
 *
 */
void CMenu::CheckMenuTimeout(void)
{
  // If menu-timer expired, show samples
  if ((_menutimer.expired==false) && ((millis() - _menutimer.Timeout) > WAIT4ACTIONS_TIMEOUT)) {
    _menutimer.expired = true;
    _menu_state = DRAW_SAMPLES; // Show samples
    this->clearDisplay();
    this->display();
    updateSelection();    // Refresh screen
  }
  // check for trigger-timeout in DRAW-state.
  //  check it against the value:MENU_TIM_10MS_VALUE.
  //  above that value the timeout is 4 times longer.
  if ((_menu_state == DRAW_SAMPLES) && is_triggertimeout(eChannel_nr1, MENU_TIM_10MS_VALUE)) {
    g_cfg.chan[eChannel_nr1].sample_draw = true;
    _menu_state = DRAW_SAMPLES; // Show samples
    //reset frequency-values
    this->_frequ_meas_value10 = 0L;
    this->_search_frequency = 0;
    //don't clear-display if measurements are running
    if (( g_cfg.chan[eChannel_nr1].option != SET_OPT_FREQU ) &&
        ( g_cfg.chan[eChannel_nr1].option != SET_OPT_TUNING))
    {
      this->clearDisplay();
      this->display();
    }
    updateSelection();    // Refresh screen
  }
}

/*!
 *
 * name: Drawupdate
 * @brief  has to be called cyclic in main-loop.
 * @param  none
 * @return none
 *
 */
void CMenu::Drawupdate(void) {
  if (this->_menu_state == DRAW_SAMPLES) {
    if (g_cfg.chan[eChannel_nr1].trigger_mode == SET_OFF) {
        updateSelection();    // Refresh screen
    } else {
      //draw if required
      if (g_cfg.chan[eChannel_nr1].sample_draw) {
        updateSelection();    // Refresh screen
      }
    }
  }
}

/*!
 *
 * name: init_cfg
 * @brief  initialisation of global configuration.
 * @param  none
 * @return none
 *
 */
void CMenu::init_cfg(void)
{
  //channel1 config
  g_cfg.selected_channel = eChannel_nr1;
  g_cfg.chan[eChannel_nr1].status = SET_ON;
  g_cfg.chan[eChannel_nr1].amplifier = SET_AMP_LEVEL_1;
  g_cfg.chan[eChannel_nr1].time = MENU_TIM_05MS_VALUE;
  g_cfg.chan[eChannel_nr1].trigger_mode = SET_OFF;
  g_cfg.chan[eChannel_nr1].offset = 0;
  g_cfg.chan[eChannel_nr1].option = SET_OPT_SINGLE;
  g_cfg.chan[eChannel_nr1].trigger_level = SET_TRIG_LEVEL_INTERN;
  g_cfg.chan[eChannel_nr1].sample_draw = true;
  g_cfg.chan[eChannel_nr1].sample_start= true;
  //channel2 config
  g_cfg.chan[eChannel_nr2].status = SET_OFF;
  g_cfg.chan[eChannel_nr2].amplifier = SET_AMP_LEVEL_1;
  g_cfg.chan[eChannel_nr2].time = MENU_TIM_05MS_VALUE;
  g_cfg.chan[eChannel_nr2].offset = 0;
  g_cfg.chan[eChannel_nr2].trigger_mode = SET_OFF;
  g_cfg.chan[eChannel_nr2].option = SET_OFF;
  g_cfg.chan[eChannel_nr2].trigger_level = SET_TRIG_LEVEL_INTERN;
  g_cfg.chan[eChannel_nr2].sample_draw = true;
  g_cfg.chan[eChannel_nr2].sample_start= true;

  if (_IsEEPROM_data_valid()) {
    //get cfg-data from EEPROM
    cfg_t eeprom_data;
    EEPROM.get(ADDR_CFG_DATA_BASE, eeprom_data);
    //store required eeprom-data to global cfg-data
    // channel1 config
    g_cfg.chan[eChannel_nr1].amplifier    = eeprom_data.chan[eChannel_nr1].amplifier;
    g_cfg.chan[eChannel_nr1].time         = eeprom_data.chan[eChannel_nr1].time;
    g_cfg.chan[eChannel_nr1].trigger_mode = eeprom_data.chan[eChannel_nr1].trigger_mode;
    g_cfg.chan[eChannel_nr1].offset       = eeprom_data.chan[eChannel_nr1].offset;
    g_cfg.chan[eChannel_nr1].option       = eeprom_data.chan[eChannel_nr1].option;
    g_cfg.chan[eChannel_nr1].trigger_level= eeprom_data.chan[eChannel_nr1].trigger_level;
    // channel2 config
    g_cfg.chan[eChannel_nr2].amplifier    = eeprom_data.chan[eChannel_nr2].amplifier;
    g_cfg.chan[eChannel_nr2].time         = eeprom_data.chan[eChannel_nr2].time;
    g_cfg.chan[eChannel_nr2].offset       = eeprom_data.chan[eChannel_nr2].offset;
  }
}

void CMenu::_InitDisplay(void)
{
  displayModulName();
}

void CMenu::_print_value_str(const uint8_t index, const channel_nr_t channel_nr, const uint8_t oldvalue=0) {
  int8_t select_value=0;
  bool bchangevalue=false;

  if ((index == _menuctrl.rowindex) && _menuctrl.mark_on) {
    bchangevalue=true;
    select_value = oldvalue;
    this->setTextColor(SH110X_BLACK, SH110X_WHITE);
    if (this->rotaryencoder.down()) {
      select_value--;
      _menutimer.Timeout = millis();
    }
    if (this->rotaryencoder.up()) {
      select_value++;
      _menutimer.Timeout = millis();
    }
  } else {
    bchangevalue=false;
  }

  switch(index) {
    case 1:
      if (bchangevalue) {
        g_cfg.selected_channel=(channel_nr_t)range(select_value, eChannel_nr1, eChannel_nr2);
      }
      this->print(g_cfg.selected_channel+1);
    break;
    case 2:
      if (bchangevalue) {
        if (channel_nr == eChannel_nr1) {
          g_cfg.chan[eChannel_nr1].amplifier = range(select_value, SET_AMP_LEVEL_1, SET_AMP_LEVEL_4);
        } else {
          g_cfg.chan[eChannel_nr2].amplifier = range(select_value, SET_AMP_LEVEL_1, SET_AMP_LEVEL_2);
        }
      }
      this->print(g_cfg.chan[channel_nr].amplifier);
    break;
    case 3:
      if (bchangevalue) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          g_cfg.chan[channel_nr].time = range(select_value, MENU_TIM_50US_VALUE, MENU_TIM_100MS_VALUE);
          //channel2 sampling-time depends on channel1,
          // sampling-time could be faster, but not less
          if ( (g_cfg.chan[eChannel_nr2].time) > g_cfg.chan[eChannel_nr1].time ) {
            g_cfg.chan[eChannel_nr2].time = g_cfg.chan[eChannel_nr1].time;
          }
          set_counter_defaults();
        } //end ATOMIC_BLOCK()
      }
      this->_print_time_str(g_cfg.chan[channel_nr].time);
    break;
    case 4:
      if (bchangevalue) {
        g_cfg.chan[channel_nr].offset = (int8_t)range(select_value, (int8_t)-31, 31);
      }
      this->print(g_cfg.chan[channel_nr].offset);
    break;
    case 5:
      //only on channel1 available
      if (bchangevalue) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          g_cfg.chan[channel_nr].trigger_mode = range(select_value, SET_TRIG_OFF, SET_TRIG_NORM_N);
        }
      }
      this->_print_trigger_str(g_cfg.chan[channel_nr].trigger_mode);
    break;
    case 6:
      //only on channel1 available
      if (channel_nr == eChannel_nr1) {
        if (bchangevalue) {
          g_cfg.chan[eChannel_nr1].trigger_level = range(select_value, 0, 1);
        }
        this->_print_triggerlevel_str(g_cfg.chan[eChannel_nr1].trigger_level);
      }
    break;
    case 7:
      //channel1 option
      if (channel_nr == eChannel_nr1) {
        if (bchangevalue) {
          g_cfg.chan[channel_nr].option = range(select_value, SET_OPT_SINGLE, SET_OPT_TUNING);
        }
      _print_option_str(g_cfg.chan[channel_nr].option);
      }
    break;
    default:
    break;
  }
  if (bchangevalue) {
    _menuctrl.menu_updated = true;
  }
  this->setTextColor(SH110X_WHITE);
}

void CMenu::_print_row_str(const uint8_t index, const channel_nr_t channel_nr) {
  switch(index) {
    case 1:
      setCursor(0, 0);
      this->setTextColor(SH110X_BLACK, SH110X_WHITE);
      this->print(STR_MENU_CHANNEL);
      this->setTextColor(SH110X_WHITE);
      this->_print_value_str(index, channel_nr);
      this->setTextColor(SH110X_BLACK, SH110X_WHITE);
      this->print(F("  Setup    "));
      this->setTextColor(SH110X_WHITE);
    break;
    case 2:
      this->print(STR_MENU_AMP);
      this->_print_value_str(index, channel_nr, g_cfg.chan[channel_nr].amplifier);
    break;
    case 3:
      this->print(STR_MENU_TIME);
      this->_print_value_str(index, channel_nr, g_cfg.chan[channel_nr].time);
    break;
    case 4:
      this->print(STR_MENU_OFFSET);
      this->_print_value_str(index, channel_nr, g_cfg.chan[channel_nr].offset);
    break;
    case 5:
      //trigger select only on channel1
      if(channel_nr == eChannel_nr1) {
        this->print(STR_MENU_TRIGGER);
        this->_print_value_str(index, channel_nr, g_cfg.chan[channel_nr].trigger_mode);
      }
    break;
    case 6:
      //filter select only available on channel1
      if(channel_nr == eChannel_nr1) {
        this->print(STR_MENU_TRG_LEVEL);
        this->_print_value_str(index, channel_nr, g_cfg.chan[eChannel_nr1].trigger_level);
      }
    break;
    case 7:
      //option select only on channel1
      if(channel_nr == eChannel_nr1) {
        this->print(STR_MENU_OPTION);
        this->_print_value_str(index, channel_nr, g_cfg.chan[eChannel_nr1].option);
        switch (g_cfg.chan[eChannel_nr1].option) {
          case SET_OPT_SINGLE:
            g_cfg.chan[eChannel_nr2].status = SET_OFF;
          break;
          case SET_OPT_DUAL:
            g_cfg.chan[eChannel_nr2].status = SET_ON;
          break;
          case SET_OPT_DUAL_PLUGGED:
            // preset second channel draw to be 'off'
               // will be set during autodetection
            g_cfg.chan[eChannel_nr2].status = SET_OFF;
          break;
          case SET_OPT_FREQU:
            g_cfg.chan[eChannel_nr2].status = SET_OFF;
          break;
          case SET_OPT_TUNING:
            g_cfg.chan[eChannel_nr2].status = SET_OFF;
          break;
          default:
            g_cfg.chan[eChannel_nr2].status = SET_OFF;
          break;
        }
      }
    break;
    case 8:
      /* nothing to do */
    break;
    default:
    break;
  }
}

void CMenu::_defaultMenu(void) {
  uint8_t x1=_menuctrl.cursor_x+39;
  uint8_t y=_index2_ypixel(_menuctrl.rowindex);
  clearDisplay();
  setTextSize(0);
  for (uint8_t irow=1; irow <= 8; irow++) {
    _print_row_str(irow, g_cfg.selected_channel);
    this->println();
  }
  this->drawLine((int16_t)_menuctrl.cursor_x, (int16_t)y, (int16_t)x1, (int16_t)y, SH110X_WHITE);
  display();
}

void CMenu::_saveMenu(bool save_data) {
  clearDisplay();
  setTextSize(0);
  setCursor(10, 16);
  this->print(F("Save config-data?"));
  setCursor(SAVE_POS_X, SAVE_POX_Y);
  if (save_data)  {
      this->setTextColor(SH110X_WHITE);
      this->print(SAVE_NO);
      this->print(STR_SLASH);
      this->setTextColor(SH110X_BLACK, SH110X_WHITE);
      this->print(SAVE_YES);
      this->setTextColor(SH110X_WHITE);
  } else {
      this->setTextColor(SH110X_BLACK, SH110X_WHITE);
      this->print(SAVE_NO);
      this->setTextColor(SH110X_WHITE);
      this->print(STR_SLASH);
      this->print(SAVE_YES);
  }
  display();
}

void CMenu::_print_time_str(const uint8_t timevalue) {
  switch (timevalue) {
    case MENU_TIM_50US_VALUE:
      this->print(STR_MENU_TIM_50US);
      this->print(STR_MENU_USEC);
    break;
    case MENU_TIM_01MS_VALUE:
      this->print(STR_MENU_TIM_01MS);
      this->print(STR_MENU_USEC);
    break;
    case MENU_TIM_02MS_VALUE:
      this->print(STR_MENU_TIM_02MS);
      this->print(STR_MENU_USEC);
    break;
    case MENU_TIM_05MS_VALUE:
      this->print(STR_MENU_TIM_05MS);
      this->print(STR_MENU_USEC);
    break;
    case MENU_TIM_1MS_VALUE:
      this->print(STR_MENU_TIM_1MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_2MS_VALUE:
      this->print(STR_MENU_TIM_2MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_5MS_VALUE:
      this->print(STR_MENU_TIM_5MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_10MS_VALUE:
      this->print(STR_MENU_TIM_10MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_20MS_VALUE:
      this->print(STR_MENU_TIM_20MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_50MS_VALUE:
      this->print(STR_MENU_TIM_50MS);
      this->print(STR_MENU_MSEC);
    break;
    case MENU_TIM_100MS_VALUE:
      this->print(STR_MENU_TIM_100MS);
      this->print(STR_MENU_MSEC);
    break;
    default:
    break;
  }
}

void CMenu::_print_trigger_str(const uint8_t triggervalue) {
  switch (triggervalue) {
    case SET_TRIG_OFF :
      this->print(F("Off   "));
    break;
    case SET_TRIG_AUTO_P :
      this->print(F("Auto +"));
    break;
    case SET_TRIG_AUTO_N :
      this->print(F("Auto -"));
    break;
    case SET_TRIG_NORM_P :
      this->print(F("Norm +"));
    break;
    case SET_TRIG_NORM_N :
      this->print(F("Norm -"));
    break;
    default:
    break;
  }
}

void CMenu::_print_triggerlevel_str(const uint8_t triggerlevel) {
  switch (triggerlevel) {
    case SET_TRIG_LEVEL_INTERN :
      this->print(F("Bandgap"));
    break;
    case SET_TRIG_LEVEL_EXTERN :
      this->print(F("Ext.Ref"));
    break;
    default:
    break;
  }
}

void CMenu::_print_option_str(const uint8_t optionvalue) {
  switch (optionvalue) {
    case SET_OPT_SINGLE:
      this->print(F("Single"));
    break;
    case SET_OPT_DUAL:
      this->print(F("Dual"));
    break;
    case SET_OPT_DUAL_PLUGGED:
      this->print(F("Dual(plugin)"));
    break;
    case SET_OPT_FREQU:
      this->print(F("Frequency"));
    break;
    case SET_OPT_TUNING:
      this->print(F("Tuning check"));
    break;
    default:
    break;
  }
}

void CMenu::_print_onoff_str(const uint8_t onoffvalue) {
  switch (onoffvalue) {
    case 0:
      this->print(F("Off"));
    break;
    case 1:
      this->print(F("On "));
    break;
    default:
    break;
  }
}

uint8_t CMenu::_index2_ypixel(const uint8_t index) {
  uint8_t rtn_value = 8 * index;
  if (rtn_value >= this->height()) {
    rtn_value = this->height()-1;
  }
  return rtn_value;
}

/*!
 *
 * name: _draw_channels
 * @brief  drawing sampled data to the display.
 * @param  none
 * @return none
 *
 */
void CMenu::_draw_channels(void) {
  // Testpin Toggle PortC PC5
//  PINC = (1<<PINC5);

  if (CMenu::_draw_channels_running) {
    return;
  } else {
    CMenu::_draw_channels_running = true;
  }
  int16_t y0_1, y1_1;
  int16_t y0_2, y1_2;

  //get frequency-value 10 times higher
  this->_read_frequency(_frequ_meas_value10, 10);
  if (_update_draw_request()) {
    this->clearDisplay();
    if ((g_cfg.chan[eChannel_nr1].option == SET_OPT_FREQU) ||
        (g_cfg.chan[eChannel_nr1].option == SET_OPT_TUNING))
    {
      _show_measurement();
    } else {
      //draw current sample-time to display
      this->setCursor(86, 0);
      this->print(F("1:"));
      this->_print_time_str(g_cfg.chan[eChannel_nr1].time);
      if (g_cfg.chan[eChannel_nr2].status == 1) {
        this->setCursor(86, 32);
        this->print(F("2:"));
        this->_print_time_str(g_cfg.chan[eChannel_nr2].time);
      }
      //check for option: plugged in on channel2
      if (g_cfg.chan[eChannel_nr1].option == SET_OPT_DUAL_PLUGGED) {
        if(_is_plugged_in()) {
          g_cfg.chan[eChannel_nr2].status = 1;
        } else {
          g_cfg.chan[eChannel_nr2].status = 0;
        }
      }

      for (uint8_t x = 0; x < this->_x_border - 1; x++) {
        //draw channel1
          //draw channel1-samples only if enabled (triggered)
        if (g_cfg.chan[eChannel_nr1].sample_draw) {
          y0_1 = this->_y_border - (int16_t)channel1.data[x];
          y1_1 = this->_y_border - (int16_t)channel1.data[(x+1)];
          if (g_cfg.chan[eChannel_nr2].status == SET_ON) {
            //set amplitude/2, if both draws are visible
            y0_1 = y0_1/2;
            y1_1 = y1_1/2;
          }
          y0_1 -= g_cfg.chan[eChannel_nr1].offset;
          y1_1 -= g_cfg.chan[eChannel_nr1].offset;
          //check range, max +-1 line out of boarder for best drawing
          y0_1 = range(y0_1, -1, this->_y_border+1);
          y1_1 = range(y1_1, -1, this->_y_border+1);
          this->drawLine((int16_t)x, y0_1, (int16_t)(x+1), y1_1, SH110X_WHITE); //left to right
        }

        //draw channel2-samples only if second draw is enabled
        if ((g_cfg.chan[eChannel_nr2].status == 1) && pchannel2 != NULL)
        {
          //set amplitude/2 and add offset
          y0_2 = (this->_y_border - (int16_t)pchannel2->data[x])/2 + 31 - g_cfg.chan[eChannel_nr2].offset;
          y1_2 = (this->_y_border - (int16_t)pchannel2->data[(x+1)])/2 + 31 - g_cfg.chan[eChannel_nr2].offset;
          //check range, max +-1 line out of boarder for best drawing
          y0_2 = range(y0_2, -1, this->_y_border+1);
          y1_2 = range(y1_2, -1, this->_y_border+1);
          this->drawLine((int16_t)x, y0_2, (int16_t)(x+1), y1_2, SH110X_WHITE); //left to right
        }
        this->display();
      }
    }
  } //end if (_update_draw_request()

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    //channel1
    if (g_cfg.chan[eChannel_nr1].trigger_mode == SET_OFF) {
      g_cfg.chan[eChannel_nr1].sample_draw = true;
      g_cfg.chan[eChannel_nr1].sample_start= true;
    } else {
      g_cfg.chan[eChannel_nr1].sample_draw = false;
    }
    //channel2
    if (g_cfg.chan[eChannel_nr2].status == SET_OFF) {
      g_cfg.chan[eChannel_nr2].sample_draw = false;
      g_cfg.chan[eChannel_nr2].sample_start= false;
    } else {
      g_cfg.chan[eChannel_nr2].sample_draw = false;
    }
  }  //end ATOMIC_BLOCK()
  CMenu::_draw_channels_running = false;
  // Testpin Toggle PortC PC5
//  PINC = (1<<PINC5);
}

/*!
 *
 * name: _show_measurement
 * @brief  show measurement values on display.
 * @param  none
 * @return none
 *
 */
void CMenu::_show_measurement(void) {
  //show measured-value: frequency from channel1 (if enabled)
  if ( g_cfg.chan[eChannel_nr1].option == SET_OPT_FREQU ) {
    _draw_frequency_value(DRAW_BIG_SIZE);
  }
  //show measured note-string (if enabled)
  if (g_cfg.chan[eChannel_nr1].option == SET_OPT_TUNING) {
    _draw_note_value();
    _draw_frequency_value();
  }
}

void CMenu::_draw_frequency_value(bool bigsize) {
  // draw frequency-value if options are set to 'On' channel1.
  uint16_t current_value = (uint16_t)_frequ_meas_value10/10.0;
  if (bigsize) {
    setTextSize(2);
    setCursor(10, 25);
  } else {
    setTextSize(0);
    this->setCursor(0, 47);
    this->print(F("Freq(Hz):"));
  }
  //this-print((float)_frequency_value, 1); //will not work
  this->print((uint16_t)current_value);
  this->print(F("."));
  this->print((uint16_t)(_frequ_meas_value10 - current_value*10.0));
  if (bigsize) {
    this->print(F(" Hz"));
  }
  this->display();
}

void CMenu::_draw_note_value(void) {
  uint8_t xpos = 64;
  uint8_t note_index=0;
  this->setCursor(43, 0);
  note_index = _print_note_value();
  //required pitch-mark in the middle
  this->drawLine(64, 10, 64, 18, SH110X_WHITE);
  //draw ruler
  for (uint8_t x=4; x < this->_x_border; x+=6) {
    if ((x-4) % 12) {
      this->drawLine(x, 19, x, 23, SH110X_WHITE);
    } else {
      this->drawLine(x, 19, x, 27, SH110X_WHITE);
    }
  }
  xpos = this->_get_procent_xpos(note_index, this->_search_frequency);
  //draw current measured pitch-mark
  this->drawLine(xpos, 30, xpos, 40, SH110X_WHITE);
  this->display();
}

void CMenu::_read_frequency(double & freq_meas, const uint16_t multiply) {
  if (FreqMeasure.available()) {
    freq_meas = (double)(multiply * FreqMeasure.countToFrequency(FreqMeasure.read() / 1));
  }
}

int8_t CMenu::_print_note_value(void) {
  int8_t note_index=0;
  note_index = _find_note_index();
  if (note_index < 0 ) {
    this->print(STR_NOTE_FAIL_R);
    this->print(STR_NOTE_NONE);
    this->print(_note_octave);
    this->print(STR_NOTE_OK_L);
  }
  if (note_index == 0 ) {
    this->print(STR_NOTE_FAIL_R);
    this->print(STR_NOTE_NONE);
    this->print(_note_octave);
    this->print(STR_NOTE_FAIL_L);
  }
  if (note_index >= 99 ) {
    this->print(STR_NOTE_OK_R);
    this->print(STR_NOTE_NONE);
    this->print(_note_octave);
    this->print(STR_NOTE_FAIL_L);
  }
  if ((note_index>0) && (note_index<13)) {
    this->print(STR_NOTE_OK_R);
    if (_is_inlimits(note_index, this->_search_frequency)) {
      this->setTextColor(SH110X_BLACK, SH110X_WHITE);
    }
    switch (note_index) {
      case 1:
        this->print(STR_NOTE_C);
      break;
      case 2:
        this->print(STR_NOTE_CIS);
      break;
      case 3:
        this->print(STR_NOTE_D);
      break;
      case 4:
        this->print(STR_NOTE_DIS);
      break;
      case 5:
        this->print(STR_NOTE_E);
      break;
      case 6:
        this->print(STR_NOTE_F);
      break;
      case 7:
        this->print(STR_NOTE_FIS);
      break;
      case 8:
        this->print(STR_NOTE_G);
      break;
      case 9:
        this->print(STR_NOTE_GIS);
      break;
      case 10:
        this->print(STR_NOTE_A);
      break;
      case 11:
        this->print(STR_NOTE_AIS);
      break;
      case 12:
        this->print(STR_NOTE_B);
      break;
      default:
        this->print(STR_NOTE_NONE);
      break;
    }
    this->print(_note_octave);
    this->setTextColor(SH110X_WHITE);
    this->print(STR_NOTE_OK_L);
  }
  return note_index;
}

int8_t CMenu::_find_note_index(void) {
  //return-values are defind as:
  //  -1 -> frequency-value is to low
  //   0 -> no matching note found
  //  1...12 matching note value found
  //  99 -> frequency-value is to high
  int8_t rtn_value = 0;
  int8_t loops = 5;
  bool found=false;
  uint16_t lower_limit, upper_limit;

  _search_frequency = (uint16_t)_frequ_meas_value10;
  if (_search_frequency == 0) {
    return 0;
  }
  //preset for search
  _note_octave = 4;

  while (!found && (loops>0)) {
    if ((_search_frequency > pgm_read_word(&g_ref_frequ10[0])) && \
        (_search_frequency < pgm_read_word(&g_ref_frequ10[13])))
    {
      found=true;
      break;
    } else {
      if (_search_frequency > pgm_read_word(&g_ref_frequ10[13])) {
        _search_frequency /= 2;
        _note_octave += 1;
        rtn_value = 99;
      }
      if (_search_frequency < pgm_read_word(&g_ref_frequ10[0])) {
        _search_frequency *= 2;
        _note_octave -= 1;
        rtn_value = -1;
      }
    }
    loops--;
  } //end while
  _note_octave = range(_note_octave, 0 ,8);
  //find note and return note-index
  for (uint8_t index=1; index < 13; index++) {
    _get_limits(pgm_read_word(&g_ref_frequ10[index]), lower_limit, upper_limit);
    if ((_search_frequency > lower_limit) && \
        (_search_frequency < upper_limit)) {
      rtn_value = index;
      break;
    }
  }
  return rtn_value;
}

bool CMenu::_is_inlimits(const uint8_t & noteindex, const uint16_t &current_freq, const uint8_t percent) {
  bool rtn_value=false;
  uint16_t lower, upper;
  uint16_t nominal_freq = pgm_read_word(&g_ref_frequ10[noteindex]);
  _get_limits(nominal_freq, lower, upper, percent);
  if ((current_freq >= lower) && \
      (current_freq < upper)) {
    rtn_value=true;
  }
  return rtn_value;
}

void CMenu::_get_limits(const uint16_t nominal, uint16_t & lower, uint16_t & upper, const uint8_t percent) {
  double limit;
  limit = ((double)nominal * (100L-percent)/100L);
  lower = (uint16_t)limit;
  limit = ((double)nominal * (100L+percent)/100L);
  upper = (uint16_t)limit;
}

uint8_t CMenu::_get_procent_xpos(const uint8_t & noteindex, const uint16_t &current_freq) {
  uint8_t rtn_value = 255;
  double pixel_value=0L;
  int16_t freq_diff = 0;
  int16_t permillies = 0;
  int8_t percent_value = 0;
  uint16_t nominal_freq = 0;

  if (noteindex>0 && noteindex<13) {
    nominal_freq = pgm_read_word(&g_ref_frequ10[noteindex]);
    freq_diff = current_freq - nominal_freq;
    // 12 pixles pro-cent
    pixel_value = static_cast<double>(freq_diff)/static_cast<double>(nominal_freq);
    pixel_value *= 1200L;
    permillies = static_cast<int16_t>(pixel_value*10L/12L);
    rtn_value = static_cast<uint8_t>(pixel_value) + 64;
  }
  if (rtn_value == 255) {
    rtn_value = 0; // left xpos as default on error
  } else {
    rtn_value = range(rtn_value, 0, this->_x_border);;
  }
  //print percent-value on display
  percent_value = static_cast<int8_t>(permillies/10);
  this->setCursor(98, 0);
  if (permillies >= 0) {
    this->print(F("+"));
  } else {
    this->print(F("-"));
  }
  this->print(abs(percent_value));
  this->print(F("."));
  this->print(abs(permillies - percent_value*10));
  this->print(F("%"));
  this->display();
  return rtn_value;
}

bool CMenu::_update_draw_request(void) {
  bool rtn_value=false;
  if (_frequ_meas_value10 > 0L) {
    uint16_t lower, upper, current_freq;
    _get_limits(_old_frequ_meas_value, lower, upper);
    current_freq = (uint16_t)(_frequ_meas_value10/10L);
    if ((current_freq >= lower) && \
        (current_freq < upper)) {
      rtn_value=false;
    } else {
      rtn_value=true;
      _old_frequ_meas_value = current_freq;
    }

  }
  //if timeout then update-request
  if (rtn_value == false) {
    if ((millis() - _drawupdateTimer) > DRAWUPDATE_TIMEOUT) {
      _drawupdateTimer = millis();
      rtn_value=true;
    }
  }
  return rtn_value;
}

/*!
 *
 * name:   _is_plugged_in()
 * @param  none
 * @return true, if channel2 is plugged in, else false
 *
 */
bool CMenu::_is_plugged_in(void) {
  bool rtn_value = false;
  uint16_t average_value_ch2 = 0;
  uint8_t max_value=0, min_value=128;
  if (pchannel2 != NULL) {
    for (uint8_t x=0; x < (this->_x_border - 1); x++) {
      max_value = max(pchannel2->data[x], max_value);
      min_value = min(pchannel2->data[x], min_value);
      average_value_ch2 += (uint16_t)pchannel2->data[x];
    }
    average_value_ch2 /= (this->_x_border - 1);
    if (((max_value - min_value) <= 5) && (uint8_t)(average_value_ch2 < 15)) {
      rtn_value = false;
    } else {
      rtn_value = true;
    }
  }
  return rtn_value;
}

/*!
 *
 * name:   _make_checksum()
 * @param  mem_type: type of memory-location for checksum
 * @return XOR checksum from cfg-data
 *
 */
uint8_t CMenu::_make_checksum(const uint8_t mem_type) {
  uint8_t checksum = 0;
  uint8_t * p_byte = (uint8_t *)&g_cfg;

  for (int addr=0; addr<sizeof(cfg_t); addr++) {
    if (mem_type == MEM_TYPE_EEPROM) {
      checksum ^= EEPROM[ADDR_CFG_DATA_BASE + addr];
    }
    if (mem_type == MEM_TYPE_GLOBAL) {
      checksum ^= *p_byte++;
    }
  }
  return checksum;
}

/*!
 *
 * name:   _IsEEPROM_data_valid()
 * @param  none
 * @return true, if EEPROM-data valid, else false
 *
 */
bool CMenu::_IsEEPROM_data_valid(void) {
  if (EEPROM[ADDR_CHECKSUM] == _make_checksum(MEM_TYPE_EEPROM)) {
    return true;
  } else {
    return false;
  }
}

/*!
 *
 * name:   _EEPROM_write_cfg()
 * @param  none
 * @return true, if EEPROM-data write successfull, else false
 *
 */
bool CMenu::_EEPROM_write_cfg(void) {
  bool rtn_value = false;
  uint8_t checksum = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    checksum = _make_checksum(MEM_TYPE_GLOBAL);
    //write global config-data
    EEPROM.put(ADDR_CFG_DATA_BASE, g_cfg);
    //write checksum
    EEPROM.write(ADDR_CHECKSUM, checksum);
  }  //end ATOMIC_BLOCK()
  //compare global config-checksum with EEPROM-checksum
  if (checksum == _make_checksum(MEM_TYPE_EEPROM)) {
    rtn_value = true;
  }
  return rtn_value;
}

void CMenu::SaveConfigdata(void) {
  _menu_state = SAVE_REQUEST;
  updateMenu();
}
