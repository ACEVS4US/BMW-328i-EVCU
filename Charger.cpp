/*
 * Charger.cpp
 *
 * Parent class for all battery management/monitoring systems
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

#include "Charger.h"
 
Charger::Charger() : Device() 
{
  
}

Charger::~Charger() 
{
}

DeviceType Charger::getType() {
  return (DEVICE_CHARGER);
}

void Charger::handleTick() {
}

void Charger::setup() {

        loadConfiguration();
        ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();
        //prefsHandler->read(EEMC_KILOWATTHRS, &kiloWattHours); //retrieve kilowatt hours from EEPROM
        //nominalVolts=config->nominalVolt;
        Device::setup();


}

/*
 * Load configuration data from EEPROM.
 *
 * If not available or the checksum is invalid, default values are chosen.
 */
void Charger::loadConfiguration() {
  ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

  if(!config) { // as lowest sub-class make sure we have a config object
    config = new ChargerConfiguration();
    setConfiguration(config);
  }

  Device::loadConfiguration(); // call parent

  if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
    prefsHandler->read(EECH_CHARGER_PON_OUT, &config->PON); //The Digital output to control the charger enable
    prefsHandler->read(EECH_CHARGER_PSU_OUT, &config->PowerSteeringOut); //The Digital output to control the power steering
    prefsHandler->read(EECH_CHARGER_COOLING_PUMP_OUT, &config->CoolingSystemOut);  //the digital output for the cooling pump
    prefsHandler->read(EECH_CHARGER_PROXIMITY, &config->Proximity);  //analog input to read the proximity voltage.
    prefsHandler->read(EECH_CHARGER_PROXIMITY_THRESHOLD_VOLTAGE_ON, &config->ProximityDetectVoltage);  //the level of voltage for the proximity circuit to be active
    prefsHandler->read(EECH_CHARGER_PROXIMITY_THRESHOLD_VOLTAGE_OFF, &config->ProximityAbsentVoltage);  //the level of voltage for the proximity circuit to be inactive
  }else { //checksum invalid. Reinitialize values.
   
    config->PON = 255; //not used by default
    config->PowerSteeringOut = 255;  //not used by default
    config->CoolingSystemOut = 255;  //not used by default
    config->Proximity = 255;   //not used by default
    config->ProximityDetectVoltage = 2500;
    config->ProximityAbsentVoltage = 2500;
  }
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void Charger::saveConfiguration() {
  ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

  Device::saveConfiguration(); // call parent

  prefsHandler->write(EECH_CHARGER_PON_OUT, config->PON);
  prefsHandler->write(EECH_CHARGER_PSU_OUT, config->PowerSteeringOut);
  prefsHandler->write(EECH_CHARGER_COOLING_PUMP_OUT, config->CoolingSystemOut);
  prefsHandler->write(EECH_CHARGER_PROXIMITY, config->Proximity);
  prefsHandler->write(EECH_CHARGER_PROXIMITY_THRESHOLD_VOLTAGE_ON, config->ProximityDetectVoltage);
  prefsHandler->write(EECH_CHARGER_PROXIMITY_THRESHOLD_VOLTAGE_OFF, config->ProximityAbsentVoltage);

  prefsHandler->saveChecksum();

  Logger::console("Charger configuration saved");
}

void Charger::processStatus(uint8_t data[]){
  
}

void Charger::processErrors(uint8_t data[]){
}

void Charger::processTemperature(uint8_t data[]){
}

