/*
   VacuumPump.h

   class for brake vacuum monitoring

  Copyright (c) 2018 Chris Young

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef VACUUMPUMP_H_
#define VACUUMPUMP_H_
#include <Arduino.h>
#include "config.h"
#include "Device.h"
#include "sys_io.h"

class VacuumPumpConfiguration : public DeviceConfiguration {
  public:
       uint16_t light_on_threshold; //volts
       uint16_t light_off_threshold;  //volts
       uint8_t vacLight; //the digital output for the brake vacuum warning light
       uint8_t VacuumInPin;  //the analog input for the brake vacuum pressure 
       
};

class VacuumPump : public Device {
  public:
    VacuumPump();
    ~VacuumPump();
    DeviceType getType();
    DeviceId getId();
    void setup();
    
    void handleTick();
    uint16_t getLevel();
    void loadConfiguration();
    void saveConfiguration();

  private:
  uint16_t lont;
  uint16_t lofft;
  uint16_t level;
  uint8_t adc_pin;
  uint8_t vac_out;
};

#endif
