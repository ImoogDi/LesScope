/*
 * @file    LScopeSample.c
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   initialisation, sampling and interrupt-handling for LesScope.
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

/*
 * used HW-resources:
 *  1. timer2/OCR2A  for signalsampling 50usec timestamps.
 */

#include "LScopeSample.h"
#include <util/atomic.h>

int16_t _sample_counter1;
int16_t _sample_counter2;

unsigned long int _Trigger_Timeout=0L;

sample_t channel1;
sample_t channel2;

int16_t _counter_values[]={ TIMER2_05MSEC, //0 default
                            TIMER2_50USEC, //1
                            TIMER2_01MSEC, //2
                            TIMER2_02MSEC, //3
                            TIMER2_05MSEC, //4
                            TIMER2_1MSEC,  //5
                            TIMER2_2MSEC,  //6
                            TIMER2_5MSEC,  //7
                            TIMER2_10MSEC, //8
                            TIMER2_20MSEC, //9
                            TIMER2_50MSEC, //10
                            TIMER2_100MSEC,//11
                            TIMER2_50MSEC  //12 unused
                          };


/*!
 *
 * name: sample_init
 *        : timer- and ADC-initialisation.
 *        : timer2 used as sampling-tick (50usec)).
 * @param  none
 * @return none
 *
 */
void sample_init( void )
{
  // Set data direction for Testpoint output on PortC PC5
  DDRC |= (1<<DDC5);
  // Set PortC PC5 to Low
  PORTC &= (uint8_t)~(1<<PORTC5);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    set_counter_defaults();
    channel1.index = 0;
    channel2.index = 0;

    //ADC settings
    ADCSRA &= 0x90; // ADC enable, clear ADIF interrupt-flag
    ADCSRA |= 0x04; // prescaler to /16 speed
    ADCSRB =  0x00; // ADC Free running mode

    //// timer0 is used for millis() / micros() on arduino
      // nothing to do here.

    //// timer1 is used for frequency-measurement together as
      //   Input Capture Unit with Analog Comparator Input.
      // { see hw_init() in LScopeSetHW.c }


    ///// Setup timer2
      // base setup is already done with: hw_init() in modul: LScopSetHW.c
    // set timer2 OCR2A to first compare-value for sample-time
    OCR2A = (uint8_t)(TIMER2_SAMPLE);
    // clear pending interrupts for timer2
    TIFR2  = (1<<OCF2B)|(1<<OCF2A)|(1<<TOV2);
    // Set Interrupt Mask Register for:
    //  enable Timer2 Compare A interrupt
    TIMSK2 = (1<<OCIE2A);
  }  //end ATOMIC_BLOCK()
  _Trigger_Timeout=millis();

  ADCSRA |= (1<<ADSC); //Start ADC conversion
}

/*!
 *
 * name: set_counter_defaults
 *        :sets counter-values to defaults.
 * @param  none
 * @return none
 *
 */
void set_counter_defaults( void )
{
  _sample_counter1 = _counter_values[g_cfg.chan[eChannel_nr1].time];
  _sample_counter2 = _counter_values[g_cfg.chan[eChannel_nr2].time];
}

/**
 *
 */

/*!
 *
 * name: is_triggertimeout
 *        : checks the trigger-timer for timeout in trigger-mode: auto.
 *        : return true on timeout and clears the buffer, else false.
 *
 * @param  channel_nr_t eChannel
 * @param  uint8_t menu_timeout
 * @return true if trigger-event timed out, else false.
 *
 */
bool is_triggertimeout(channel_nr_t eChannel, const uint8_t menu_timeout) {
  bool rtn_value=false;
  uint16_t max_timeout=TRIGGER_TIMEOUT_VALUE_MSEC;
  if ((g_cfg.chan[eChannel].trigger_mode == SET_TRIG_AUTO_P) || \
      (g_cfg.chan[eChannel].trigger_mode == SET_TRIG_AUTO_N))
  {
    if ( g_cfg.chan[eChannel].time > menu_timeout ) {
      max_timeout = 4 * TRIGGER_TIMEOUT_VALUE_MSEC;
    }
    if ((uint16_t)(millis() - _Trigger_Timeout) > max_timeout) {
      //clear sample-buffer
      for (uint8_t x = 0; x < SAMPLE_DATA_SIZE; x++) {
        if (eChannel == eChannel_nr1) {
          channel1.data[x]=31; //set to offset-value channel1
        } else {
          channel2.data[x]=45; //set to offset-value channel2
        }
      }
      rtn_value=true;
    }
  }
  return rtn_value;
}

