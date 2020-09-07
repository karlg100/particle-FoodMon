#include "WifiMeter.h"
#include <Adafruit_LiquidCrystal.h>

WifiMeter::WifiMeter(LiquidCrystal *l, int slot, int x, int y) {
    lcd=l;
    charSlot = slot;
    lcd->createChar(charSlot, wifiNS);
    posX = x;
    posY = y;
}

void WifiMeter::print() {
    lcd->setCursor(posX, posY);
    lcd->write(byte(charSlot));
}

void WifiMeter::wifiStrength() {
    int rssi;
    rssi = WiFi.RSSI();
    if (rssi > -50)
        lcd->createChar(charSlot, wifi100);
    else if (rssi > -65)
        lcd->createChar(charSlot, wifi75);
    else if (rssi > -70)
        lcd->createChar(charSlot, wifi50);
    else if (rssi > -80)
        lcd->createChar(charSlot, wifi25);
    else if (rssi <= -90)
        lcd->createChar(charSlot, wifi0);
    else if (rssi >= 1)
        lcd->createChar(charSlot, wifiNS);
}

