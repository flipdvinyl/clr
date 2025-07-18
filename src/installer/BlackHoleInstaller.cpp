#include "BlackHoleInstaller.h"

bool BlackHoleInstaller::installBlackHole() {
    juce::Logger::writeToLog("BlackHole 2ch 설치 시작...");
    
    // 이미 설치되어 있는지 확인
    if (checkBlackHoleInstalled()) {
        juce::Logger::writeToLog("BlackHole 2ch가 이미 설치되어 있습니다.");
        return true;
    }
    
    // Homebrew로 설치 시도
    if (checkHomebrew()) {
        if (installWithHomebrew()) {
            return true;
        }
    }
    
    // 수동 설치 시도
    return installManually();
}

bool BlackHoleInstaller::checkBlackHoleInstalled() {
    juce::String command = "system_profiler SPAudioDataType | grep -q 'BlackHole'";
    return system(command.toRawUTF8()) == 0;
}

bool BlackHoleInstaller::installWithHomebrew() {
    juce::Logger::writeToLog("Homebrew로 BlackHole 2ch 설치 중...");
    
    juce::String command = "brew install blackhole-2ch";
    int result = system(command.toRawUTF8());
    
    if (result == 0) {
        juce::Logger::writeToLog("Homebrew로 BlackHole 2ch 설치 성공");
        return true;
    }
    
    juce::Logger::writeToLog("Homebrew로 BlackHole 2ch 설치 실패");
    return false;
}

bool BlackHoleInstaller::installManually() {
    juce::Logger::writeToLog("수동으로 BlackHole 2ch 설치 중...");
    
    // BlackHole 다운로드
    juce::String downloadCommand = "curl -L -o /tmp/blackhole.pkg https://github.com/ExistentialAudio/BlackHole/releases/download/v0.3.2/BlackHole.v0.3.2.pkg";
    if (system(downloadCommand.toRawUTF8()) != 0) {
        juce::Logger::writeToLog("BlackHole 다운로드 실패");
        return false;
    }
    
    // BlackHole 설치
    juce::String installCommand = "sudo installer -pkg /tmp/blackhole.pkg -target /";
    int result = system(installCommand.toRawUTF8());
    
    // 임시 파일 정리
    system("rm /tmp/blackhole.pkg");
    
    if (result == 0) {
        juce::Logger::writeToLog("수동으로 BlackHole 2ch 설치 성공");
        return true;
    }
    
    juce::Logger::writeToLog("수동으로 BlackHole 2ch 설치 실패");
    return false;
}

bool BlackHoleInstaller::checkHomebrew() {
    juce::String command = "which brew";
    return system(command.toRawUTF8()) == 0;
} 