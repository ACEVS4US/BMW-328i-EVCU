/*
   BrusaMotorController.h


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

#ifndef BRUSACHARGER_H_
#define BRUSACHARGER_H_

#include <Arduino.h>
#include "config.h"
#include "Device.h"
#include "DeviceManager.h"
//#include "sys_io.h"
//#include "TickHandler.h"

#include "Charger.h"
//#include "ichip_2128.h"
//#include "DeviceTypes.h"

#include "CanHandler.h"



//Can messages
#define NLG5_CTL 0x618
#define NLG5_DIAG_RX 0x718
#define NLG5_ST 0x610
#define NLG5_ACT_I 0x611
#define NLG5_ACT_II 0x612
#define NLG5_TEMP 0x613
#define NLG5_ERR 0x614
#define NLG5_DIAG_TX 0x71A

enum CTL {
  NLG5_C_C_EN   = 1 << 0,  //Rx --- 0x0001
  NLG5_C_C_EL   = 1 << 1,  //Rx ---  Cycle this value 0-1-0 to clear all latched charger errors
  NLG5_C_CP_V   = 1 << 2,  //Rx ---  Special signal to tell a charging station equipped with the Control pilot interface (SAE J1772) to switch on the
  //Rx ---  facility's ventilation (in order to get rid of battery gases like hydrogen, e.g.)
  NLG5_C_MR     = 1 << 3,  //Rx ---  Enables CP-State C to request mains voltage
  NLG5_MC_MAX   = 1 << 8,  //Rx ---  Maximum current to be drawn from the mains outlet, usually 16 A. Must always contain a valid value in order to keep on charging.
  NLG5_OV_COM   = 1 << 24, //Rx ---  Desired voltage to be output to the battery. Must always contain a valid value in order to keep on charging.
  NLG5_OC_COM   = 1 << 40  //Rx ---  Desired battery current. Must always contain a valid value in order to keep on charging.
};

enum DIAG_RX {
  LAD_D_ParamRq = 1 << 0   //Diag requested data from host
};

enum ST {
  NLG5_S_HE = 1 << 0,        // Tx --- Indicates if hardware enabled, i.e. a high or low signal is fed to the 'Power On' pin (pin3 of control connector)
  NLG5_S_ERR = 1 << 1,       // Tx --- An error has been detected, red LED is ON, no power is output
  NLG5_S_WAR = 1 << 2,       // Tx --- Warning condition on, i.e. charging power limited due to any limiting condition; red LED is blinking
  NLG5_S_FAN = 1 << 3,       // Tx --- Charger cooling fan is active
  NLG5_S_EUM = 1 << 4,       // Tx --- European mains input detected (230 V, 50 Hz)
  NLG5_S_UM_I = 1 << 5,      // Tx --- US mains level 1 (120 V AC / 60 Hz) detected
  NLG5_S_UM_II = 1 << 6,     // Tx --- US mains level 2 (240 V AC / 60 Hz) detected
  NLG5_S_CP_DT = 1 << 7,     // Tx --- Control pilot signal (SAE J1772) detected
  NLG5_S_BPD_I = 1 << 8,     // Tx --- Bypass detection bit 1; 00: no bypass, 01: DC bypass detected, 10: AC bypass in phase, 11: AC bypass not in phase
  NLG5_S_BPD_II = 1 << 9,    // Tx --- Bypass detection bit 2; 00: no bypass, 01: DC bypass detected, 10: AC bypass in phase, 11: AC bypass not in phase
  NLG5_S_L_OV = 1 << 10,     // Tx --- Output power limited by battery output voltage limit
  NLG5_S_L_OC = 1 << 11,     // Tx --- Output power limited by battery output current limit
  NLG5_S_L_MC = 1 << 12,     // Tx --- Output power limited by mains current limit
  NLG5_S_L_PI = 1 << 13,     // Tx --- Output power limited by analogue input 'power indicator' (PI) limit
  NLG5_S_L_CP = 1 << 14,     // Tx --- Output power limited by control pilot signal (SAE J1772)
  NLG5_S_L_PMAX = 1 << 15,   // Tx --- Output power limited by maximum power capability of NLG5
  NLG5_S_L_MC_MAX = 1 << 16, // Tx --- Output power limited by maximum mains current capability of NLG5
  NLG5_S_L_OC_MAX = 1 << 17, // Tx --- Output power limited by maximum output current capability of NLG5
  NLG5_S_L_MO_MAX = 1 << 18, // Tx --- Output power limited by maximum output voltage capability of NLG5
  NLG5_S_L_T_CPRIM = 1 << 19, // Tx --- Output power limited by temperature of primary capacitors
  NLG5_S_L_T_POW = 1 << 20,  // Tx --- Output power limited by temperature of power stage
  NLG5_S_L_T_DIO = 1 << 21,  // Tx --- Output power limited by temperature of diodes
  NLG5_S_L_T_TR = 1 << 22,   // Tx --- Output power limited by temperature of transformer
  NLG5_S_L_T_BATT = 1 << 23, // Tx --- Output power limited by battery temperature
  NLG5_S_AAC = 1 << 31       // Tx --- AUX 12 V Automatic Charging Active
};

enum ACT_I {
  NLG5_MC_ACT = 1 << 0,      // Tx --- Actual mains current
  NLG5_MV_ACT = 1 << 16,     // Tx --- Actual mains voltage
  NLG5_OV_ACT = 1 << 32,     // Tx --- Actual battery output voltage
  NLG5_OC_ACT = 1 << 48      // Tx --- Output current to the battery
};

enum ACT_II {
  NLG5_S_MC_M_CP  = 1 << 0,  // Tx --- Value of mains current limit, commanded by the control pilot (SAE J1772) signal
  NLG5_S_MC_M_PI  = 1 << 16, // Tx --- Value of mains current limit, commanded by the 'power indicator' (PI) analogue input signal
  NLG5_ABV        = 1 << 24  // Tx --- Aux battery voltage
};

enum TEMP {
  NLG5_P_TMP     = 1 << 0,    //Tx --- Power stage temperature
  NLG5_TMP_EXT1  = 1 << 16,   //Tx --- Value of external battery temperature sensor #1 (if connected and enabled)
  NLG5_TEMP_EXT2 = 1 << 32,   //Tx --- Value of external battery temperature sensor #2 (if connected and enabled)
  NLG5_TMP_EXT3  = 1 << 48   //Tx --- Value of external battery temperature sensor #3 (if connected and enabled)

};

enum ERR {
  NLG5_E_OOV = 1 << 0,   //Tx --- Battery output over-voltage; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_MOV_II = 1 << 1, //Tx --- Mains over-voltage 2 detected
  NLG5_E_MOV_I = 1 << 2, //Tx --- Mains over-voltage 1 detected
  NLG5_E_SC = 1 << 3,    //Tx --- Power stage short circuit condition detected; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_P_OM = 1 << 4,  //Tx --- Plausibility battery output voltage measurement wrong
  NLG5_E_P_MV = 1 << 5,  //Tx --- Plausibility mains voltage measurement wrong
  NLG5_E_OF  = 1 << 6,   //Tx --- Output fuse defective
  NLG5_E_MF = 1 << 7,    //Tx --- Mains fuse defective
  NLG5_E_B_P = 1 << 8,   //Tx --- Wrong battery polarity; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_T_C = 1 << 9,   //Tx --- Temperature sensor for prim capacitor defective
  NLG5_E_T_POW = 1 << 10, //Tx --- Temperature sensor for prim power stage defective
  NLG5_E_T_DIO = 1 << 11, //Tx --- Temperature sensor for diodes defective
  NLG5_E_T_TR = 1 << 12, //Tx --- Temperature sensor for transformer defective
  NLG5_E_T_EXT1 = 1 << 13, //Tx --- Ext. temperature sensor 1 defective (if enabled)
  NLG5_E_T_EXT2 = 1 << 14, //Tx --- Ext. temperature sensor 2 defective (if enabled), ext. temperature sensor 2 defective (if enabled)
  NLG5_E_T_EXT3 = 1 << 15, //Tx --- Ext. temperature sensor 3 defective (if enabled)
  NLG5_E_F_CRC = 1 << 16, //Tx --- Flash memory checksum failure
  NLG5_E_NV_CRC = 1 << 17, //Tx --- NVSRAM check sum failure; contains most of the scaling & calibration values, CAN ID's and charging profile etc.
  NLG5_E_ES_CRC = 1 << 18, //Tx --- Sys EEPROM checksum failure
  NLG5_E_EP_CRC = 1 << 19, //Tx --- Pow EEPROM checksum failure
  NLG5_E_WDT = 1 << 20,  //Tx --- Internal Watchdog Timeout
  NLG5_E_INIT = 1 << 21, //Tx --- Initialisation error
  NLG5_E_C_TO = 1 << 22, //Tx --- CAN timeout, no control message received for >300 ms
  NLG5_E_C_OFF = 1 << 23, //Tx --- CAN off, transmit buffer >255
  NLG5_E_C_TX = 1 << 24, //Tx --- CAN transmit buffer >127
  NLG5_E_C_RX = 1 << 25, //Tx --- CAN receiver buffer >127
  NLG5_E_SDT_BT = 1 << 26, //Tx --- Emergency shutdown threshold 'Battery Temperature' is exceeded; see ChargeStar help section 'protective features'; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_SDT_BV = 1 << 27, //Tx --- Emergency shutdown threshold 'Battery Voltage' is exceeded; see ChargeStar help section 'protective features'; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_SDT_AH = 1 << 28, //Tx --- Emergency shutdown threshold 'Amp Hours' is exceeded; see ChargeStar help section 'protective features'; error can only be cleared by cycling power ON-OFF-ON
  NLG5_E_SDT_CT = 1 << 29, //Tx --- Emergency shutdown threshold 'Charging Time' is exceeded; see ChargeStar help section 'protective features'; error can only be cleared by cycling power ON-OFF-ON
  NLG5_W_PL_MV = 1 << 32, //Tx --- Output power limited by low mains voltage
  NLG5_W_PL_BV = 1 << 33, //Tx --- Output power limited by low battery voltage
  NLG5_W_PL_IT = 1 << 34, //Tx --- Output power limited by charger internal excessive temperature
  NLG5_W_C_VOR = 1 << 35, //Tx --- Commanded value is out of specified range; max or min applicable value is assumed instead
  NLG5_W_CM_NA = 1 << 36, //Tx --- NLG5 control message not active
  NLG5_W_OD = 1 << 38,   //Tx --- LED output driver defective, LEDs can't be controlled by NLG5 anymore. Charging is still possible.
  NLG5_W_SC_M = 1 << 39  //Tx --- Save-Charging-Mode reduces primary current to 3.95 A as long as one of the four internal temperature sensors indicates -18° C or less

};

enum DIAG_TX {
  LAD_D_ParamRp = 1 << 0 //Tx
};



class BrusaCharger : public Charger
{
  public:

    BrusaCharger();
    void handleTick();
    void handleCanFrame(CAN_FRAME *frame);
    void setup();
    DeviceId getId();
    uint32_t getTickInterval();
    void processStatus(uint8_t data[]);
    void processErrors(uint8_t data[]);
    void processTemperature(uint8_t data[]);

  private:


    int tickCounter; // count how many times handleTick() was called
    uint32_t proximity_voltage;
    uint32_t cable_connected;
    uint8_t controlBitField; // the control bit field to send via DMC_CTRL in data[0]
    CAN_FRAME outputFrame; // the output CAN frame;


    void sendControl();
    void sendControl2();
    void sendLimits();
    void prepareOutputFrame(uint32_t);
    
    void DisablePS();
    void DisableHeating();
    void DisableDrive();
    bool CheckProximity();
    uint8_t print_counter;
};

#endif /* BRUSACHARGER_H_ */
