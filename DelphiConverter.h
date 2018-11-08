/*
   DmocMotorController.h

   Note that the dmoc needs to have some form of input for gear selector (drive/neutral/reverse)

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

#ifndef DelphiConverter_H_
#define DelphiConverter_H_

#include <Arduino.h>
#include "config.h"
#include "DCDC.h"
#include "sys_io.h"
#include "TickHandler.h"
#include "CanHandler.h"

/*
   Class for DMOC specific configuration parameters
*/
class DelphiConverterConfiguration : public DCDCConfiguration {
  public:
};

class DelphiConverter: public DCDC, CanObserver {

  public:
    virtual void handleTick();
    virtual void handleCanFrame(CAN_FRAME *frame);
    virtual void setup();
    
    DelphiConverter();
    DeviceId getId();
    uint32_t getTickInterval();
    virtual void loadConfiguration();
    virtual void saveConfiguration();

  private:



    byte online; //counter for whether Delphi appears to be operating
    void timestamp();
    void setVoltage();
    void sendCmd1();
    //fields for message ID 0x1D5 (status messages)
    uint8_t DCDCLVVlt;
    uint8_t DCDCLVCur;
    bool DCDCVltCondMet;
    uint8_t DCDCDTCStat;
    uint8_t DCDCDTCldx;
    uint8_t DCDCHVCur;
    uint8_t DCDCHVCurVld;
    bool DCDCMstrFlt;

    //fields for message ID 0x48A (status messages)
    bool DCDCHVLoFlt;
    uint8_t DCDCPwrLoss;
    uint8_t DCDC14VCirctTmp;
    uint8_t DCDCHeatPlateTmp;

    //fields for message ID 0x1D7 (setable fields)
    uint8_t DCDCBuckEnblReq;
    uint8_t DCDCVNCEEnbl;
    uint8_t DCDCVltSetptReq;
    uint8_t DCDCBoostEnblReq;

    int milliseconds;
    int seconds;
    int minutes;
    int hours ;
    

};

#endif /* DelphiConverter_H_ */

