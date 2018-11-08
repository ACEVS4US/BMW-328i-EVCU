/*
 * VacuumPump.cpp
 *
 * Parent class for all throttle controllers

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

#include "VacuumPump.h"

/*
 * Constructor
 */
VacuumPump::VacuumPump() : Device() {
  prefsHandler = new PrefHandler(VACUUM_PUMP);
  commonName = "vacuum pump";
}

void VacuumPump::setup(){
  TickHandler::getInstance()->detach(this);
  Logger::info("add device: VACUUM PUMP (id:%X, %X)", VACUUM_PUMP, this);
  Device::setup();
  VacuumPumpConfiguration *config = (VacuumPumpConfiguration *) getConfiguration();
  loadConfiguration();
  TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_VAC_PUMP);
}


/*
 * Controls the main flow of throttle data acquisiton, validation and mapping to
 * user defined behaviour.
 *
 * Get's called by the sub-class which is triggered by the tick handler
 */
void VacuumPump::handleTick() {
  Device::handleTick();
  VacuumPumpConfiguration *config = (VacuumPumpConfiguration *) getConfiguration();

  uint16_t l = getLevel();

  //Logger::info("vacuum voltage: %i mV", l);
  if(l<config->light_off_threshold){  //if the vacuum level is lower than the light off level, then turn off the light
    setOutput(config->vacLight,0);
  }
  else if(l>config->light_on_threshold){
    setOutput(config->vacLight,1);    //if the vacuum level is higher than the light on level, then turn on the light
  }
  
}


DeviceType VacuumPump::getType() {
  return (DEVICE_VACUUM_PUMP);
}

DeviceId VacuumPump::getId(){
  return VACUUM_PUMP;
}

/*
 * Returns the currently calculated brake vacuum level.
 */
uint16_t VacuumPump::getLevel() {
  VacuumPumpConfiguration *config = (VacuumPumpConfiguration *) getConfiguration();
  level = getAnalog(config->VacuumInPin);

  return map(level, 0, 4096, (int32_t) 0, (int32_t) 5000);
  
}

/*
 * Load the config parameters which are required by all throttles
 */
void VacuumPump::loadConfiguration() {
  VacuumPumpConfiguration *config = (VacuumPumpConfiguration *) getConfiguration();

  if (!config) {
    config = new VacuumPumpConfiguration();
    setConfiguration(config);
  }
  Device::loadConfiguration(); // call parent

  if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
    prefsHandler->read(EEBV_VACUUM_LIGHTON_THRESHOLD, &config->light_on_threshold);
    prefsHandler->read(EEBV_VACUUM_LIGHTOFF_THRESHOLD, &config->light_off_threshold);
    prefsHandler->read(EEBV_VACUUM_OUT, &config->vacLight);
    prefsHandler->read(EEBV_VACUUM_IN, &config->VacuumInPin);

  } else { //checksum invalid. Reinitialize values.
   
    config->light_on_threshold = 65535; //light will not turn on by default
    config->light_off_threshold = 65535;
    config->VacuumInPin = 255;  //not used by default
    config->vacLight = 255;   //not used by default
  }
  lont = config->light_on_threshold;
  lofft = config->light_off_threshold;
  adc_pin = config->VacuumInPin;
  vac_out = config->vacLight;
}

/*
 * Store the current configuration to EEPROM
 */
void VacuumPump::saveConfiguration() {
  VacuumPumpConfiguration *config = (VacuumPumpConfiguration *) getConfiguration();

  Device::saveConfiguration(); // call parent

  prefsHandler->write(EEBV_VACUUM_LIGHTON_THRESHOLD, config->light_on_threshold);
  prefsHandler->write(EEBV_VACUUM_LIGHTOFF_THRESHOLD, config->light_off_threshold);
  prefsHandler->write(EEBV_VACUUM_OUT, config->vacLight);
  prefsHandler->write(EEBV_VACUUM_IN, config->VacuumInPin);

  prefsHandler->saveChecksum();

  Logger::console("Vacuum pump configuration saved");
}
