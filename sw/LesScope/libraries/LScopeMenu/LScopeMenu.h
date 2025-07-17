/*
 * @file    LScopeMenu.h
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   declaration-file for 'LScopeMenu.cpp'.
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

#ifndef _LSCOPEMENU_h_
#define _LSCOPEMENU_h_

#include <stdio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <EEPROM.h>

#ifndef cfg_t
  #include "LScopeCfg.h"
#endif

#include "rotaryencoder.h"

// Show Default Menu for 5 seconds if no encoder-data available.
#define WAIT4ACTIONS_TIMEOUT 5000
#define DRAWUPDATE_MSEC       100
#define DRAWUPDATE_TIMEOUT    500

typedef enum eMenu {
  STARTUP,            // 0
  INITDEFAULTS,
  MENU_DEFAULT,
  SETTINGS,
  SELECT_VALUES,
  DRAW_SAMPLES,
  SAVE_REQUEST,
  SAVE_SET,
  SAVE_DATA,
} eMenu_t;

typedef struct menuctrl_val {
  uint8_t rowindex;
  uint8_t cursor_x;
  uint8_t cursor_y;
  bool mark_on;
  bool menu_updated;
  bool save_yes;
} menuctrl_t;

typedef struct menutimer_val {
  bool expired;
  unsigned long int Timeout;
} menutimer_t;

typedef struct note_value {
  // 10 times above nominal frequency
  uint16_t ref_frequ10;
} note_values_t;

//Menu strings
#define STR_MENU_CHANNEL      F("Channel: ")
#define STR_MENU_AMP          F("Amplify: * ")
#define STR_MENU_TIME         F("S-Time : ")
#define STR_MENU_TRIGGER      F("Trigger: ")
#define STR_MENU_TRG_LEVEL    F("TrgLevl: ")
#define STR_MENU_OFFSET       F("Offset : ")
#define STR_MENU_OPTION       F("Option : ")

#define STR_MENU_TIM_50US     F(" 50")
#define STR_MENU_TIM_01MS     F("100")
#define STR_MENU_TIM_02MS     F("200")
#define STR_MENU_TIM_05MS     F("500")
#define STR_MENU_TIM_1MS      F("1.0")
#define STR_MENU_TIM_2MS      F("2.0")
#define STR_MENU_TIM_5MS      F("5.0")
#define STR_MENU_TIM_10MS     F(" 10")
#define STR_MENU_TIM_20MS     F(" 20")
#define STR_MENU_TIM_50MS     F(" 50")
#define STR_MENU_TIM_100MS    F("100")
#define STR_MENU_MSEC         F("ms")
#define STR_MENU_USEC         F("us")

#define MENU_TIM_50US_VALUE   1
#define MENU_TIM_01MS_VALUE   2
#define MENU_TIM_02MS_VALUE   3
#define MENU_TIM_05MS_VALUE   4
#define MENU_TIM_1MS_VALUE    5
#define MENU_TIM_2MS_VALUE    6
#define MENU_TIM_5MS_VALUE    7
#define MENU_TIM_10MS_VALUE   8
#define MENU_TIM_20MS_VALUE   9
#define MENU_TIM_50MS_VALUE   10
#define MENU_TIM_100MS_VALUE  11

#define STR_MENU_LESSCOPE     F("LesScope")
#define STR_MENU_MODUL_TYPE   F("Dual Channel Scope")
#define STR_MENU_OWNER        F("github.com/ImoogDi")

#define DRAW_BIG_SIZE         true

//note str-defines
#define STR_NOTE_C            F("C ")
#define STR_NOTE_CIS          F("C#")
#define STR_NOTE_D            F("D ")
#define STR_NOTE_DIS          F("D#")
#define STR_NOTE_E            F("E ")
#define STR_NOTE_F            F("F ")
#define STR_NOTE_FIS          F("F#")
#define STR_NOTE_G            F("G ")
#define STR_NOTE_GIS          F("G#")
#define STR_NOTE_A            F("A ")
#define STR_NOTE_AIS          F("A#")
#define STR_NOTE_B            F("B ")
#define STR_NOTE_NONE         F("..")
#define STR_NOTE_OK_R         F(" >")
#define STR_NOTE_OK_L         F("< ")
#define STR_NOTE_FAIL_L       F(">>")
#define STR_NOTE_FAIL_R       F("<<")
#define STR_SLASH             F("/")

#define STR_OK                F("OK")
#define STR_FAILED            F("write failed")

#define SAVE_POS_X            40
#define SAVE_POX_Y            32
#define SAVE_YES              F("yes")
#define SAVE_NO               F("no")

#define MEM_TYPE_EEPROM       1
#define MEM_TYPE_GLOBAL       2

/*!
 *
 * name: CMenu
 * @brief  Class that handles menu-items using SH1106 OLED displays.
 * @param  none
 * @return none
 *
 */
