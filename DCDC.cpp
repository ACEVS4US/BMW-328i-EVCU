/*
 * MotorController.cpp
 *
 * Parent class for all motor controllers.
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
 
#include "DCDC.h"
 
DCDC::DCDC() : Device() {
	ready = false;
	running = false;
	faulted = false;
	warning = false;

}

void DCDC::Enable() {

  DCDCConfiguration *config = (DCDCConfiguration *)getConfiguration();
  allow_voltage_set = true;
  setOutput(config->dcdcEnable, 1);   //set the enable line for the DCDC converter
}

DeviceType DCDC::getType() {
	return (DEVICE_DCDC);
}

void DCDC::setup() {
  
	      DCDCConfiguration *config = (DCDCConfiguration *)getConfiguration();
        Device::setup();
   
}
void DCDC::handleTick() {

      DCDCConfiguration *config = (DCDCConfiguration *)getConfiguration();
}
uint32_t DCDC::getTickInterval() {
        return CFG_TICK_INTERVAL_DCDC;
}

void DCDC::loadConfiguration() {
	DCDCConfiguration *config = (DCDCConfiguration *)getConfiguration();
	Device::loadConfiguration(); // call parent

#ifdef USE_HARD_CODED
	if (false) {
#else
	if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
#endif
		prefsHandler->read(EEDC_LV_SET_VLTG, &config->lvoutputSet);
    prefsHandler->read(EEDC_ENABLE, &config->dcdcEnable);
   
    
	}
	else { //checksum invalid. Reinitialize values and store to EEPROM
		dcdcsetpt = 178;
    config->lvoutputSet=dcdcsetpt;
    config->dcdcEnable = 255;


	}
}

void DCDC::saveConfiguration() {
	DCDCConfiguration *config = (DCDCConfiguration *)getConfiguration();
	Device::saveConfiguration(); // call parent
  prefsHandler->write(EEDC_LV_SET_VLTG, config->lvoutputSet);
  prefsHandler->write(EEDC_ENABLE, config->dcdcEnable);
	prefsHandler->saveChecksum();
	loadConfiguration();
}

//This routine is used to set an optional cooling fan output to on if the current temperature 
//exceeds a specified value.  Annunciators are set on website to indicate status.
void DCDC::coolingcheck()
{	
}


bool DCDC::isReady() {
	return ready;
}

bool DCDC::isRunning() {
	return running;
}

bool DCDC::isFaulted() {
	return faulted;
}

bool DCDC::isWarning() {
	return warning;
}

	
int16_t DCDC::getLVVoltage(){
}

int16_t DCDC::getLVCurrent(){
}
int16_t DCDC::getHVVoltage(){
}
int16_t DCDC::getHVCurrent(){
}
int16_t DCDC::getOutputPower(){
}
int16_t DCDC::getOutputPowerLoss(){
}
int16_t DCDC::getEfficiency(){
}
int16_t DCDC::getTemperature(){
} 







