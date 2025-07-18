#include "InstallerManager.h"
#include "BlackHoleInstaller.h"
#include "PermissionManager.h"
#include "SystemChecker.h"

bool InstallerManager::runInstallation() {
    juce::Logger::writeToLog("=== clr 설치 시작 ===");
    
    return checkSystemRequirements() &&
           installDependencies() &&
           setupPermissions() &&
           verifyInstallation();
}

bool InstallerManager::checkSystemRequirements() {
    juce::Logger::writeToLog("시스템 요구사항 확인 중...");
    
    if (!checkMacOSVersion()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "시스템 요구사항",
            "clr는 macOS 10.14 (Mojave) 이상이 필요합니다.",
            "확인"
        );
        return false;
    }
    
    if (!checkDiskSpace()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "디스크 공간 부족",
            "설치를 위해 최소 500MB의 여유 공간이 필요합니다.",
            "확인"
        );
        return false;
    }
    
    if (!checkNetworkAccess()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "네트워크 연결 오류",
            "인터넷 연결이 필요합니다.",
            "확인"
        );
        return false;
    }
    
    return true;
}

bool InstallerManager::installDependencies() {
    juce::Logger::writeToLog("의존성 설치 중...");
    
    if (!installCommandLineTools()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "설치 오류",
            "Command Line Tools 설치에 실패했습니다.",
            "확인"
        );
        return false;
    }
    
    if (!installHomebrew()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "설치 오류",
            "Homebrew 설치에 실패했습니다.",
            "확인"
        );
        return false;
    }
    
    if (!installBlackHole()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "설치 오류",
            "BlackHole 2ch 설치에 실패했습니다.",
            "확인"
        );
        return false;
    }
    
    if (!installSwitchAudio()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "설치 오류",
            "switchaudio-osx 설치에 실패했습니다.",
            "확인"
        );
        return false;
    }
    
    return true;
}

bool InstallerManager::setupPermissions() {
    juce::Logger::writeToLog("시스템 권한 설정 중...");
    
    if (!requestSystemPermissions()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "권한 설정 오류",
            "시스템 권한 설정에 실패했습니다.",
            "확인"
        );
        return false;
    }
    
    return true;
}

bool InstallerManager::verifyInstallation() {
    juce::Logger::writeToLog("설치 검증 중...");
    
    juce::String report;
    bool allGood = true;
    
    // BlackHole 확인
    if (!BlackHoleInstaller::checkBlackHoleInstalled()) {
        report += "❌ BlackHole 2ch 미설치\n";
        allGood = false;
    } else {
        report += "✅ BlackHole 2ch 설치됨\n";
    }
    
    // switchaudio-osx 확인
    if (!SystemChecker::checkSwitchAudioInstalled()) {
        report += "❌ switchaudio-osx 미설치\n";
        allGood = false;
    } else {
        report += "✅ switchaudio-osx 설치됨\n";
    }
    
    // 권한 확인
    if (!PermissionManager::checkAllPermissions()) {
        report += "❌ 시스템 권한 미설정\n";
        allGood = false;
    } else {
        report += "✅ 시스템 권한 설정됨\n";
    }
    
    // 결과 표시
    juce::AlertWindow::showMessageBoxAsync(
        allGood ? juce::MessageBoxIconType::InfoIcon : juce::MessageBoxIconType::WarningIcon,
        "설치 검증 결과",
        report,
        "확인"
    );
    
    return allGood;
}

bool InstallerManager::installCommandLineTools() {
    juce::String command = "xcode-select -p";
    if (system(command.toRawUTF8()) == 0) {
        return true; // 이미 설치됨
    }
    
    command = "xcode-select --install";
    return system(command.toRawUTF8()) == 0;
}

bool InstallerManager::installHomebrew() {
    juce::String command = "which brew";
    if (system(command.toRawUTF8()) == 0) {
        return true; // 이미 설치됨
    }
    
    command = "/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"";
    return system(command.toRawUTF8()) == 0;
}

bool InstallerManager::installBlackHole() {
    return BlackHoleInstaller::installBlackHole();
}

bool InstallerManager::installSwitchAudio() {
    juce::String command = "brew install switchaudio-osx";
    return system(command.toRawUTF8()) == 0;
}

bool InstallerManager::requestSystemPermissions() {
    return PermissionManager::requestAllPermissions();
}

bool InstallerManager::checkMacOSVersion() {
    juce::String command = "sw_vers -productVersion";
    FILE* pipe = popen(command.toRawUTF8(), "r");
    if (!pipe) return false;
    
    char buffer[128];
    juce::String result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    
    // macOS 10.14 이상 확인
    juce::StringArray parts = juce::StringArray::fromTokens(result, ".", "");
    if (parts.size() >= 2) {
        int major = parts[0].getIntValue();
        int minor = parts[1].getIntValue();
        return major > 10 || (major == 10 && minor >= 14);
    }
    
    return false;
}

bool InstallerManager::checkDiskSpace() {
    juce::File appDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    juce::int64 freeSpace = appDir.getBytesFreeOnVolume();
    return freeSpace >= 500 * 1024 * 1024; // 500MB
}

bool InstallerManager::checkNetworkAccess() {
    juce::String command = "ping -c 1 8.8.8.8";
    return system(command.toRawUTF8()) == 0;
} 