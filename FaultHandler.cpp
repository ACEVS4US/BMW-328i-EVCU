/*
 * FaultHandler.cpp
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

#include "FaultHandler.h"
#include "eeprom_layout.h"

  FaultHandler::FaultHandler()
  {
  }

  void FaultHandler::setup() 
  {
	  TickHandler::getInstance()->detach(this);

	  Logger::info("Initializing Fault Handler", CODAUQM, this);

	  loadFromEEPROM();

	  //Use the heartbeat interval because it's slow and already exists so we can piggyback on the interrupt
	  //so as to not create more timers than necessary.
	  TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_HEARTBEAT);
  }


  //Every tick update the global time and save it to EEPROM (delayed saving)
  void FaultHandler::handleTick() 
  {
	  globalTime = baseTime + (millis() / 100);
	  memCache->Write(EE_FAULT_LOG + EEFAULT_RUNTIME, globalTime);
  }

  uint16_t FaultHandler::raiseFault(uint16_t device, uint16_t code) 
  {
	  bool incPtr = false;
	  globalTime = baseTime + (millis() / 100);

	  uint16_t tempIdx = faultWritePointer;
	  if (tempIdx > 0) tempIdx--;
	  else tempIdx = CFG_FAULT_HISTORY_SIZE - 1;

	  //if this is the same as the previously registered fault then just update the time
	  if (faultList[tempIdx].device == device && faultList[tempIdx].faultCode == code) 
	  {
		  faultList[tempIdx].timeStamp = globalTime;
	  } 
	  else 
	  {
		  faultList[faultWritePointer].timeStamp = globalTime;
		  faultList[faultWritePointer].ack = false;
		  faultList[faultWritePointer].device = device;
		  faultList[faultWritePointer].faultCode = code;
		  faultList[faultWritePointer].ongoing = false;
		  incPtr = true;
	  }

	  memCache->Write(EE_FAULT_LOG + EEFAULT_FAULTS_START + sizeof(FAULT) * faultWritePointer, &faultList[faultWritePointer], sizeof(FAULT));
	  //Cause the memory caching system to immediately write the page
	  memCache->InvalidateAddress(EE_FAULT_LOG + EEFAULT_FAULTS_START + sizeof(FAULT)* faultWritePointer);

	  if (incPtr) faultWritePointer = (faultWritePointer + 1) % CFG_FAULT_HISTORY_SIZE;

	  memCache->Write(EE_FAULT_LOG + EEFAULT_WRITEPTR , faultWritePointer);
	  //Cause the page to be immediately fully aged so that it is written very soon.
	  memCache->AgeFullyAddress(EE_FAULT_LOG + EEFAULT_WRITEPTR);	  
  }

  uint16_t FaultHandler::getFaultCount() 
  {
	  int count = 0;
	  for (int i = 0; i < CFG_FAULT_HISTORY_SIZE; i++) 
	  {
		  if (faultList[i].ack == false) 
		  {
			  count++;
		  }
	  }
	  return count;
  }

  //the fault handler isn't a device per se and uses more memory than a device would normally be allocated so
  //it does not use PrefHandler
  void FaultHandler::loadFromEEPROM() 
  {
	  uint8_t validByte;
	  memCache->Read(EE_FAULT_LOG, &validByte);
	  if (validByte == 0xB2) //magic byte value for a valid fault cache
	  {
		  memCache->Read(EE_FAULT_LOG + EEFAULT_READPTR, &faultReadPointer);
		  memCache->Read(EE_FAULT_LOG + EEFAULT_WRITEPTR, &faultWritePointer);
		  memCache->Read(EE_FAULT_LOG + EEFAULT_RUNTIME, &globalTime);
		  baseTime = globalTime;
		  for (int i = 0; i < CFG_FAULT_HISTORY_SIZE; i++) 
		  {
			  memCache->Read(EE_FAULT_LOG + EEFAULT_FAULTS_START + sizeof(FAULT) * i, &faultList[i], sizeof(FAULT));
		  }
	  }
	  else //reinitialize the fault cache storage
	  {
		  validByte = 0xB2;
		  memCache->Write(EE_FAULT_LOG, validByte);
		  memCache->Write(EE_FAULT_LOG + EEFAULT_READPTR, (uint16_t)0);
		  memCache->Write(EE_FAULT_LOG + EEFAULT_WRITEPTR, (uint16_t)0);
		  globalTime = baseTime = millis() / 100;
		  memCache->Write(EE_FAULT_LOG + EEFAULT_RUNTIME, globalTime);

		  FAULT tempFault;
		  tempFault.ack = true;
		  tempFault.device = 0xFFFF;
		  tempFault.faultCode = 0xFFFF;
		  tempFault.ongoing = false;
		  tempFault.timeStamp = 0;
		  for (int i = 0; i < CFG_FAULT_HISTORY_SIZE; i++) 
		  {
			  faultList[i] = tempFault;
		  }
		  saveToEEPROM();
	  }
  }

  void FaultHandler::saveToEEPROM() 
  {
	memCache->Write(EE_FAULT_LOG + EEFAULT_READPTR, faultReadPointer);
	memCache->Write(EE_FAULT_LOG + EEFAULT_WRITEPTR, faultWritePointer);
	memCache->Write(EE_FAULT_LOG + EEFAULT_RUNTIME, globalTime);
	for (int i = 0; i < CFG_FAULT_HISTORY_SIZE; i++) 
	{
		memCache->Write(EE_FAULT_LOG + EEFAULT_FAULTS_START + sizeof(FAULT) * i, &faultList[i], sizeof(FAULT));
	}
  }


  bool FaultHandler::getNextFault(FAULT *fault)
  {
	uint16_t j;
	for (int i = 0; i < CFG_FAULT_HISTORY_SIZE; i++) 
	{
		j = faultReadPointer + i;
		if (faultList[j].ack == false) {
			fault = &faultList[j];
			return true;
		}
	}
	return false;
  }

  bool FaultHandler::getFault(uint16_t fault, FAULT *outFault)
  {
	  if (fault > 0 && fault < CFG_FAULT_HISTORY_SIZE) {
		  outFault = &faultList[fault];
		  return true;
	  }
	  return false;
  }
  
  uint16_t FaultHandler::setFaultACK(uint16_t fault)
  {
	  if (fault > 0 && fault < CFG_FAULT_HISTORY_SIZE) 
	  {
		  faultList[fault].ack = 1;
	  }
  }

  uint16_t FaultHandler::setFaultOngoing(uint16_t fault, bool ongoing)
  {
	  if (fault > 0 && fault < CFG_FAULT_HISTORY_SIZE) 
	  {
		  faultList[fault].ongoing = ongoing;
	  }
  }

  FaultHandler faultHandler;
