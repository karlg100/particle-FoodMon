#include "CountdownTimer.h"
#include <Adafruit_LiquidCrystal.h>
LCDTimerAnim::LCDTimerAnim(LiquidCrystal *l, int slot, int x, int y) {
    lcd = l;
    charSlot = slot;
    lcd->createChar(charSlot, Timer01);
    posX = x;
    posY = y;
}

bool LCDTimerAnim::runTimer() {
    if (resetTimer) {
        sleepTime = 0;
        resetTimer = false;
        return true;
    }
    if (millis() < sleepTime)
        return false;
    int sleep = 0;
    bool returnCode = false;
    currentIcon++;
    switch (currentIcon) {
        case 1:
            lcd->createChar(charSlot, Timer00);
            sleep = 500;
            break;
        case 2:
            lcd->createChar(charSlot, Timer01);
            returnCode = true;
            sleep = 100;
            break;
        case 3:
            lcd->createChar(charSlot, Timer02);
            sleep = 100;
            break;
        case 4:
            lcd->createChar(charSlot, Timer03);
            sleep = 100;
            break;
        case 5:
            lcd->createChar(charSlot, Timer04);
            sleep = 100;
            break;
        case 6:
            lcd->createChar(charSlot, Timer05);
            sleep = 100;
            break;
        case 7:
            lcd->createChar(charSlot, Timer10);
            sleep = 100;
            break;
        case 8:
            lcd->createChar(charSlot, Timer11);
            sleep = 100;
            break;
        case 9:
            lcd->createChar(charSlot, Timer12);
            sleep = 250;
            break;
        case 10:
            lcd->createChar(charSlot, Timer13);
            sleep = 250;
            break;
        case 11:
            lcd->createChar(charSlot, Timer14);
            sleep = 250;
            break;
        case 12:
            lcd->createChar(charSlot, Timer15);
            sleep = 250;
            break;
        case 13:
            lcd->createChar(charSlot, Timer16);
            sleep = 250;
            currentIcon = 0;
            break;
    }
    sleepTime = millis() + sleep;
    return returnCode;
}

void LCDTimerAnim::print() {
    lcd->setCursor(posX, posY);
    lcd->write(byte(charSlot));
}

void LCDTimerAnim::reset() {
    resetTimer = true;
}
