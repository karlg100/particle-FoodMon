#include "Probe.h"
#include "application.h"
#include "spark-dallas-temperature.h"
#include <math.h>
// This #include statement was automatically added by the Particle IDE.
//#include <spark-dallas-temperature.h>

Probe::Probe(int probeId, int dTemp) {
    id = probeId;
    disTemp = dTemp;

    // set calibration data for each probe
    probeLow[1] = 44.4;              // what did you read thru the probe?
    probeHigh[1] = 213.0;             // what did you read thru the probe?
    probeRange[1] = probeHigh[1] - probeLow[1]; // calculated probe range

    probeLow[2] = 44.4;              // what did you read thru the probe?
    probeHigh[2] = 213.0;             // what did you read thru the probe?
    probeRange[2] = probeHigh[2] - probeLow[2]; // calculated probe range

    probeLow[3] = 32.0;              // what did you read thru the probe?
    probeHigh[3] = 210.0;             // what did you read thru the probe?
    probeRange[3] = probeHigh[3] - probeLow[3]; // calculated probe range

    probeLow[4] = 32.0;              // what did you read thru the probe?
    probeHigh[4] = 212.0;             // what did you read thru the probe?
    probeRange[4] = probeHigh[4] - probeLow[4]; // calculated probe range

    //float probeRange = 190.35;       // high read temp - what did you read using the probe?
}

void Probe::sample(float tmpTemp) {
    //float tmpTemp = dh->getTempFByIndex(id);
    //Serial.print("probe ");
    //Serial.print(id);
    //Serial.print(" - temp ");
    //Serial.println(tmpTemp);
//    if ( tmpTemp == DEVICE_DISCONNECTED_F) {
    if ( tmpTemp == DEVICE_DISCONNECTED_F) {                                            // disconnected
        connected = false;
//        rawTemp = NAN;
//        calTemp = NAN;
        conCount++;
    } else if (floor(tmpTemp) == 32 || floor(tmpTemp*100) == floor(disTemp*100) ) {     // bogus reading
        conCount++;
    } else {
        rawTemp = tmpTemp;
//        if (probeIndex == 0)
            calTemp = rawTemp;
//        else
//            calTemp = (((rawTemp - probeLow[probeIndex]) * refRange) / probeRange[probeIndex]) + refLow;
        connected = true;
        conCount=0;
    }
}

int Probe::getID() {
    return id;
}

float Probe::getRawTemp() {
    return rawTemp;
}

float Probe::getTemp() {
    return calTemp;
}

bool Probe::present() {
    if (conCount >= 0 && conCount < 10)
        return true;
    else
        return connected;
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

void Probe::setProbeIndex(int index) {
    probeIndex = index;
//    Particle.publish("probeIndex", probeIndex);
}

bool Probe::fetchAlarm() {
    if (!connected) {
        highAlarmActive = FALSE;
        lowAlarmActive = FALSE;
        return FALSE;
    }

    // add up flags
    if (calTemp > highAlarm) {
        highAlarmActive = TRUE;
    } else {
        highAlarmActive = FALSE;
    }
    
    if (calTemp < lowAlarm) {
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
