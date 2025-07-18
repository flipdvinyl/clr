#pragma once
#include <JuceHeader.h>
#include "InstallerManager.h"

class InstallerGUI : public juce::Component, public juce::Button::Listener {
public:
    InstallerGUI();
    ~InstallerGUI() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void startInstallation();
    
    // Button::Listener override
    void buttonClicked(juce::Button* button) override;
    
private:
    void nextStep();
    void updateProgress(double progress);
    void showError(const juce::String& error);
    void showSuccess();
    
    std::unique_ptr<juce::TextButton> nextButton;
    std::unique_ptr<juce::TextButton> cancelButton;
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::ProgressBar> progressBar;
    
    int currentStep = 0;
    bool isInstalling = false;
    double progress = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstallerGUI)
}; 