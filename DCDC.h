/*
   DCDC.h

   Parent class for all motor controllers.

  Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

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

#ifndef DCDC_H_
#define DCDC_H_

#include <Arduino.h>
#include "config.h"
#include "Device.h"
//#include "Throttle.h"
#include "DeviceManager.h"
#include "sys_io.h"



class DCDCConfiguration : public DeviceConfiguration {
  public:
    uint8_t lvoutputSet; 
    uint8_t dcdcEnable;



};

class DCDC: public Device {

  public:


    DCDC();
    DeviceType getType();
    void setup();
    void handleTick();
    uint32_t getTickInterval();
    void loadConfiguration();
    void saveConfiguration();
    void coolingcheck();
    bool isReady();
    bool isRunning();
    bool isFaulted();
    bool isWarning();
    int16_t getLVVoltage();
    int16_t getLVCurrent();
    int16_t getHVVoltage();
    int16_t getHVCurrent();
    int16_t getOutputPower();
    int16_t getOutputPowerLoss();
    int16_t getEfficiency();
    int16_t getTemperature();
    void Enable();



  protected:
    bool ready; // indicates if the controller is ready to enable the power stage
    bool running; // indicates if the power stage of the inverter is operative
    bool faulted; // indicates a error condition is present in the controller
    bool warning; // indicates a warning condition is present in the controller
    uint8_t outputpower;           // power in 1/10th of a watt
    uint8_t outputpowerloss;       // power in 1/10th of a watt
    uint8_t efficiency;            // percentage in 1/10th of a percent
    uint8_t temperature;           // temperature in 1/10th of a degree
    uint8_t dcdcsetpt;
    bool allow_voltage_set;

};

#endif
