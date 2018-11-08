/*
   DelphiConverter.cpp

   Interface to the Delphi DC DC - Handles sending of commands and reception of status frames

  Copyright (c) 2013 Chris Young

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



#include "DelphiConverter.h"



DelphiConverter::DelphiConverter() : DCDC() {
  prefsHandler = new PrefHandler(DELPHIDCDC);


  online = 0;
  commonName = "Delphi DC DC Converter";
}

void DelphiConverter::setup() {
  TickHandler::getInstance()->detach(this);
  Logger::info("add device: DELPHIDCDC (id:%X, %X)", DELPHIDCDC, this);
  loadConfiguration();
  DCDC::setup(); // run the parent class version of this function

  // register ourselves as observer of 0x1D5, 0x1D7 and 0x48A can frames
  CanHandler::getInstanceCar()->attach(this, 0x1D5, 0x7ff, false);
  CanHandler::getInstanceCar()->attach(this, 0x1D7, 0x7ff, false);
  CanHandler::getInstanceCar()->attach(this, 0x48A, 0x7ff, false);

  running = false;

  allow_voltage_set = false;

  TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_DCDC);
}

/*
  Finally, the firmware actually processes some of the status messages from the DmocMotorController
  However, currently the alive and checksum bytes aren't checked for validity.
  To be valid a frame has to have a different alive value than the last value we saw
  and also the checksum must match the one we calculate. Right now we'll just assume
  everything has gone according to plan.
*/

void DelphiConverter::handleCanFrame(CAN_FRAME *frame) {


  online = 1; //if a frame got to here then it passed the filter and must have been from the DMOC

  Logger::debug("Delphi Converter CAN received: %X  %X  %X  %X  %X  %X  %X  %X  %X", frame->id, frame->data.bytes[0] , frame->data.bytes[1], frame->data.bytes[2], frame->data.bytes[3], frame->data.bytes[4], frame->data.bytes[5], frame->data.bytes[6], frame->data.bytes[7]);


  switch (frame->id) {
    case 0x1D5: //Measurement Status

      DCDCLVCur = 2 * (frame->data.bytes[1] & 0x7F);

      if (frame->data.bytes[3] & 0x80 == 0x80) DCDCVltCondMet = true;
      else DCDCVltCondMet = false;

      DCDCLVVlt = frame->data.bytes[4];     //255 scale to span 0 to 20 for correct reading.
      DCDCHVCur = frame->data.bytes[6];     //255 scale to span 0 to 25.5

      if (frame->data.bytes[7] & 0x80 == 0x80) DCDCMstrFlt = true;
      else DCDCMstrFlt = false;

      break;
    case 0x48A:
      DCDCHeatPlateTmp = frame->data.bytes[2];
      DCDC14VCirctTmp = frame->data.bytes[3];
      DCDCPwrLoss = frame->data.bytes[5];

      if (frame->data.bytes[7] & 0x80 == 0x80) DCDCHVLoFlt = true;
      else DCDCHVLoFlt = false;
  }
  Logger::debug("Low Voltage Current Output: %X A\n", DCDCLVCur);
  Logger::debug("Low Voltage Voltage Output (x0.078431: %X V\n", DCDCLVVlt);
}

/*Do note that the DMOC expects all three command frames and it expect them to happen at least twice a second. So, probably it'd be ok to essentially
  rotate through all of them, one per tick. That gives us a time frame of 30ms for each command frame. That should be plenty fast.
*/
void DelphiConverter::handleTick() {

  DCDC::handleTick(); //kick the ball up to papa


  //running=online;
  // online=0;//This flag will be set to 1 by received frames.
  sendCmd1();  //This actually sets the voltage and starts the DCDC converter




}



//Commanded setpoint voltage
void DelphiConverter::sendCmd1() {

  if (allow_voltage_set) {
    DelphiConverterConfiguration *config = (DelphiConverterConfiguration *)getConfiguration();
    DCDCVltSetptReq = config->lvoutputSet;
    //Logger::console("commanded %i", DCDCVltSetptReq);
    CAN_FRAME output;
    output.length = 8;
    output.id = 0x1D7;
    output.extended = 0; //standard frame
    output.rtr = 0;


    output.data.bytes[0] = 128;  //0-7
    output.data.bytes[1] = DCDCVltSetptReq;  //8-15
    output.data.bytes[2] = 0; //16-23
    output.data.bytes[3] = 0; //24-31
    output.data.bytes[4] = 0; //32-39
    output.data.bytes[5] = 0; //40-47
    output.data.bytes[6] = 0; //48-55
    output.data.bytes[7] = 0; //56-63

    CanHandler::getInstanceCar()->sendFrame(output);
  }
}



DeviceId DelphiConverter::getId() {
  return (DELPHIDCDC);
}

uint32_t DelphiConverter::getTickInterval()
{
  return CFG_TICK_INTERVAL_DCDC;
}

void DelphiConverter::loadConfiguration() {
  DelphiConverterConfiguration *config = (DelphiConverterConfiguration *)getConfiguration();

  if (!config) {
    config = new DelphiConverterConfiguration();
    setConfiguration(config);
  }

  DCDC::loadConfiguration(); // call parent

}

void DelphiConverter::saveConfiguration() {
  DCDC::saveConfiguration();
}

void DelphiConverter::timestamp()
{
  milliseconds = (int) (millis() / 1) % 1000 ;
  seconds = (int) (millis() / 1000) % 60 ;
  minutes = (int) ((millis() / (1000 * 60)) % 60);
  hours   = (int) ((millis() / (1000 * 60 * 60)) % 24);
  // char buffer[9];
  //sprintf(buffer,"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
  // Serial<<buffer<<"\n";


}
