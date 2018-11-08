/*
 * BatteryManager.h
 *
 * Parent class for battery management / monitoring systems
 *
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
 
#ifndef CHARGER_H_
#define CHARGER_H_

#include <Arduino.h>
#include "config.h"
#include "Device.h"

class ChargerConfiguration : public DeviceConfiguration {
public:

    uint8_t Proximity;         //analog input to read the proximity voltage.
    uint8_t PowerSteeringOut;  //digital pin to activate power steering pump.
    uint8_t CoolingSystemOut;  //digital pin to activate cooling system.
    uint8_t PON;               //digital output for charger PON.
    uint32_t ProximityDetectVoltage = 2500;
    uint32_t ProximityAbsentVoltage = 2500;
        
};

class Charger : public Device {
public:
  Charger();
  ~Charger();
  
  DeviceType getType();
  void setup();
  void handleTick();
  
  //a bunch of boolean functions. Derived classes must implment
  //these functions to tell everyone else what they support
  virtual void processStatus(uint8_t data[]);
  virtual void processErrors(uint8_t data[]);
  virtual void processTemperature(uint8_t data[]);

  void loadConfiguration();
  void saveConfiguration();
  
protected:
  uint16_t charger_cutoff_voltage;
  uint16_t max_charging_current;
  

private:
};

#endif
