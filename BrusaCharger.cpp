/*
 * BrusaCharger.cpp
 *
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

#include "BrusaCharger.h"



/*
 * Constructor
 */
BrusaCharger::BrusaCharger(){
	prefsHandler = new PrefHandler(BRUSA_NLG513);
	
	commonName = "Brusa NLG513 Charger";
}

/*
 * Setup the device if it is enabled in configuration.
 */
void BrusaCharger::setup() {
	TickHandler::getInstance()->detach(this);

	Logger::info("add device: Brusa NLG513 (id: %X, %X)", BRUSA_NLG513, this);

	//loadConfiguration();
	Charger::setup(); // run the parent class version of this function

	// register ourselves as observer of 0x610-0x618, 0x718 and 0x71A can frames
	//CanHandler::getInstanceCar()->attach(this, 0x610, 0x7f0, false);
	//CanHandler::getInstanceCar()->attach(this, 0x710, 0x7f0, false);

	TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_BRUSA_CHARGER);
  proximity_voltage = 5000;
  charger_cutoff_voltage = 400;
  max_charging_current = 16;
  print_counter = 0;
}

/*
 * Process event from the tick handler.
 *
 * The super-class requests desired levels from the throttle and
 * brake and decides which one to apply.
 */
void BrusaCharger::handleTick() {

  ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

	Charger::handleTick(); // call parent
	//tickCounter++;

  MotorController* motorController = (MotorController*) DeviceManager::getInstance()->getMotorController();
  
  //check the proximity of the charger cable every 40ms
  if(CheckProximity()){
    //the charge cable is connected, so turn off the power steering and the cooling system
    setOutput(config->PowerSteeringOut,0);  //disable power steering pump.
    setOutput(config->CoolingSystemOut,0);  //disable cooling system pump;
    setOutput(config->PON,1);               //enable the charging system;
    motorController->setChargingStatus(true);
    motorController->setOpState((MotorController::OperationState) 0);         //disabled
  }
  else{
    //the charge cable is not connected, so turn so okay to turn on the power steering and the cooling system. 
    setOutput(config->PowerSteeringOut,1);  //enable power steering pump.
    setOutput(config->CoolingSystemOut,1);  //ensable cooling system pump;
    setOutput(config->PON,0);               //disable the charging;
    motorController->setChargingStatus(false);
  }

    
     
	//sendControl();	
	//if (tickCounter > 10) {
		//sendControl2();	// send CTRL_2 every 100ms
		//sendLimits();	// send LIMIT every 100ms
		//tickCounter = 0;
	//}
}

/*
 * Send DMC_CTRL message to the motor controller.
 *
 * This message controls the power-stage in the controller, clears the error latch
 * in case errors were detected and requests the desired torque / speed.
 */
void BrusaCharger::sendControl() {
	ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();
	//prepareOutputFrame(CAN_ID_CONTROL);

	CanHandler::getInstanceEV()->sendFrame(outputFrame);
}

/*
 * Send DMC_CTRL2 message to the motor controller.
 *
 * This message controls the mechanical power limits for motor- and regen-mode.
 */
void BrusaCharger::sendControl2() {
	ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

  //prepareOutputFrame(CAN_ID_LIMIT);
	CanHandler::getInstanceEV()->sendFrame(outputFrame);
}

/*
 * Send DMC_LIM message to the motor controller.
 *
 * This message controls the electrical limits in the controller.
 */
void BrusaCharger::sendLimits() {
	ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

	//prepareOutputFrame(CAN_ID_LIMIT);
	//outputFrame.data.bytes[0] = (config->dcVoltLimitMotor & 0xFF00) >> 8;
	//outputFrame.data.bytes[1] = (config->dcVoltLimitMotor & 0x00FF);
	//outputFrame.data.bytes[2] = (config->dcVoltLimitRegen & 0xFF00) >> 8;
	//outputFrame.data.bytes[3] = (config->dcVoltLimitRegen & 0x00FF);
	//outputFrame.data.bytes[4] = (config->dcCurrentLimitMotor & 0xFF00) >> 8;
	//outputFrame.data.bytes[5] = (config->dcCurrentLimitMotor & 0x00FF);
	//outputFrame.data.bytes[6] = (config->dcCurrentLimitRegen & 0xFF00) >> 8;
	//outputFrame.data.bytes[7] = (config->dcCurrentLimitRegen & 0x00FF);

	CanHandler::getInstanceEV()->sendFrame(outputFrame);
}

/*
 * Prepare the CAN transmit frame.
 * Re-sets all parameters in the re-used frame.
 */
void BrusaCharger::prepareOutputFrame(uint32_t id) {
	outputFrame.length = 8;
	outputFrame.id = id;
	outputFrame.extended = 0;
	outputFrame.rtr = 0;

	outputFrame.data.bytes[1] = 0;
	outputFrame.data.bytes[2] = 0;
	outputFrame.data.bytes[3] = 0;
	outputFrame.data.bytes[4] = 0;
	outputFrame.data.bytes[5] = 0;
	outputFrame.data.bytes[6] = 0;
	outputFrame.data.bytes[7] = 0;
}

/*
 * Processes an event from the CanHandler.
 *
 * In case a CAN message was received which pass the masking and id filtering,
 * this method is called. Depending on the ID of the CAN message, the data of
 * the incoming message is processed.
 */
void BrusaCharger::handleCanFrame( CAN_FRAME *frame) {
	switch (frame->id) {
	case NLG5_ST:
		processStatus(frame->data.bytes);
		break;
  case NLG5_TEMP:
    processTemperature(frame->data.bytes);
    break;
	default:
		Logger::warn(BRUSA_NLG513, "received unknown frame id %X", frame->id);
	}
}



/*
 * Return the device id of this device
 */
DeviceId BrusaCharger::getId() {
	return BRUSA_NLG513;
}

/*
 * Expose the tick interval of this controller
 */
uint32_t BrusaCharger::getTickInterval() {
	return CFG_TICK_INTERVAL_BRUSA_CHARGER;
}

/*
 * Process a NLG5_ST message which was received from the charger.
 *
 * This message provides the general status of the charger
 */
void BrusaCharger::processStatus(uint8_t data[]) {
  
}

/*
 * Process a NLG5_TEMP message which was received from the charger.
 *
 * This message provides temperature of various parts of the charger
 */
void BrusaCharger::processTemperature(uint8_t data[]){
  
}

/*
 * Process a NLG5_ST message which was received from the charger.
 *
 * This message provides the general status of the charger
 */
void BrusaCharger::processErrors(uint8_t data[]) {
  
}

void BrusaCharger::DisablePS(){
  
}
void BrusaCharger::DisableHeating(){
    
}
void BrusaCharger::DisableDrive(){
  
}

/*
 * returns true if the proximity switch on the charging handle is open.  i.e. the charging plug is connected
 */
bool BrusaCharger::CheckProximity(){
  ChargerConfiguration *config = (ChargerConfiguration *)getConfiguration();

  print_counter++;

 
  uint32_t proximity_voltage = 0;  //default state 
  proximity_voltage = getAnalog(config->Proximity);
  
  map(proximity_voltage, 0, 4096, (int32_t) 0, (int32_t) 5000);
  if(print_counter==255){
    print_counter = 0;
    //Logger::info("Proximity Voltage: %i mV", proximity_voltage);
  }
  
  
  if(proximity_voltage > config->ProximityAbsentVoltage){
    //not plugged in
    cable_connected = false;
    return false;
  }
  else if(proximity_voltage < config->ProximityDetectVoltage){
    cable_connected = true;
    return true;
  }

  //can't determine
  else return false;
}



