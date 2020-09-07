#include "application.h"
#include <math.h>

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_LiquidCrystal.h>

// This #include statement was automatically added by the Particle IDE.
#include <OneWire.h>

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include <blynk.h>

// This #include statement was automatically added by the Particle IDE.
#include "WifiMeter.h"

// This #include statement was automatically added by the Particle IDE.
#include "CountdownTimer.h"

// This #include statement was automatically added by the Particle IDE.
#include "Probe.h"

//SYSTEM_MODE(SEMI_AUTOMATIC)


// faster startup time for the LCD
//SYSTEM_MODE(MANUAL);
//SYSTEM_MODE(SEMI_AUTOMATIC);
//SYSTEM_THREAD(DISABLED);

// are we OTA flashing?
bool flashPrep = false;

// Setup an app watchdog to reset if loop hangs
ApplicationWatchdog wd(120000, System.reset);

#define ALARM_SLEEP 600000
#define BUZZER_SLEEP 60000

#define BUZZER A5


// map the sensor numbers to the OneWire bus ID
#define S0 1
#define S1 0
#define S2 2

// blynk mapping
#define S1B         V1
#define S1B_LA_O    V11
#define S1B_HA_O    V21
#define S1B_C_O     V31
#define S1B_LA_I    V41
#define S1B_HA_I    V51
#define S1B_INDEX   V61
#define S2B         V2
#define S2B_LA_O    V12
#define S2B_HA_O    V22
#define S2B_C_O     V32
#define S2B_LA_I    V42
#define S2B_HA_I    V52
#define S2B_INDEX   V62
#define S3B         V3
#define S3B_LA_O    V13
#define S3B_HA_O    V23
#define S3B_C_O     V33
#define S3B_LA_I    V43
#define S3B_HA_I    V53
#define S3B_INDEX   V63

// LCD Pin (data and clk +1)
#define LCDPIN 0

// define special LCD chars.  We only have 8 slots!
#define WIFIICON 0
#define TIMER 1
#define DEGF 6

// Init Dallas on pin digital pin 3
OneWire oneWire(D4);
//DallasTemperature dallas(&oneWire);
DallasTemperature dallas(&oneWire);
//DallasTemperature dallas(new oneWire(D4));
DeviceAddress addr;

// Wifi Meter
WifiMeter *wifiMtr;
LCDTimerAnim *LCDTimer;

// Blynk Auth Token
// Food Mon box
char auth[] = "141ba00d105048d09aeb93d71bd59e80";

bool isFirstConnect = TRUE;

LiquidCrystal *lcd;  // LCD I2C

// define our probes
Probe *p1;
Probe *p2;
Probe *p3;

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

BLYNK_CONNECTED() // runs every time Blynk connection is established
{
    Blynk.syncAll();
}

BLYNK_DISCONNECTED() // runs every time Blynk connection is established
{
}


// serial debugging functions
int safeMode(String extra) {
    lcd->clear();
    lcd->home();
//    lcd.setCursor(0, 0);
    lcd->print("SAFE MODE");
    System.enterSafeMode();
    return 0;
}

void serialDebug() {
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
    Serial.print("probe 1 - ");
    Serial.print(p1->getTemp());
    Serial.print(" connected ");
    Serial.println(p1->present());
    Serial.print("probe 2 - ");
    Serial.print(p2->getTemp());
    Serial.print(" connected ");
    Serial.println(p2->present());
    Serial.print("probe 3 - ");
    Serial.print(p3->getTemp());
    Serial.print(" connected ");
    Serial.println(p3->present());
}


// Alarm handlers
unsigned long blkNextAlarm = 0;
unsigned long buzNextAlarm = 0;

void blynkNotify(Probe *p, char msg[255]) {
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
    if (p1->fetchAlarm()) {
         if (Blynk.connected()) blynkNotify(p1, "Kitchen: Probe 1 is alarming");
        alarming = TRUE;
    }
    if (p2->fetchAlarm()) {
        if (Blynk.connected()) blynkNotify(p2, "Kitchen: Probe 2 is alarming");
        alarming = TRUE;
    }
    if (p3->fetchAlarm()) {
        if (Blynk.connected()) blynkNotify(p3, "Kitchen:Probe 3 is alarming");
        alarming = TRUE;
    }
    if (!alarming) {
        blkNextAlarm = 0;
        buzNextAlarm = 0;
    }
}

// init our probes
void initProbes() {
    p1 = new Probe(S0, DEVICE_DISCONNECTED_F);
    p2 = new Probe(S1, DEVICE_DISCONNECTED_F);
    p3 = new Probe(S2, DEVICE_DISCONNECTED_F);
}

// poll the probes
unsigned long nextUpdate = 0;
void pollProbes() {
    if (millis() > nextUpdate) {
        ATOMIC_BLOCK() {
            dallas.requestTemperatures();
            delay(100);
            p1->sample(dallas.getTempFByIndex(S0));
            p2->sample(dallas.getTempFByIndex(S1));
            p3->sample(dallas.getTempFByIndex(S2));
        }
        checkAlarmStates();
        //serialDebug();
        //serialDebug2();
        nextUpdate = millis() + 500;
    }
}

//Timer probesTimer(1000, pollProbes);

// LCD handlers
void lcdProbeScreen(int id, Probe *p);

bool forceRead = false;

