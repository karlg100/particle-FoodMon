
#include <application.h>
#include <math.h>
#include <Adafruit_LiquidCrystal.h>
#include <OneWire.h>
#include <blynk.h>

////
/// Local Libraries
//
#include "DallasTemperature.h""
#include "BlynkSetup.h"
#include "WifiMeter.h"
#include "CountdownTimer.h"
#include "Probe.h"

////
/// System Configuration
//
//SYSTEM_MODE(SEMI_AUTOMATIC)

// faster startup time for the LCD
//SYSTEM_MODE(MANUAL);
//SYSTEM_MODE(SEMI_AUTOMATIC);
//SYSTEM_THREAD(DISABLED);

// Setup an app watchdog to reset if loop hangs
ApplicationWatchdog wd(120000, System.reset);


////
/// Init Device Libs
//
// Init Dallas on pin D4
OneWire oneWire(D4);
DallasTemperature dallas(&oneWire);

// OTA flash flag
bool flashPrep = false;

// Alarm Params
#define ALARM_SLEEP 600000
#define BUZZER_SLEEP 60000
#define BUZZER A5


////
/// Setup the probes
//

// The "A" address pins can be set to map the hardware in a speicfic order. Set the vaules of SA* to the order of the A addresses
// The rest will be discovered. (should be in order from 0-3, unelss you didn't install them in the right order on your board)

#define MAX_DEVICES 16

const int SA[] = {
                    0,
                    1,
                    3,
                    2,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1,
                    -1
                };


////
/// LCD related macros
//

// LCD Pin (data and clk +1)
#define LCDPIN D0

// define special LCD chars.  We only have 8 slots!
#define WIFIICON 0
#define TIMER 1
#define DEGF 6

// Wifi Meter
WifiMeter *wifiMtr;
LCDTimerAnim *LCDTimer;

////
/// Blynk Config
//

// Blynk Auth Token
// Food Mon box
char auth[] = "141ba00d105048d09aeb93d71bd59e80";

bool isFirstConnect = TRUE;

LiquidCrystal *lcd;  // LCD I2C

// define our probes

Probe *p[15];

// Deg F icon
byte degF[8] = {
    0b11000,
    0b11000,
    0b00000,
    0b00111,
    0b00100,
    0b00110,
    0b00100,
    0b00100
};

void regDegFIcon() {
    lcd->createChar(DEGF, degF);
}


////
/// Blynk Functions
//

/// NFR Move to blynk include
BLYNK_CONNECTED() // runs every time Blynk connection is established
{
    Blynk.syncAll();
}

BLYNK_DISCONNECTED() // runs every time Blynk connection is established
{

}

// sound sequence for buzzer alarm
void soundAlarm() {
    digitalWrite(BUZZER, HIGH);
    delay(10);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(10);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(10);
    digitalWrite(BUZZER, LOW);
}

////
/// serial debugging functions
//
int safeMode(String extra) {
    lcd->clear();
    lcd->home();
//    lcd.setCursor(0, 0);
    lcd->print("SAFE MODE");
    System.enterSafeMode();
    return 0;
}

void serialDebug() {
    DeviceAddress addr;
    dallas.requestTemperatures();
    Serial.println(dallas.getDeviceCount());
//    Serial.println(dallas->getTempFByIndex(0));
//    Serial.println(dallas->getTempFByIndex(1));
//    Serial.println(dallas->getTempFByIndex(2));
    Serial.println(DEVICE_DISCONNECTED_F);
    for (uint8_t s=0; s < dallas.getDeviceCount(); s++) {
        // get the unique address 
        dallas.getAddress(addr, s);
        // just look at bottom two bytes, which is pretty likely to be unique
        int smalladdr = (addr[6] << 8) | addr[7];

        Serial.print("Temperature for the device #"); Serial.print(s); 
        Serial.print(" with ID #"); Serial.print(smalladdr);
        Serial.print(" is: ");
        Serial.println(dallas.getTempFByIndex(s));  
    }
}


void serialDebug2() {
    for (uint8_t s=0; s < dallas.getDeviceCount(); s++)
//    for (uint8_t s=0; s < sizeof(p); s++) 
        if (p[s])
            Serial.println("probe "+String(s)+" - connected "+String(p[s]->present())+" Temp "+String(p[s]->getTemp()));
        else
            Serial.println("probe "+String(s)+" - not defined");
}

// Alarm handlers
size_t blkNextAlarm = 0;
size_t buzNextAlarm = 0;

void blynkNotify(char probe[255], char msg[255]) {
    if (Blynk.connected() && millis() > blkNextAlarm) {
        Blynk.notify(msg);
        Particle.publish(msg);
        blkNextAlarm = millis() + ALARM_SLEEP;
    }
    if (millis() > buzNextAlarm) {
        soundAlarm();
        buzNextAlarm = millis() + BUZZER_SLEEP;
    }
}

