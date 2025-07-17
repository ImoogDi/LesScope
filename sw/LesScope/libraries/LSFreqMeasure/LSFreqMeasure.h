#ifndef LSFreqMeasure_h
#define LSFreqMeasure_h

#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////
/////// required modification to use FreqMeasure-lib with less free memory /////
//                                                                            //
// original library-license see header in file: LSFreqMeasure.cpp             //
/* FreqMeasure Library, for measuring relatively low frequencies
 * http://www.pjrc.com/teensy/td_libs_FreqMeasure.html
 * Copyright (c) 2011 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>
 */
// @file    LSFreqMeasure.h                                                   //
// @author  ImoogDi (https://github.com/ImoogDi/)                             //
// @version 0.1                                                               //
// @date    2025-07-15                                                        //
////////////////////////////////////////////////////////////////////////////////

#if defined(MODIFIED_FREQMEASURE_BUFFER_LEN) && MODIFIED_FREQMEASURE_BUFFER_LEN < 12
  #ifdef FREQMEASURE_BUFFER_LEN
    #undef  FREQMEASURE_BUFFER_LEN
  #endif
  #define FREQMEASURE_BUFFER_LEN  MODIFIED_FREQMEASURE_BUFFER_LEN
#else
  #define FREQMEASURE_BUFFER_LEN 12
#endif

class FreqMeasureClass {
public:
  static void begin(void);
  static uint8_t available(void);
  static uint32_t read(void);
  static float countToFrequency(uint32_t count);
  static void end(void);
};

extern FreqMeasureClass FreqMeasure;

#endif

