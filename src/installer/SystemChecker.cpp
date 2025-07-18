#include "SystemChecker.h"

bool SystemChecker::checkSwitchAudioInstalled() {
    juce::String command = "which switchaudio-osx";
    return system(command.toRawUTF8()) == 0;
}

bool SystemChecker::checkHomebrewInstalled() {
    juce::String command = "which brew";
    return system(command.toRawUTF8()) == 0;
}

bool SystemChecker::checkCommandLineToolsInstalled() {
    juce::String command = "xcode-select -p";
    return system(command.toRawUTF8()) == 0;
}

bool SystemChecker::checkBlackHoleInstalled() {
    juce::String command = "system_profiler SPAudioDataType | grep -q 'BlackHole'";
    return system(command.toRawUTF8()) == 0;
}

bool SystemChecker::checkAllRequirements() {
    juce::Logger::writeToLog("시스템 요구사항 확인 중...");
    
    // macOS 버전 확인
    juce::String version = getSystemVersion();
    juce::Logger::writeToLog("macOS 버전: " + version);
    
    // 디스크 공간 확인
    if (!checkDiskSpace(500 * 1024 * 1024)) { // 500MB
        juce::Logger::writeToLog("디스크 공간 부족");
        return false;
    }
    
    // 네트워크 연결 확인
    if (!checkNetworkConnectivity()) {
        juce::Logger::writeToLog("네트워크 연결 없음");
        return false;
    }
    
    // Command Line Tools 확인
    if (!checkCommandLineToolsInstalled()) {
        juce::Logger::writeToLog("Command Line Tools 미설치");
        return false;
    }
    
    // Homebrew 확인
    if (!checkHomebrewInstalled()) {
        juce::Logger::writeToLog("Homebrew 미설치");
        return false;
    }
    
    // BlackHole 확인
    if (!checkBlackHoleInstalled()) {
        juce::Logger::writeToLog("BlackHole 2ch 미설치");
        return false;
    }
    
    // switchaudio-osx 확인
    if (!checkSwitchAudioInstalled()) {
        juce::Logger::writeToLog("switchaudio-osx 미설치");
        return false;
    }
    
    juce::Logger::writeToLog("모든 시스템 요구사항 충족");
    return true;
}

juce::String SystemChecker::getSystemVersion() {
    juce::String command = "sw_vers -productVersion";
    FILE* pipe = popen(command.toRawUTF8(), "r");
    if (!pipe) return "Unknown";
    
    char buffer[128];
    juce::String result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    
    return result.trim();
}

bool SystemChecker::checkDiskSpace(juce::int64 requiredBytes) {
    juce::File appDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    juce::int64 freeSpace = appDir.getBytesFreeOnVolume();
    return freeSpace >= requiredBytes;
}

bool SystemChecker::checkNetworkConnectivity() {
    juce::String command = "ping -c 1 8.8.8.8";
    return system(command.toRawUTF8()) == 0;
} 