void checkAlarmStates() {
    bool alarming = FALSE;
    char subj[30], msg[128];
    for (uint8_t s=0; s < dallas.getDeviceCount(); s++) {
//    for (uint8_t s=0; s < sizeof(p); s++) {
//        if (p[s]) {
            if (p[s]->fetchAlarm()) {
                sprintf(subj, "Probe %d", p[s]->getID());
                sprintf(msg, "Kitchen: Probe %d is alarming", p[s]->getID());
                if (Blynk.connected()) blynkNotify(subj, msg);
                alarming = TRUE;
            }
//        }
    }

/* DELETE
    if (p1->fetchAlarm()) {
    }
    if (p2->fetchAlarm()) {
        if (Blynk.connected()) blynkNotify("Probe 2", "Kitchen: Probe 2 is alarming");
        alarming = TRUE;
    }
    if (p3->fetchAlarm()) {
        if (Blynk.connected()) blynkNotify("Probe 3", "Kitchen: Probe 3 is alarming");
        alarming = TRUE;
    }
*/

    if (!alarming) {
        blkNextAlarm = 0;
        buzNextAlarm = 0;
    }
}

// init our probes
void initProbes() {
    for (uint8_t s=0; s < dallas.getDeviceCount(); s++) {
        if (SA[s] == -1)
            continue;
        p[s] = new Probe(&dallas, s, SA[s]);
    }
}

// poll the probes
size_t nextUpdate = 0;
void pollProbes() {
    if (millis() > nextUpdate) {
        ATOMIC_BLOCK() {
            dallas.requestTemperatures();
            //delay(100);
            for (uint8_t s=0; s < dallas.getDeviceCount(); s++)
                p[s]->sample();
        }
        checkAlarmStates();
        //serialDebug();
        serialDebug2();
        nextUpdate = millis() + 500;
    }
}

// LCD handlers
bool forceRead = false;

void lcdScanning(int id) {
    char msg[15];
    wifiMtr->print();

    lcd->setCursor(0, 0);
    sprintf(msg, "Scanning for");
    lcd->print(msg);
    lcd->setCursor(0, 1);
    sprintf(msg, "probe %d...", id+1);
    lcd->print(msg);
}

void lcdProbeScreen(int id, Probe *p) {
    char msg[15];
    wifiMtr->print();

    lcd->setCursor(0, 0);
    if (forceRead || p->present()) {
        sprintf(msg, "Probe %d %3.0f", id+1, p->getTemp());
        lcd->print(msg);
        lcd->write(DEGF);
        lcd->print("  ");
        lcd->setCursor(0, 1);
        sprintf(msg, "%d", p->getLowAlarm());
        lcd->print(msg);
        lcd->write(DEGF);
        sprintf(msg, "/%d", p->getHighAlarm());
        lcd->print(msg);
        lcd->write(DEGF);
        lcd->print("  ");
        if (p->alarming()) {
            lcd->setCursor(14, 0);
            lcd->print("A");
        } else {
            lcd->setCursor(14, 0);
            lcd->print(" ");
        }
    } else {
        sprintf(msg, "Probe %d N/C   ", id);
        lcd->setCursor(0, 0);
        lcd->print(msg);
    }
}

/* DELETE
void padStr(int chars, int len) {
    for (int x=chars; x<=len; x++) {
        lcd->print(" ");
    }
}
*/

int probeScreen = 0;
bool clearScreen = FALSE;
size_t nextWifi = 0;
void lcdReadOut() {
    if (clearScreen) {
        lcd->clear();
        LCDTimer->print();
        clearScreen = FALSE;
    }

    if (forceRead || p[probeScreen]->present()) {
        lcdProbeScreen(probeScreen, p[probeScreen]);
    } else {
        lcdScanning(probeScreen);
        LCDTimer->reset();
    }
}

void lcdMeters() {
    if (LCDTimer->runTimer()) {

        // keep to screen 1
        //probeScreen = 1;
        probeScreen++;
        clearScreen = TRUE;

        if (probeScreen == dallas.getDeviceCount()) {
            probeScreen = 0;
        }
    }
    if (millis() > nextWifi) {
        wifiMtr->wifiStrength();
        nextWifi = millis() + 500;
    }
}

// Blynk incoming event handlers
BLYNK_WRITE(V5)
{
    if (param.asInt())
        LCDTimer->reset();
}


