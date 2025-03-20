/*
 * @file    LScopeSetHW.c
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   initialisation and configuration of trigger- and amplifier-data.
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

#include "LScopeCfg.h"
#include "LScopeSetHW.h"

/*!
 *
 * name: hw_init
 * @brief  hardware initialisation.
 * @param  none
 * @return none
 *
 */
void hw_init(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    analogReference(DEFAULT);
    //pin mode setting
    pinMode(7, INPUT); //external trigger detect
    pinMode(12, INPUT_PULLUP);// Encoder push sw

    ////fast pwm setting
      // set ForceOutputCompare A/B off and No Clock on Timer2
    TCCR2B = 0;

    //setup analog comparator for channel1
    //  Arduino Nano Pins
    //  Pin:D6->(AIN0) := 'Channel1 offset-DC'/2 or 'Bandgap Ref.'
    //  Pin:D7->(AIN1) := 'Trigger Input for MCU'
    //                |\
    //    AIN0 -------|+ \---{Interrupt-select}-+->{Analog Comparator IRQ}
    //    AIN1 -------|- /     (Source/Slope)   |
    //                |/                        +-> ACO Out to Timer1
    //
    ADCSRB &= (uint8_t)~(1<<ACME); //Analog Comparator-Mux disable -> AIN1
    ACSR &= (uint8_t)~(1<<ACIE);  //Analog Comparator Interrupt disable
    ACSR |= (1<<ACIS1); //Analog Comparator Interrupt on falling edge
    ACSR |= (1<<ACBG);  //Bandgab Reference enabled
    //enable Comparator Out and set Input Capture Enable for Timer1
    ACSR |= ((1<<ACO)|(1<<ACIC));
    // set the noise canceler bit for timer1 and falling edge
    TCCR1B |= (1<<ICNC1);
    TCCR1B &= (uint8_t)~(1<<ICES1);

    // setup Timer2 for PWM on D3(OC2B) and sample-timesteps
    // disable powersaving mode for Timer2
    PRR &= (uint8_t)~(PRTIM2);
    // setup:clear OC2B on compare and Fast PWM Mode:7
    TCCR2A |= (1<<COM2B1)|(1<<WGM21)|(1<<WGM20);
    // set clock-select to clk/8 and Fast PWM Flag:WGM22
    TCCR2B |= (1<<WGM22)|(1<<CS21);

    set_amplifier(eChannel_nr1);
    set_amplifier(eChannel_nr2);

    //set portC5 as testpoint output and low
    DDRC |= (1<<DDC5);
    PORTC &= (uint8_t)~(1<<PORTC5);
  } //end ATOMIC_BLOCK()
}

/*!
 *
 * name: set_amplifier
    @brief  amplifier set to sig-in*1 or *2 factor.
    @param  echannel_nr: channel-number to be set.
*/
void set_amplifier(const channel_nr_t echannel_nr) {

  switch(echannel_nr) {
    // setup channel 1
    case eChannel_nr1:
      // This offset (PWM) depends on Timer2 'OCR2A' value: TIMER2_SAMPLE
         // see: sample_init() in modul: LScopeSample.c
      pinMode(OFFSETPIN_OUT_CHAN1, OUTPUT) ;//offset voltage-Channel1 activate
      switch (g_cfg.chan[eChannel_nr1].amplifier) {
        case SET_AMP_LEVEL_1:
          //offset channel1 = 2.5V set with PWM 50%
          analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 2);
          //set port to input for amplification * 1
          pinMode(A2, INPUT);
          pinMode(A4, INPUT);
        break;
        case SET_AMP_LEVEL_2:
          //offset channel1 = 1.25V set with PWM 25%
          analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 4);
          //wait for PWM offset set
          delay(100L);
          digitalWrite(A2, 0);
          //set port to output for amplification * 2
          pinMode(A2, OUTPUT);
          pinMode(A4, INPUT);
        break;
        case SET_AMP_LEVEL_3:
          //offset channel1 = 0.625V set with PWM 12,5%
          analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 8);
          //wait for PWM offset set
          delay(100L);
          pinMode(A2, INPUT);
          //set port to output for amplification * 3
          digitalWrite(A4, 0);
          pinMode(A4, OUTPUT);
        break;
        case SET_AMP_LEVEL_4 :
          //offset channel1 = 0.32V set with PWM 6,25%
          analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 16);
          //wait for PWM offset set
          delay(100L);
          digitalWrite(A2, 0);
          pinMode(A2, OUTPUT);
          //set port to output for amplification * 4
          digitalWrite(A4, 0);
          pinMode(A4, OUTPUT);
        break;
        default:
          //offset channel1 = 2.5V set with PWM 50%
          analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 2);
          //set port to input for amplification * 1
          pinMode(A2, INPUT);
          pinMode(A4, INPUT);
        break;
      }
