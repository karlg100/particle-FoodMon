#include <Adafruit_LiquidCrystal.h>

class LCDTimerAnim {
    private:
        byte Timer00[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b01110
        };

        byte Timer01[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11111,
        	0b01110
        };
        
        
        byte Timer02[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        
        byte Timer03[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        
        byte Timer04[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        
        byte Timer05[8] = {
        	0b01110,
        	0b10001,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        
        byte Timer10[8] = {
        	0b01110,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        byte Timer11[8] = {
        	0b01110,
        	0b11011,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        byte Timer12[8] = {
        	0b01110,
        	0b10001,
        	0b11011,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        byte Timer13[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b11011,
        	0b11111,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        byte Timer14[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11011,
        	0b11111,
        	0b11111,
        	0b01110
        };
        
        byte Timer15[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11011,
        	0b11111,
        	0b01110
        };
        
        byte Timer16[8] = {
        	0b01110,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b10001,
        	0b11011,
        	0b01110
        };

        int currentIcon = 2;
        unsigned long sleepTime = 0;
        LiquidCrystal *lcd;
        int charSlot;
        int posX;
        int posY;
        bool resetTimer = false;
    public:
        LCDTimerAnim(LiquidCrystal *l, int slot, int x, int y);
        bool runTimer();
        void print();
        void reset();
};