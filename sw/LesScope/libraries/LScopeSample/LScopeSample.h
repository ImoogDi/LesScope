/*
 * @file    LScopeSample.h
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   declaration-file for 'LScopeSample.c'.
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

#ifndef _LSCOPESAMPLE_h_
#define _LSCOPESAMPLE_h_

/* Enable c-linkage */
#if defined (__cplusplus)
  extern "C" {
#endif

  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <assert.h>
  #include "LScopeCfg.h"

  #ifndef F_CPU
    #define F_CPU 16000000UL
  #endif

  #define SAMPLE_DATA_SIZE  128
  typedef struct sample {
    uint8_t data[SAMPLE_DATA_SIZE];
    uint8_t index;
  } sample_t;

  extern sample_t channel1;
  extern sample_t channel2;

  void sample_init( void );
  void set_counter_defaults( void );
  bool is_triggertimeout(channel_nr_t eChannel, const uint8_t menu_timeout);

#if defined (__cplusplus)
} //extern "C"
#endif


#endif // end #ifndef _LSCOPESAMPLE_h_