void lcdProbeScreen(int id, Probe *p) {
    char msg[15];
    //lcd->clear();
    LCDTimer->print();
//    wifiMtr->print();

    lcd->setCursor(0, 0);
    if (forceRead || p->present()) {
        sprintf(msg, "Probe %d %3.0f", id, p->getTemp());
        lcd->print(msg);
        lcd->write(DEGF);
        lcd->print("  ");
        lcd->setCursor(0, 1);
        sprintf(msg, "L:%d", p->getLowAlarm());
        lcd->print(msg);
        lcd->write(DEGF);
        sprintf(msg, " H:%d", p->getHighAlarm());
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

void padStr(int chars, int len) {
    for (int x=chars; x<=len; x++) {
        lcd->print(" ");
    }
}

int probeScreen = 1;
int clearScreen = FALSE;
unsigned long nextWifi = 0;
void lcdReadOut() {
    if (clearScreen) {
        lcd->clear();
        clearScreen = FALSE;
    }
    switch (probeScreen) {
        case (1):
            lcdProbeScreen(1, p1);
            break;
        case (2):
            lcdProbeScreen(2, p2);
            break;
        case (3):
            lcdProbeScreen(3, p3);
            break;
        case (0):
            lcd->setCursor(0, 0);
            lcd->print("Almost Done...");
            break;
    }
}

void lcdMeters() {
    if (LCDTimer->runTimer()) {

        // keep to screen 1
        //probeScreen = 1;

        if (probeScreen >= 3) {
            probeScreen = 1;
        } else
            probeScreen++;
        clearScreen = TRUE;

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
//void blynkUpdateProbe(Probe *p, int pin);
void blynkUpdateProbe(Probe *p, int pin) {
    if ( p->present() )
        Blynk.virtualWrite(pin, p->getTemp());
    else
        Blynk.virtualWrite(pin, "No Probe");

    if ( p->lowAlarming() )
        Blynk.virtualWrite(pin+10, 255);
    else
        Blynk.virtualWrite(pin+10, 0);

    if ( p->highAlarming() )
        Blynk.virtualWrite(pin+20, 255);
    else
        Blynk.virtualWrite(pin+20, 0);

    if ( p->present() )
        Blynk.virtualWrite(pin+30, 255);
    else
        Blynk.virtualWrite(pin+30, 0);
}


//bool blynkUpdating = false;

unsigned long nextBlynkUpdate = 0;
void blynkUpdate() {
    if ( Blynk.connected() && nextBlynkUpdate != nextUpdate) {
        blynkUpdateProbe(p1, S1B);
        blynkUpdateProbe(p2, S2B);
        blynkUpdateProbe(p3, S3B);
        nextBlynkUpdate = nextUpdate;
    }
}

// Blynk run timer/thread
//void runBlynk() {
//    Blynk.run();
//}

//Timer blynkTimer(1000, blynkUpdate);
//Timer blynkRunTimer(200, runBlynk);

// OTA flash handler
void OTAFlash() {
//    lcd.setCursor(0, 0);
    if (flashPrep == false) {
        //blynkTimer.stop();
        lcdTimer.stop();
        lcdMeterTimer.stop();
        //probesTimer.stop();
        //blynkRunTimer.stop();
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

    //dallas = new DallasTemperature(&oneWire);
    dallas.begin();
    lcd->setCursor(14, 1);
    lcd->print(" ");

    lcd->setCursor(14, 1);
    lcd->print("r");
    dallas.setResolution(12);
    lcd->setCursor(14, 1);
    lcd->print(" ");

    lcd->setCursor(14, 1);
    lcd->print("p");
    initProbes();
    lcd->setCursor(14, 1);
    lcd->print(" ");

    // start reading probes
    //lcd->setCursor(14, 1);
    //lcd->print("t");
    //probesTimer.start();
    //lcd->setCursor(14, 1);
    //lcd->print(" ");

    // stat outputting to LCD
    wifiMtr = new WifiMeter(lcd, WIFIICON, 15, 0);
    LCDTimer = new LCDTimerAnim(lcd, TIMER, 15, 1);
    lcd->clear();
    lcdTimer.start();
    lcdMeterTimer.start();
    Time.zone(-4);                   // Eastern Time Zone
    Particle.function("safeMode", safeMode);

    // start up Blynk
    //lcd->setCursor(14, 1);
    //lcd->print("b");

    // setup output for buzzer
    pinMode(BUZZER, OUTPUT);

    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
    Blynk.begin(auth);
    //blynkRunTimer.start();
    //blynkTimer.start();
    //lcd->setCursor(14, 1);
    //lcd->print(" ");

    // set up the LCD's number of rows and columns:
    // Print a message to the LCD.
    //lcd.print("hello, world!");
    //lcd->clear();
    //lcd->home();
}


void loop() {
    //if (Blynk.connected())

    pollProbes();
    //lcdReadOut();

    if (Blynk.connected()) blynkUpdate();
    Blynk.run();

    if (Blynk.connected()) {
        digitalWrite(D7, LOW);
    } else
        digitalWrite(D7, HIGH);

    //serialDebug();

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    //lcd->setCursor(8, 1);
    // print the number of seconds since reset:
    //lcd->print(millis()/1000);
    
    //lcd.setCursor(15, 1);
    //lcd.write(byte(0));
}


