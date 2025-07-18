#pragma once
#include <JuceHeader.h>

class PermissionManager {
public:
    static bool requestAllPermissions();
    static bool checkAllPermissions();
    
private:
    static bool requestAccessibilityPermission();
    static bool requestMicrophonePermission();
    static bool requestAudioDevicePermission();
    static bool checkAccessibilityPermission();
    static bool checkMicrophonePermission();
    static bool checkAudioDevicePermission();
    static bool waitForPermissionGrant();
}; 