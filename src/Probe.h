
/*
#define ALARM_NONE 0x00
#define ALARM_LOW 0x01
#define ALARM_HIGH 0x04
#define ALARM_ERROR 0x08
*/

class Probe {
    private:
        int id;
        //char name[10];

        bool connected = false;
        int conCount = 10;
        volatile float calTemp = 0;   // our readings
        volatile float rawTemp = 0;   // our readings

        // Disconnected float
        float disTemp =32;

        // Calibration data (all in F)
        float refLow = 32.9;                // low reference temp
        float refHigh = 212;                // high reference temp
        float refRange = refHigh-refLow;    // calculate the reference range - ref high minus low

        // probe calibraiton info
        // 1 - oven probe 1 (needle)
        // 1 - oven probe 2 (needle)
        // 1 - probe 3 (blunt end)
        // 1 - probe 4 (blunt end)

        int probeIndex = 0;
        float probeLow[10] = { };
        float probeHigh[10] = { };
        float probeRange[10] = { };

        // Alarm tracking variables
        volatile int highAlarm = 100;
        volatile int lowAlarm = -100;
        volatile bool highAlarmActive = false;
        volatile bool lowAlarmActive = false;
//        volatile unsigned char alarmState = ALARM_NONE;

        int alarmCount = 0;
        int alarmThresh = 1;
        unsigned long lastAlarmTime = 0;

        //Probe *nextProbe;
        //DallasTemperature *dallas;

    public:

//        Probe(int probeId, DallasTemperature *dh);
        Probe(int probeId, int dTemp);
//        void sample();
        void sample(float tmpTemp);
	    int getID();
	    float getRawTemp();
	    float getTemp();
	    bool present();
	    void setHighAlarm(int level);
        int getHighAlarm();
        void setLowAlarm(int level);
        int getLowAlarm();
        void setProbeIndex(int index);
        bool fetchAlarm();
        bool alarming();
        bool highAlarming();
        bool lowAlarming();
};