////////////////////////////////////////////////////////////////
// ISR - handlers aren't used for Timer0 and Timer1
//  they are used for:
//   Timer0: millis() / micros()
//   Timer1: frequency-measurement
////////////////////////////////////////////////////////////////

/*!
 *
 * name: ISR interrupt-service routine
 * @brief  Interrupt Service for Timer2 compare match A.
 * @param  none
 * @return none
 *
 */
ISR(TIMER2_COMPA_vect)
{
  ///// testpin options, activate as required
  // Testpin Toggle PortC PC5
  // PINC = (1<<PINC5);
  // set Testpin high or
  PORTC |= (1<<PORTC5);
  // set Testpin low
  // PORTC &= (uint8_t)~(1<<PORTC5);
  /////

  if (g_cfg.chan[eChannel_nr1].sample_start) {
    if (_sample_counter1 > 0) {
      _sample_counter1--;
    } else {
      // stop auto trigger
      ADCSRA &= (uint8_t)~(1<<ADATE);
      // set channel A0 -->> ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))
      ADMUX = (1<<REFS0);
      // start the ADC-conversion
      ADCSRA |= (1<<ADSC);
      // ADSC is cleared when the conversion finishes
      while ((ADCSRA & (1<<ADSC))) {};
      //get value from ADCL/ADCH and save it
      channel1.data[channel1.index] = ADC/16;
      channel1.index++;
      if (channel1.index >= SAMPLE_DATA_SIZE) {
        channel1.index = 0;
        if (g_cfg.chan[eChannel_nr1].trigger_mode != SET_OFF) {
          g_cfg.chan[eChannel_nr1].sample_start = false;
          g_cfg.chan[eChannel_nr1].sample_draw  = true;
        }
      }
      _sample_counter1  = _counter_values[g_cfg.chan[eChannel_nr1].time];
    }
  } else {
    _sample_counter1  = _counter_values[g_cfg.chan[eChannel_nr1].time];
  }

  //sample channel2, if required
  if (g_cfg.chan[eChannel_nr2].sample_draw  == false) {
    if ((g_cfg.chan[eChannel_nr2].status == 1) || \
        (g_cfg.chan[eChannel_nr1].option  == SET_OPT_DUAL_PLUGGED)
      )
    {
      if (_sample_counter2 > 0) {
        _sample_counter2--;
      } else {
        // stop auto trigger
        ADCSRA &= (uint8_t)~(1<<ADATE);
        // set channel A1 -->> (1<<MUX0)
        ADMUX = (1<<REFS0)|(1<<MUX0);
        // start the ADC-conversion
        ADCSRA |= (1<<ADSC);
        // ADSC is cleared when the conversion finishes
        while ((ADCSRA & (1<<ADSC))) {};
        //get value from ADCL/ADCH and save it
        channel2.data[channel2.index] = ADC/16;
        channel2.index++;
        if (channel2.index >= SAMPLE_DATA_SIZE) {
          channel2.index = 0;
          g_cfg.chan[eChannel_nr2].sample_draw = true;
        }
        _sample_counter2  = _counter_values[g_cfg.chan[eChannel_nr2].time];

        ///// testpin options, activate as required
        //set Testpin low
        PORTC &= (uint8_t)~(1<<PORTC5);
      }
    }
  }
} //end TIMER2_COMPA_vect

/*!
 *
 * name: ISR interrupt-service routine
 * @brief  Interrupt Service for Analog Comparator.
 * @param  none
 * @return none
 *
 */
ISR(ANALOG_COMP_vect)
{
  // Testpin Toggle PortC PC5
  //PINC = (1<<PINC5);

  if ( g_cfg.chan[eChannel_nr1].trigger_mode != SET_OFF ) {
    if (g_cfg.chan[eChannel_nr1].sample_draw == false) {
      if ( g_cfg.chan[eChannel_nr1].sample_start == false ) {
        //reset index channel1 for start sampling with index:=0
        channel1.index = 0;
        _sample_counter1 = 0;
        //reset index channel2 at this trigger-time for best drawing
        channel2.index = 0;
        _sample_counter2 = 0;
        g_cfg.chan[eChannel_nr1].sample_start = true;
        g_cfg.chan[eChannel_nr2].sample_start = true;
  //  Testpin Toggle PortC PC5
  //PINC = (1<<PINC5);
      }
    }
  }
  _Trigger_Timeout=millis();
}
