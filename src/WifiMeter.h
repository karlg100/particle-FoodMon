#include <Adafruit_LiquidCrystal.h>

class WifiMeter {
    private:
        byte wifi100[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00001,
        	0b00011,
        	0b00111,
        	0b01111,
        	0b11111
        };
        
        byte wifi75[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00010,
        	0b00110,
        	0b01110,
        	0b11110
        };
        
        byte wifi50[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00100,
        	0b01100,
        	0b11100
        };
        
        byte wifi25[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b01000,
        	0b11000
        };
        
        byte wifi0[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b00000,
        	0b10000
        };
        
        byte wifiNS[8] = {
        	0b00000,
        	0b00000,
        	0b00000,
        	0b01010,
        	0b00100,
        	0b01010,
        	0b00000,
        	0b10000
        };
        LiquidCrystal *lcd;
        int charSlot;
        int posX;
        int posY;
        
    public:
        WifiMeter(LiquidCrystal *l, int slot, int x, int y);
        void wifiStrength();
        void print();
};