/* FIX
BLYNK_WRITE(S1B_LA_I)
{
    p1->setLowAlarm(param.asInt());
}

BLYNK_WRITE(S1B_HA_I)
{
    p1->setHighAlarm(param.asInt());
}

BLYNK_WRITE(S1B_INDEX)
{
    p1->setProbeIndex(param.asInt());
}

BLYNK_WRITE(S2B_LA_I)
{
    p2->setLowAlarm(param.asInt());
}

BLYNK_WRITE(S2B_HA_I)
{
    p2->setHighAlarm(param.asInt());
}

BLYNK_WRITE(S2B_INDEX)
{
    p2->setProbeIndex(param.asInt());
}

BLYNK_WRITE(S3B_LA_I)
{
    p3->setLowAlarm(param.asInt());
}

BLYNK_WRITE(S3B_HA_I)
{
    p3->setHighAlarm(param.asInt());
}

BLYNK_WRITE(S3B_INDEX)
{
    p3->setProbeIndex(param.asInt());
}
*/

BLYNK_WRITE(V4)
{
    if (param.asInt() == 1) {
        soundAlarm();
    } else
        digitalWrite(BUZZER, LOW);
}

// Define Timers/threads for the two right icons
Timer lcdTimer(500, lcdReadOut);
Timer lcdMeterTimer(150, lcdMeters);

// blynk updates
void blynkUpdateProbe(Probe *p, int pin) {
    if ( p->present() )
        Blynk.virtualWrite(pin, p->getTemp());
    else
        Blynk.virtualWrite(pin, "No Probe");

    if ( p->lowAlarming() )
        Blynk.virtualWrite(pin+(1*MAX_DEVICES), 255);
    else
        Blynk.virtualWrite(pin+(1*MAX_DEVICES), 0);

    if ( p->highAlarming() )
        Blynk.virtualWrite(pin+(2*MAX_DEVICES), 255);
    else
        Blynk.virtualWrite(pin+(2*MAX_DEVICES), 0);

    if ( p->present() )
        Blynk.virtualWrite(pin+(3*MAX_DEVICES), 255);
    else
        Blynk.virtualWrite(pin+(3*MAX_DEVICES), 0);
}


size_t nextBlynkUpdate = 0;
void blynkUpdate() {
    if ( Blynk.connected() && nextBlynkUpdate != nextUpdate) {
        for (uint8_t s=0; s < dallas.getDeviceCount(); s++) 
//        for (uint8_t s=0; s < sizeof(p); s++) 
            if (p[s])
                blynkUpdateProbe(p[s], BLYNK_START_PIN + s);
        nextBlynkUpdate = nextUpdate;
    }
}

// OTA flash handler
void OTAFlash() {
//    lcd.setCursor(0, 0);
    if (flashPrep == false) {
        //blynkTimer.stop();
        lcdTimer.stop();
        lcdMeterTimer.stop();
        lcd->clear();
        lcd->home();
        lcd->print("OTA Flash Mode");
        flashPrep = true;
    }
    //System.enterSafeMode();
}

void setup() {
    // init LCD    
    lcd = new LiquidCrystal(LCDPIN);
    lcd->begin(20, 4);
    lcd->setBacklight(HIGH);

    // setup LCD OTA messaging
    System.on(firmware_update, OTAFlash);

    // startup the serial line
    Serial.begin(9600);

    // startup message
    lcd->print("Starting up...");
    lcd->setCursor(0, 1);
    lcd->print("Please Wait");
    regDegFIcon();

    // init probes
    lcd->setCursor(14, 1);
    lcd->print("d");

    dallas.begin();

    lcd->setCursor(14, 1);
    lcd->print(" ");

    lcd->setCursor(14, 1);
    lcd->print("r");
    //dallas.setResolution(10); // Hack to get faster read times from the MAX31850.  still reads 12 bit rez.  it's about ~7 times faster.
    dallas.setResolution(9); // Hack to get faster read times from the MAX31850.  still reads 12 bit rez.  it's about ~7 times faster.  9 bit reads a little faster, but is under spec by ~5ms
    lcd->setCursor(14, 1);
    lcd->print(" ");

    lcd->setCursor(14, 1);
    lcd->print("p");
    initProbes();
    lcd->setCursor(14, 1);
    lcd->print(" ");

    // stat outputting to LCD
    wifiMtr = new WifiMeter(lcd, WIFIICON, 15, 0);
    LCDTimer = new LCDTimerAnim(lcd, TIMER, 15, 1);
    lcd->clear();
    lcdTimer.start();
    lcdMeterTimer.start();
    Time.zone(-4);                   // Eastern Time Zone
    Particle.function("safeMode", safeMode);

    // setup output for buzzer
    pinMode(BUZZER, OUTPUT);

    // start up Blynk
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
    Blynk.begin(auth);
}

void loop() {
    pollProbes();

    if (Blynk.connected()) {
        digitalWrite(D7, LOW);
        blynkUpdate();
    } else
        digitalWrite(D7, HIGH);

    Blynk.run();

}