/*
      if (g_cfg.chan[eChannel_nr1].amplifier == SET_AMP_LEVEL_2) {
        //offset channel1 = 1.25V set with PWM 25%
        analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 4);
        //wait for PWM offset set
        delay(100L);
        digitalWrite(A2, 0);
        //set port to output for amplification * 2
        pinMode(A2, OUTPUT);
      } else {
        //offset channel1 = 2.5V set with PWM 50%
        analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 2);
        //set port to input for amplification * 1
        pinMode(A2, INPUT);
      }
*/
    break;
    // setup channel 2
    case eChannel_nr2:
      pinMode(OFFSETPIN_OUT_CHAN2, OUTPUT) ;//offset voltage-Channel2 activate
      if (g_cfg.chan[eChannel_nr2].amplifier == SET_AMP_LEVEL_2) {
        //offset channel1 = 2.5V set with PWM 25%
        analogWrite(OFFSETPIN_OUT_CHAN2, OFFSET_1_25_VOLT);
        //wait for PWM offset set
        delay(100L);
        digitalWrite(A3, 0);
        //set port to output for amplification * 2
        pinMode(A3, OUTPUT);
      } else {
        //offset channel2 = 2.5V set with PWM 50%
        analogWrite(OFFSETPIN_OUT_CHAN2, OFFSET_2_5_VOLT);
        //set port to input for amplification * 1
        pinMode(A3, INPUT);
      }
    break;
    default:
      pinMode(OFFSETPIN_OUT_CHAN1, OUTPUT) ;//offset voltage-Channel1 activate
      pinMode(OFFSETPIN_OUT_CHAN2, OUTPUT) ;//offset voltage-Channel2 activate
      //offset channel1 = 2.5V set with PWM 50%
      analogWrite(OFFSETPIN_OUT_CHAN1, TIMER2_SAMPLE / 2);
      //offset channel2 = 2.5V set with PWM 50%
      analogWrite(OFFSETPIN_OUT_CHAN2, OFFSET_2_5_VOLT);
      //set ports to input for amplification * 1
      pinMode(A2, INPUT);
      pinMode(A3, INPUT);
      pinMode(A4, INPUT);
    break;
  }
}

/*!
 * name: set_trigger_mode
    @brief  function set the triggermode on selected channel for
              positiv and negativ slopes.
              1. Mode: Off -  > free run
              2. Mode: Auto  -> triggered sampling and display.
              *                 without trigger, empty line is drawn.
              3. Mode: Normal-> triggered sampling and display.
              *                 without trigger, nothing drawn.
              !! Make sure calling this with cli() interrupts disabled !!
        @param  eChannel_nr: channel-number to be set.
*/
void set_trigger_mode(const channel_nr_t eChannel_nr) {
  switch (g_cfg.chan[eChannel_nr].trigger_mode) {
    case SET_OFF :
      //free run for display and sampling
      if (eChannel_nr == eChannel_nr1) {
        g_cfg.chan[eChannel_nr].sample_draw = true;
        ACSR &= (uint8_t)~(1<<ACIE);  //Analog Comparator Interrupt disable
      }
      //only free-run sample-draw on channel2, if enabled
      if (eChannel_nr == eChannel_nr2) {
        g_cfg.chan[eChannel_nr].sample_draw = g_cfg.chan[eChannel_nr].status;
      }
    break;
    case SET_TRIG_AUTO_P :
      // timeout-check on main-loop with 'is_triggertimeout()'.
      // no break required
    case SET_TRIG_NORM_P :
      if (eChannel_nr == eChannel_nr1) {
        ACSR &= (uint8_t)~(1<<ACIE);  //Analog Comparator Interrupt disable
        ACSR &= (uint8_t)~(1<<ACIS0); //clear settingmode-flag: 0
        ACSR |= (1<<ACIS1); //Analog Comparator Interrupt on falling edge
        ACSR |= (1<<ACIE);  //Analog Comparator Interrupt enable
      }
    break;
    case SET_TRIG_AUTO_N :
      // timeout-check on main-loop with 'is_triggertimeout()'.
      // no break required
    case SET_TRIG_NORM_N :
      if (eChannel_nr == eChannel_nr1) {
        ACSR &= (uint8_t)~(1<<ACIE);  //Analog Comparator Interrupt disable
        ACSR |= ((1<<ACIS1)|(1<<ACIS0)); //Analog Comparator Interrupt on rising edge
        ACSR |= (1<<ACIE);  //Analog Comparator Interrupt enable
      }
    break;
    default :
    break;
  }
}

/*!
 *
 * name: set_trigger_level
 * @brief  sets trigger-level to 1. internal or 2. external reference.
 * @param  none
 * @return none
 *
 */
void set_trigger_level(void) {
  switch (g_cfg.chan[eChannel_nr1].trigger_level) {
    case SET_TRIG_LEVEL_INTERN:
      ACSR |= (1<<ACBG);  //Bandgab Reference enabled
    break;
    case SET_TRIG_LEVEL_EXTERN:
      //Bandgab Reference disabled and
        // AIN0 (D6) enabled for analog comparator
      ACSR &= (uint8_t)~(1<<ACBG);
    break;
    default:
      ACSR |= (1<<ACBG);  //Bandgab Reference enabled
    break;
  }
}
