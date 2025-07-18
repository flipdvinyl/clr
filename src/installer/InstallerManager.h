#pragma once
#include <JuceHeader.h>

class InstallerManager {
public:
    static bool runInstallation();
    static bool checkSystemRequirements();
    static bool installDependencies();
    static bool setupPermissions();
    static bool verifyInstallation();
    
private:
    static bool installCommandLineTools();
    static bool installHomebrew();
    static bool installBlackHole();
    static bool installSwitchAudio();
    static bool requestSystemPermissions();
    static bool checkMacOSVersion();
    static bool checkDiskSpace();
    static bool checkNetworkAccess();
}; 