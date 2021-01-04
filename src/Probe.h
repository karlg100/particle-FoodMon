
/*
#define ALARM_NONE 0x00
#define ALARM_LOW 0x01
#define ALARM_HIGH 0x04
#define ALARM_ERROR 0x08
*/

#include <inttypes.h>
#include <DallasTemperature.h>

class Probe {
    private:
        int id;
        DallasTemperature *dh;
        uint8_t pinAddress = 0;
        uint8_t probeIndex = 0;

        volatile double F = DEVICE_DISCONNECTED_F;   // our readings
        volatile double C = DEVICE_DISCONNECTED_C;   // our readings

        // Alarm tracking variables
        volatile int highAlarm = 100;
        volatile int lowAlarm = -100;
        volatile bool highAlarmActive = false;
        volatile bool lowAlarmActive = false;
//        volatile unsigned char alarmState = ALARM_NONE;

        int alarmCount = 0;
        int alarmThresh = 1;
        size_t lastAlarmTime = 0;

        void setDeviceIndexByPinAddress(int probePinAddress);
        
    public:
        Probe(DallasTemperature *dhIn, int newID, int probePinAddress);
        void sample();
        void setID(int newID);
        int getID();
        void setProbeIndexFromPinAddress();
        void setProbeIndex(int index);
        double getTemp();
        double getTempC();
        bool present();
        void setHighAlarm(int level);
        int getHighAlarm();
        void setLowAlarm(int level);
        int getLowAlarm();
        bool fetchAlarm();
        bool alarming();
        bool highAlarming();
        bool lowAlarming();
};