class CMenu : public Adafruit_SH1106G
{
  bool _draw_channels_running;

  public:
    CMenu(uint16_t w, uint16_t h, SPIClass *spi, int16_t dc_pin, int16_t rst_pin, int16_t cs_pin);
    ~CMenu(void);

    CRotaryEncoder rotaryencoder;

    void begin(int addr=0, bool reset=true);
    void init_cfg(void);
    // Called whenever button is pushed
    void updateMenu(void);
    // Called whenever encoder is turned
    void updateSelection(void);
    void SetPrevMenu(void) {
      _menu_state = _prev_menu;
      updateSelection();
    }
    void SetDefaultMenu(void) {
      _menu_state = MENU_DEFAULT;
      if (_prev_menu != MENU_DEFAULT) {
        updateSelection();
      }
    }
    void CheckMenuTimeout(void);
    void Drawupdate(void);
    void displayModulName(void);
    void SaveConfigdata(void);

  private:
    int16_t _x_border;
    int16_t _y_border;
    eMenu_t _menu_state{STARTUP};
    eMenu_t _prev_menu{MENU_DEFAULT};
    menuctrl_t _menuctrl;
    //frequency-measurent values
    double   _frequ_meas_value10={0L};
    uint16_t _old_frequ_meas_value={0};
    uint16_t _search_frequency={0};
    int8_t   _note_octave={4};
    // Flag indicating expired timer to show Default-menu
    menutimer_t _menutimer;
    unsigned long int _drawupdateTimer{0L};

    void _draw_frequency_value(bool bigsize=false);
    void _draw_note_value(void);
    uint8_t _index2_ypixel(const uint8_t index);
    void _print_time_str(const uint8_t timevalue);
    void _print_trigger_str(const uint8_t triggervalue);
    void _print_triggerlevel_str(const uint8_t triggerlevel);
    void _print_option_str(const uint8_t messvalue);
    void _print_onoff_str(const uint8_t filtervalue);
    void _print_row_str(const uint8_t index, const channel_nr_t channel_nr);
    void _print_value_str(const uint8_t index, const channel_nr_t channel_nr, const uint8_t oldvalue);
    void _read_frequency(double & freq_meas, const uint16_t multiply=1);
    int8_t _print_note_value(void);
    int8_t _find_note_index(void);
    uint8_t _get_procent_xpos(const uint8_t & noteindex, const uint16_t &current_freq);
    void _get_limits(const uint16_t nominal_freq, uint16_t & lower, uint16_t & upper, const uint8_t percent=3);
    bool _is_inlimits(const uint8_t & noteindex, const uint16_t & current_freq, const uint8_t percent=1);
    bool _update_draw_request(void);
    bool _is_plugged_in(void);

    void _defaultMenu(void);
    void _InitDisplay(void);
    void _draw_channels(void);
    void _show_measurement(void);
    void _saveMenu(bool save_data = false);

    uint8_t _make_checksum(const  uint8_t mem_type=MEM_TYPE_GLOBAL);
    bool _IsEEPROM_data_valid(void);
    bool _EEPROM_write_cfg(void);
}; //end CMenu

#endif // end #ifndef _LSCOPEMENU_h_
