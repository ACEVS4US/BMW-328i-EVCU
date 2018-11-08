/*
   BMW328BatteryManager.cpp

   Interface to the BMS which is within the Think City battery packs

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

#include "BMW328BatteryManager.h"

BMW328BatteryManager::BMW328BatteryManager() : BatteryManager() {
  prefsHandler = new PrefHandler(BMS_BMW328);
  allowCharge = false;
  allowDischarge = false;
  commonName = "BMW328 BMS";
  online = 0;
}

void BMW328BatteryManager::setup() {
  TickHandler::getInstance()->detach(this);

  Logger::info("add device: BMW328 BMS (id: %X, %X)", BMS_BMW328, this);

  BatteryManager::setup(); // run the parent class version of this function

  //Relevant BMS messages are 0x300 - 0x30F
  CanHandler::getInstanceCar()->attach(this, 0x300, 0x7f0, false);
 
  TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_BMS_BMW328);
}

/*For all multibyte integers the format is MSB first, LSB last
*/
void BMW328BatteryManager::handleCanFrame(CAN_FRAME *frame) {
  //Logger::info("processing can frame for bms - frameID: %X", frame->id);
  switch (frame->id) {
    case 0x300: //Start up message
      //we're not really interested in much here except whether init worked.
      if ((frame->data.bytes[6] & 1) == 0)  //there was an initialization error!
      {
        //set fault condition here
      }
      else {
        online = 1;
      }
      break;
    case 0x301: //System Cell data
      //first two bytes = min cell, next two max cell, next two average cell
      //readings in microvolts
      lowestCellV = ((frame->data.bytes[0] * 256) + frame->data.bytes[1]);
      highestCellV = ((frame->data.bytes[2] * 256) + frame->data.bytes[3]);
      averageCellV = ((frame->data.bytes[4] * 256) + frame->data.bytes[5]);
      break;
    case 0x302: //System pack data
      //first two bytes = pack volts (tenths of a volt),next two pack current (tenths of an amp), last byte state of charge (%)
      packVoltage = ((frame->data.bytes[0] * 256) + frame->data.bytes[1]);
      packCurrent = ((frame->data.bytes[2] * 256) + frame->data.bytes[3]);
      SOC = (frame->data.bytes[4]);
      break;
    case 0x303: //errors frame
      if ((frame->data.bytes[0] & 1) == 1) //Byte 0 bit 0 = general error
      {
        //raise a fault
      }
      if ((frame->data.bytes[0] & 2) == 2) //Byte 0 bit 1 = persistent low cell
      {
        //over discharge condition - critical fault - open a contactor
        critical_fault = true;
      }
      if ((frame->data.bytes[0] & 4) == 4) //Byte 0 bit 2 = persistent high cell
      {
        //over charge condition -critical fault - open a contactor
        critical_fault = true;
      }
      if ((frame->data.bytes[0] & 8) == 8) //Byte 0 bit 3 = high temperature
      {
        //high temperature condition -critical fault - open a contactor
        critical_fault = true;
      }
      if ((frame->data.bytes[0] & 16) == 16) //Byte 0 bit 4 = low temperature
      {
        //low temperature condition - no charging alloweed.

      }
      if ((frame->data.bytes[0] & 32) == 32) //Byte 2 bit 0 = general isolation error
      {
        //show warning light
      }
      break;
    case 0x304: //System Data 2
      
      break;
    case 0x305: //System Data 3
      
      break;
    case 0x306: //System Data 4
      
      break;
    case 0x307: //System Data 5
      break;
  }
}

void BMW328BatteryManager::handleTick() {
  BatteryManager::handleTick(); //kick the ball up to papa

  //sendKeepAlive();

}

//Contactors in pack will close if we sent these two frames with all zeros.
void BMW328BatteryManager::sendKeepAlive()
{
  CAN_FRAME output;
  output.length = 3;
  output.id = 0x310;
  output.extended = 0; //standard frame
  output.rtr = 0;
  for (int i = 0; i < 8; i++) output.data.bytes[i] = 0;
  CanHandler::getInstanceCar()->sendFrame(output);

  output.id = 0x311;
  output.length = 2;
  CanHandler::getInstanceCar()->sendFrame(output);
}

DeviceId BMW328BatteryManager::getId()
{
  return (BMS_BMW328);
}

bool BMW328BatteryManager::hasPackVoltage()
{
  return true;
}

bool BMW328BatteryManager::hasPackCurrent()
{
  return true;
}

bool BMW328BatteryManager::hasTemperatures()
{
  return true;
}

bool BMW328BatteryManager::isChargeOK()
{
  return allowCharge;
}

bool BMW328BatteryManager::isDischargeOK()
{
  return allowDischarge;
}

