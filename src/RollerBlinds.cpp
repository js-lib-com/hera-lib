#include "RollerBlinds.h"
#include "Log.h"

#define ADDR_DOWN_POSITION 0
#define ADDR_LAST_KNOWN_POSITION ADDR_DOWN_POSITION + sizeof(long)
#define ADDR_OPEN_PENDING ADDR_LAST_KNOWN_POSITION + sizeof(long)

const char* RollerBlinds::deviceClass = "js.hera.dev.RollerBlinds";
Action RollerBlinds::metaActions[] = {
  ACTION("open", &RollerBlinds::open),
  ACTION("close", &RollerBlinds::close),
  ACTION("position", &RollerBlinds::position),
  ACTION("state", &RollerBlinds::state),
  ACTION("move", &RollerBlinds::move),
  ACTION("stop", &RollerBlinds::stop),
  ACTION("updateUpPosition", &RollerBlinds::updateUpPosition),
  ACTION("updateDownPosition", &RollerBlinds::updateDownPosition),
  ACTION("dump", &RollerBlinds::dump)
};

RollerBlinds::RollerBlinds(const char* deviceName, byte pin1, byte pin2, byte pin3, byte pin4, MotorPosition motorPosition, byte eepromAddr):
  Device(deviceClass, deviceName),
  stepper(4, pin1, pin2, pin3, pin4),
  downPosition(0),
  lastKnownPosition(0),
  calibrationRequired(false),
  movingSteps(0),
  openPending(0),
  rotationSens(motorPosition == LEFT ? 1 : -1),
  eepromAddr(eepromAddr)
{
  actions = metaActions;
  actionsCount = sizeof(metaActions) / sizeof(metaActions[0]);
}

void RollerBlinds::setup() {
  Log::trace("RollerBlinds::setup");

  Log::debug("Reading down position from EEPROM address: ", eepromAddr + ADDR_DOWN_POSITION);
  E2PROM::get(eepromAddr + ADDR_DOWN_POSITION, downPosition);
  Log::debug("Down position: ", downPosition);

  Log::debug("Reading last know position from EEPROM address: ", eepromAddr + ADDR_LAST_KNOWN_POSITION);
  E2PROM::get(eepromAddr + ADDR_LAST_KNOWN_POSITION, lastKnownPosition);
  Log::debug("Last known position: ", lastKnownPosition);

  Log::debug("Reading open pending from EEPROM address: ", eepromAddr + ADDR_OPEN_PENDING);
  openPending = E2PROM::read(eepromAddr + ADDR_OPEN_PENDING);
  Log::debug("Open pending: ", openPending);

  if (openPending) {
    Log::debug("Open pending. Force calibration.");
    downPosition = 0;
    lastKnownPosition = 0;
    openPending = 0;
    calibrationRequired = true;
  }

  stepper.setMaxSpeed(500);
  stepper.setAcceleration(50);
  Log::debug("Set stepper motor current position to ", lastKnownPosition);
  stepper.setCurrentPosition(lastKnownPosition);
}

void RollerBlinds::loop() {
  if (movingSteps != 0) {
    // movingSteps are non zero only on roller calibration when stepper is moved manually to maximum up and down positions
    stepper.move(rotationSens * movingSteps);
    stepper.runSpeedToPosition();
    return;
  }
  if (!openPending) {
    return;
  }
  stepper.run();
  if (stepper.distanceToGo() == 0) {
    openPending = 0;
    stepper.disableOutputs();

    E2PROM::write(eepromAddr + ADDR_OPEN_PENDING, openPending);
    Log::debug("Reset open pending on EEPROM.");

    lastKnownPosition = stepper.currentPosition();
    E2PROM::put(eepromAddr + ADDR_LAST_KNOWN_POSITION, lastKnownPosition);
    Log::debug("Save last known position on EEPROM: ", lastKnownPosition);
  }
}

String RollerBlinds::open(const String& parameter) {
  if (calibrationRequired) {
    Log::debug("Calibrarion required.");
    return state(parameter);
  }

  movingSteps = 0;
  openPending = 1;
  E2PROM::write(eepromAddr + ADDR_OPEN_PENDING, openPending);
  Log::debug("Set open pending on EEPROM.");
  stepper.enableOutputs();

  float percent = parameter.toFloat();
  stepper.moveTo((1.0F - percent) * downPosition);

  return state(parameter);
}

String RollerBlinds::updateDownPosition(const String& parameter) {
  calibrationRequired = false;
  downPosition = stepper.currentPosition();
  E2PROM::put(eepromAddr + ADDR_DOWN_POSITION, downPosition);
  Log::debug("Save down position to E2PROM: ", downPosition);
  return String(stepper.currentPosition());
}

String RollerBlinds::dump(const String& parameter) {
  String state = "{";
  state += "\"calibrationRequired\":";
  state += calibrationRequired;
  state += ",\"movingSteps\":";
  state += movingSteps;
  state += ",\"downPosition\":";
  state += downPosition;
  state += ",\"lastKnownPosition\":";
  state += lastKnownPosition;
  state += ",\"openPending\":";
  state += openPending;
  state += ",\"eepromAddr\":";
  state += eepromAddr;
  state += "}";
  return state;
}

