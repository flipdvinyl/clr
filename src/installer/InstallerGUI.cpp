#include "InstallerGUI.h"

InstallerGUI::InstallerGUI() {
    nextButton = std::make_unique<juce::TextButton>("다음");
    nextButton->addListener(this);
    addAndMakeVisible(nextButton.get());
    
    cancelButton = std::make_unique<juce::TextButton>("취소");
    cancelButton->addListener(this);
    addAndMakeVisible(cancelButton.get());
    
    statusLabel = std::make_unique<juce::Label>("status", "clr 설치를 시작합니다.");
    statusLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel.get());
    
    progressBar = std::make_unique<juce::ProgressBar>(progress);
    addAndMakeVisible(progressBar.get());
    
    nextStep();
}

InstallerGUI::~InstallerGUI() = default;

void InstallerGUI::buttonClicked(juce::Button* button) {
    if (button == nextButton.get()) {
        nextStep();
    } else if (button == cancelButton.get()) {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
}

void InstallerGUI::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::white);
    
    // 단계별 설명 텍스트
    juce::String stepText;
    switch (currentStep) {
        case 0:
            stepText = "clr 설치를 시작합니다.\n\n이 마법사는 다음 항목들을 자동으로 설치합니다:\n\n• BlackHole 2ch (오디오 드라이버)\n• switchaudio-osx (오디오 제어 도구)\n• 시스템 권한 설정\n\n계속하시겠습니까?";
            break;
        case 1:
            stepText = "시스템 요구사항을 확인합니다...\n\n• macOS 10.14 이상\n• 500MB 이상의 여유 공간\n• 인터넷 연결";
            break;
        case 2:
            stepText = "Command Line Tools를 설치합니다...\n\nApple 개발 도구가 필요합니다.";
            break;
        case 3:
            stepText = "Homebrew를 설치합니다...\n\n패키지 관리자가 필요합니다.";
            break;
        case 4:
            stepText = "BlackHole 2ch를 설치합니다...\n\n이 오디오 드라이버는 시스템 오디오를 캡처하는데 필요합니다.";
            break;
        case 5:
            stepText = "switchaudio-osx를 설치합니다...\n\n이 도구는 오디오 장치를 제어하는데 필요합니다.";
            break;
        case 6:
            stepText = "시스템 권한을 설정합니다...\n\n시스템 환경설정이 열립니다. clr에 권한을 부여해주세요.";
            break;
        case 7:
            stepText = "설치를 검증합니다...\n\n모든 구성 요소가 올바르게 설치되었는지 확인합니다.";
            break;
        case 8:
            stepText = "설치가 완료되었습니다!\n\nclr를 실행할 준비가 되었습니다.";
            break;
    }
    
    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText(stepText, getLocalBounds().reduced(20), juce::Justification::centred);
}

void InstallerGUI::resized() {
    auto bounds = getLocalBounds().reduced(20);
    
    statusLabel->setBounds(bounds.removeFromTop(200));
    progressBar->setBounds(bounds.removeFromTop(30));
    
    auto buttonArea = bounds.removeFromBottom(50);
    cancelButton->setBounds(buttonArea.removeFromLeft(100));
    nextButton->setBounds(buttonArea.removeFromRight(100));
}

void InstallerGUI::startInstallation() {
    isInstalling = true;
    progress = 0.0;
    
    // 백그라운드에서 설치 실행
    juce::Thread::launch([this]() {
        bool success = InstallerManager::runInstallation();
        
        // UI 업데이트를 메인 스레드에서 실행
        juce::MessageManager::callAsync([this, success]() {
            isInstalling = false;
            progress = 1.0;
            
            if (success) {
                showSuccess();
            } else {
                showError("설치에 실패했습니다.");
            }
        });
    });
}

void InstallerGUI::nextStep() {
    currentStep++;
    
    if (currentStep == 1) {
        startInstallation();
    } else if (currentStep == 8) {
        nextButton->setButtonText("완료");
        nextButton->addListener(this);
        
        // clr 앱 실행
        juce::String command = "open /Applications/clr.app";
        system(command.toRawUTF8());
        
        // 설치 마법사 종료
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
    repaint();
}

void InstallerGUI::updateProgress(double newProgress) {
    progress = newProgress;
    repaint();
}

void InstallerGUI::showError(const juce::String& error) {
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::WarningIcon,
        "설치 오류",
        error,
        "확인"
    );
}

void InstallerGUI::showSuccess() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "설치 완료",
        "clr 설치가 성공적으로 완료되었습니다!",
        "확인"
    );
} 