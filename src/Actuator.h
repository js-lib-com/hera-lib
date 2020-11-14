#ifndef __HERA_ACTUATOR
#define __HERA_ACTUATOR

#include <Adafruit_NeoPixel.h>

#include "Device.h"
#include "OutPort.h"
#include "E2PROM.h"

class Actuator: public Device {
  public:
    Actuator(const char* deviceName, byte port, OutMode outMode, byte eepromAddr = NO_EEPROM);
    Actuator(const char* deviceName, byte port, OutMode outMode, byte indicatorPort, uint32_t ledOnColor, uint32_t ledOffColor, byte eepromAddr = NO_EEPROM);

    void setup();
    String invoke(const String& action, const String& parameter = "");
    virtual const char* getDeviceClass() const;

  private:
    void update();

  private:
    // output port, driver for actuator
    OutPort port;

    // optional adressable LED for actuator state local indication; used only if indicator port is set on constructor
    Adafruit_NeoPixel* indicatorLED;

    // color used to signal ON state
    uint32_t ledOnColor;

    // color used to signal OFF state
    uint32_t ledOffColor;

    // EEPROM address to persist this actuator state
    byte eepromAddr;

  private:
    static const char* deviceClass;
};

inline const char* Actuator::getDeviceClass() const {
  return deviceClass;
}

#endif


