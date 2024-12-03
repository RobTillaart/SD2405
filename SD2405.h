#pragma once
//
//    FILE: SD2405.h
//  AUTHOR: Rob Tillaart
// PURPOSE: Arduino library for I2C SD2405 RTC and compatibles.
// VERSION: 0.1.1
//    DATE: 2022-03-17
//     URL: https://github.com/RobTillaart/SD2405


#include "Arduino.h"
#include "Wire.h"


#define SD2405_LIB_VERSION           (F("0.1.1"))


//  ERROR CODES
#define SD2405_OK                  0
#define SD2405_ERROR_ADDR          -10
#define SD2405_ERROR_I2C           -11
#define SD2405_ERROR_CONNECT       -12


//  REGISTERS SD2405 (maybe also for SD2405)
//  not all used yet
#define SD2405_TIME_BASE            0x00
#define SD2405_ALARM_BASE           0x07
#define SD2405_CONTROL_1            0x0F
#define SD2405_CONTROL_2            0x10
#define SD2405_CONTROL_3            0x11
#define SD2405_TIME_TRIMMING        0x12
#define SD2405_COUNTDOWN            0x13
#define SD2405_SRAM_BASE            0x14



// SD2405AL

class SD2405
{
public:
  //  Constructor
  SD2405(TwoWire *wire = &Wire);
  int      begin();
  bool     isConnected();
  uint8_t  getAddress();


  ////////////////////////////////////////////////////////////////////
  //
  // BASE RTC
  //
  int      read();
  int      write();
  uint32_t lastRead();

  //  Getters
  uint8_t seconds();
  uint8_t minutes();
  uint8_t hours();
  uint8_t weekDay();
  uint8_t day();
  uint8_t month();
  uint8_t year();

  //  Setters
  void setSeconds(uint8_t value);
  void setMinutes(uint8_t value);
  void setHours(uint8_t value);
  void setWeekDay(uint8_t value);
  void setDay(uint8_t value);
  void setMonth(uint8_t value);
  void setYear(uint8_t value);


  ////////////////////////////////////////////////////////////////////
  //
  //  CONFIGURE INTERRUPT FUNCTIONS
  //
  //  par 5.3, register 0x10, INTS0, INTS1, INTDE, INTAE, INTFE, IM
  //  source: disable = 0, 1 = alarm, 2 = frequency, 3 = timer
  //  repeat: single = false, repeat = true until INTAF is reset
  //  autoReset: ARST = 0 = false, ARST = 1 = true.
  int configureInterrupt(uint8_t source, bool repeat, bool autoReset);

  //  par 5.3, register 0x0F, INTAF, INTDF reset both manually.
  int resetInterruptFlags() { return readRegister(0x0F); };


  ////////////////////////////////////////////////////////////////////
  //
  //  ALARM INTERRUPT FUNCTIONS
  //
  //  par 5.3, register 0x07 - 0x0D
  int setAlarmSecond(uint8_t value) { return writeRegister(0x07, value); };
  int setAlarmMinute(uint8_t value) { return writeRegister(0x08, value); };
  int setAlarmHour(uint8_t value)   { return writeRegister(0x09, value); };
  int setAlarmWeek(uint8_t value)   { return writeRegister(0x0A, value); };  //  format == bit mask.
  int setAlarmDay(uint8_t value)    { return writeRegister(0x0B, value); };
  int setAlarmMonth(uint8_t value)  { return writeRegister(0x0C, value); };
  int setAlarmYear(uint8_t value)   { return writeRegister(0x0D, value); };
  int getAlarmSecond()              { return readRegister(0x07); };
  int getAlarmMinute()              { return readRegister(0x08); };
  int getAlarmHour()                { return readRegister(0x09); };
  int getAlarmWeek()                { return readRegister(0x0A); };
  int getAlarmDay()                 { return readRegister(0x0B); };
  int getAlarmMonth()               { return readRegister(0x0C); };
  int getAlarmYear()                { return readRegister(0x0D); };

  //  par 5.3, register 0x0E
  //  bit_mask = { 0 Y M W D H Min S }
  int setAlarmInterrupt(uint8_t bit_mask) { return writeRegister(0x0E, bit_mask); };


  ////////////////////////////////////////////////////////////////////
  //
  //  FREQUENCY INTERRUPT FUNCTIONS
  //
  //  par 5.3. register 0x11, FS0..FS3
  //  bit_mask = 0..15, TODO add table in readme.md
  int setFrequencyMask(uint8_t bit_mask);


  ////////////////////////////////////////////////////////////////////
  //
  //  COUNTDOWN INTERRUPT FUNCTIONS
  //
  //  par 5.3. register 0x11, FS0..FS3
  //  bit_mask = 0..3, TODO add table in readme.md
  //  TODO examples
  //  1 - 255 minutes, 1 - 255 seconds,
  //  1/64 .. 255/64 seconds, 1/4096 - 255/4096)  => 1/100 ~~ 41/4096
  //  e.g 5 minute repeating alarm?
  int setCountDownMask(uint8_t bit_mask);
  //  par 5.3. register 0x13
  int setCountDown(uint8_t value) { return writeRegister(0x13, value); };


  ////////////////////////////////////////////////////////////////////
  //
  //  TIME TRIMMING FUNCTIONS
  //
  //  par 5.4. register 0x12, 0..127
  //  read the data sheet (twice)
  //  oscillator = actual frequency (ist)
  //  target = target frequency (soll)
  int adjustClockFrequency(int32_t oscillator, int32_t target);


  ////////////////////////////////////////////////////////////////////
  //
  //  OTHER FUNCTIONS
  //
  //  par 5.5. register
  //  read the data sheet (twice)
  int enableWriteRTC();
  int disableWriteRTC();

/*
  void setFOBAT(bool flag);

  bool getRCTF();

*/


  ////////////////////////////////////////////////////////////////////
  //
  //  SRAM SUPPORT
  //
  //  SRAM 12 bytes, register 0x14-0x1F
  //  index = 0..11 == 0x00..0x1B
  //  12 bytes = 3 x uint32_t (e.g. time stamps)
  //  note: no boundary check
  //  TODO: optimize read/write multiple bytes at once.
  int      SRAMwrite8(uint8_t index, uint8_t value);
  int      SRAMwrite16(uint8_t index, uint16_t value);
  int      SRAMwrite32(uint8_t index, uint32_t value);
  uint8_t  SRAMread8(uint8_t index);
  uint16_t SRAMread16(uint8_t index);
  uint32_t SRAMread32(uint8_t index);


  //  LOW LEVEL access to all registers
  //  check datasheet for details of registers.
  //  return < 0 is error
  int      readRegister(uint8_t reg);
  int      writeRegister(uint8_t reg, uint8_t value);


protected:
  uint8_t   _address;
  TwoWire * _wire;

  //  array holding the values
  uint8_t   _reg[7];
  uint32_t  _lastRead = 0;

  int       _rv;
  uint16_t  _type = 3231;

  uint8_t   dec2bcd(uint8_t value);
  uint8_t   bcd2dec(uint8_t value);
};



//  -- END OF FILE --

