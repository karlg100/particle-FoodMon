#include "Probe.h"
#include "application.h"
//include "DallasTemperature.h"
#include <math.h>
// This #include statement was automatically added by the Particle IDE.
//#include <spark-dallas-temperature.h>

Probe::Probe(DallasTemperature *dhIn, int newID, int probePinAddress) {
    pinAddress = probePinAddress;
    id = newID;
    dh = dhIn;
    setDeviceIndexByPinAddress(probePinAddress);
}

void Probe::setDeviceIndexByPinAddress(int probePinAddress) {
    for (uint8_t s=0; s < dh->getDeviceCount(); s++)
        if (dh->getAddressPinsByIndex(s) == probePinAddress) {
            setProbeIndex(s);
            break;
        }

}

void Probe::sample() {
    C = dh->getTempCByIndex(probeIndex);
    F = dh->getTempFByIndex(probeIndex);
}

int Probe::getID() {
    return id;
}

void Probe::setID(int newID) {
    id = newID;
}

void Probe::setProbeIndex(int index) {
    probeIndex = index;
}

double Probe::getTemp() {
    return F;
}

double Probe::getTempC() {
    return C;
}

bool Probe::present() {
    if ( C == DEVICE_DISCONNECTED_C || C == DEVICE_FAULT_OPEN_C || C == DEVICE_FAULT_SHORTGND_C || C == DEVICE_FAULT_SHORTVDD_C)
        return false;
    else
        return true;
}

void Probe::setHighAlarm(int level) {
    highAlarm = level;
}

int Probe::getHighAlarm() {
    return highAlarm;
}

void Probe::setLowAlarm(int level) {
    lowAlarm = level;
}

int Probe::getLowAlarm() {
    return lowAlarm;
}

bool Probe::fetchAlarm() {
    if (!present()) {
        highAlarmActive = FALSE;
        lowAlarmActive = FALSE;
        return FALSE;
    }

    // add up flags
    if (F > highAlarm) {
        highAlarmActive = TRUE;
    } else {
        highAlarmActive = FALSE;
    }
    
    if (F < lowAlarm) {
        lowAlarmActive = TRUE;
    } else {
        lowAlarmActive = FALSE;
    }

    // incremnt counter
    if (highAlarmActive == FALSE && lowAlarmActive == FALSE) {
        alarmCount = 0;
        lastAlarmTime = 0;
    } else
        alarmCount++;

    if (alarmCount >= alarmThresh) {
        return TRUE;
    } else
        return FALSE;
}

bool Probe::alarming() {
    if (highAlarmActive || lowAlarmActive)
        return TRUE;
    else
        return FALSE;
}

bool Probe::highAlarming() {
    return highAlarmActive;
}

bool Probe::lowAlarming() {
    return lowAlarmActive;
}
