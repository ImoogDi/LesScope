/*
 * @file    LScopeSetHW.h
 * @author  ImoogDi (https://github.com/ImoogDi/)
 * @brief   declaration-file for 'LScopeSetHW.c'.
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

#ifndef _LSCOPESETHW_h_
#define _LSCOPESETHW_h_

/* Enable c-linkage */
#if defined (__cplusplus)
  extern "C" {
#endif

#include <avr/io.h>
#include <assert.h>
#include <util/atomic.h>

#define OFFSETPIN_OUT_CHAN1 3
#define OFFSETPIN_OUT_CHAN2 5
#define OFFSET_5_0_VOLT  255
#define OFFSET_2_5_VOLT  127
#define OFFSET_1_25_VOLT  63

void hw_init(void);
void set_amplifier(const channel_nr_t echannel_nr);
void set_trigger_mode(const channel_nr_t eChannel_nr);
void set_trigger_level(void);

#if defined (__cplusplus)
} //extern "C"
#endif


#endif // end #ifndef _LSCOPESETHW_h_
