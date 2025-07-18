#pragma once
#include <JuceHeader.h>

class SystemChecker {
public:
    static bool checkSwitchAudioInstalled();
    static bool checkHomebrewInstalled();
    static bool checkCommandLineToolsInstalled();
    static bool checkBlackHoleInstalled();
    static bool checkAllRequirements();
    
private:
    static juce::String getSystemVersion();
    static bool checkDiskSpace(juce::int64 requiredBytes);
    static bool checkNetworkConnectivity();
}; 