#include "Device.h"

Device::Device(const char* deviceName):
  deviceName(deviceName)
{
  local = true;
}

Device::Device(const char* deviceClass, const char* deviceName):
  deviceName(deviceName)
{
  local = false;
}

String Device::invoke(const String& action, const String& parameter) {
  return "";
}

