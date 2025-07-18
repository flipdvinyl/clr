#pragma once
#include <JuceHeader.h>

class BlackHoleInstaller {
public:
    static bool installBlackHole();
    static bool checkBlackHoleInstalled();
    
private:
    static bool installWithHomebrew();
    static bool installManually();
    static bool checkHomebrew();
}; 