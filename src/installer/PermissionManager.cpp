#include "PermissionManager.h"

bool PermissionManager::requestAllPermissions() {
    juce::Logger::writeToLog("모든 권한 요청 중...");
    
    return requestAccessibilityPermission() &&
           requestMicrophonePermission() &&
           requestAudioDevicePermission();
}

bool PermissionManager::checkAllPermissions() {
    return checkAccessibilityPermission() &&
           checkMicrophonePermission() &&
           checkAudioDevicePermission();
}

bool PermissionManager::requestAccessibilityPermission() {
    if (checkAccessibilityPermission()) {
        return true;
    }
    
    // 사용자에게 권한 설정 안내
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "시스템 권한 설정",
        "clr가 시스템을 제어하려면 접근성 권한이 필요합니다.\n\n"
        "시스템 환경설정이 열립니다. 다음 단계를 따라해주세요:\n\n"
        "1. '보안 및 개인 정보 보호' 클릭\n"
        "2. '개인 정보 보호' 탭 클릭\n"
        "3. 왼쪽 목록에서 '접근성' 선택\n"
        "4. clr에 체크 표시\n"
        "5. 설정 완료 후 '확인' 클릭",
        "확인"
    );
    
    // 시스템 환경설정 열기
    juce::String command = "open 'x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility'";
    system(command.toRawUTF8());
    
        // 권한 확인 대기
    return waitForPermissionGrant();
}

bool PermissionManager::requestMicrophonePermission() {
    // 실제 마이크 접근을 시도하여 권한 요청
    juce::AudioDeviceManager deviceManager;
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    if (deviceType) {
        juce::StringArray inputDevices = deviceType->getDeviceNames(true);
    
        if (inputDevices.isEmpty()) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "마이크 권한",
                "마이크 접근 권한이 필요합니다.\n\n"
                "시스템 환경설정 > 보안 및 개인 정보 보호 > 개인 정보 보호 > 마이크에서 clr에 권한을 부여해주세요.",
                "확인"
            );
            return false;
        }
        
        return true;
    }
    
    return false;
}

bool PermissionManager::requestAudioDevicePermission() {
    // 오디오 장치 변경 권한 확인
    juce::String command = "osascript -e 'tell application \"System Events\" to keystroke \"a\"'";
    return system(command.toRawUTF8()) == 0;
}

bool PermissionManager::checkAccessibilityPermission() {
    // 접근성 권한 확인
    juce::String command = "osascript -e 'tell application \"System Events\" to keystroke \"a\"'";
    return system(command.toRawUTF8()) == 0;
}

bool PermissionManager::checkMicrophonePermission() {
    // 마이크 권한 확인
    juce::AudioDeviceManager deviceManager;
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    if (deviceType) {
        juce::StringArray inputDevices = deviceType->getDeviceNames(true);
        return !inputDevices.isEmpty();
    }
    return false;
}

bool PermissionManager::checkAudioDevicePermission() {
    // 오디오 장치 권한 확인
    juce::String command = "osascript -e 'tell application \"System Events\" to keystroke \"a\"'";
    return system(command.toRawUTF8()) == 0;
}

bool PermissionManager::waitForPermissionGrant() {
    // 사용자가 권한을 부여할 때까지 대기
    for (int i = 0; i < 60; i++) { // 60초 대기
        if (checkAccessibilityPermission()) {
            return true;
        }
        juce::Thread::sleep(1000);
    }
    return false;
} 