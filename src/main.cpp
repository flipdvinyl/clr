#include <JuceHeader.h>

class ClearHostApp : public juce::AudioAppComponent, public juce::AudioProcessorPlayer, public juce::Slider::Listener, public juce::AudioProcessorListener, public juce::ComboBox::Listener, public juce::Button::Listener, public juce::Timer {
public:
    ClearHostApp() {
            // 애니메이션 설정
    animationDuration = 1.0; // 1초 (초기 변수)
    animationTimerInterval = 16; // 60fps (16ms 간격)
    
    // 시스템 사운드 출력 소스 저장 변수 초기화
    originalSystemOutputDevice = "";
    
    // 자동 초기 설정
    performAutoSetup();
        
        pluginManager.addDefaultFormats();
        loadClearVST3();
        
        // 오디오 채널 설정 - BlackHole은 보통 스테레오 출력을 제공
        setAudioChannels(2, 2); // stereo in, stereo out for BlackHole compatibility
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        if (!midiInputs.isEmpty()) {
            midiInput = juce::MidiInput::openDevice(midiInputs[0].identifier, this);
            if (midiInput) midiInput->start();
        }
        setProcessor(clearPlugin.get());
        
        // 노브 컨트롤들 생성
        for (int i = 0; i < 3; ++i) {
            auto knob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            knob->setRange(0.0, 1.0, 0.01);
            knob->setValue(0.5);
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            knob->addListener(this);
            knobs.add(knob.release());
            addAndMakeVisible(knobs.getLast());
        }
        
        // 프리셋 버튼들 생성
        bypassButton = std::make_unique<juce::TextButton>("Bypass");
        ambientRoomButton = std::make_unique<juce::TextButton>("S* up**");
        tooLoudButton = std::make_unique<juce::TextButton>("Too Loud");
        muteButton = std::make_unique<juce::TextButton>("Silence");
        clearVoiceButton = std::make_unique<juce::TextButton>("Clear Voice");
        dryVoiceButton = std::make_unique<juce::TextButton>("Dry Voice");
        vocalReferenceButton = std::make_unique<juce::TextButton>("Vocal Reference");
        stereoMonoButton = std::make_unique<juce::TextButton>("Stereo");
        
        bypassButton->addListener(this);
        muteButton->addListener(this);
        clearVoiceButton->addListener(this);
        ambientRoomButton->addListener(this);
        vocalReferenceButton->addListener(this);
        dryVoiceButton->addListener(this);
        tooLoudButton->addListener(this);
        stereoMonoButton->addListener(this);
        
        addAndMakeVisible(bypassButton.get());
        addAndMakeVisible(muteButton.get());
        addAndMakeVisible(clearVoiceButton.get());
        addAndMakeVisible(ambientRoomButton.get());
        addAndMakeVisible(vocalReferenceButton.get());
        addAndMakeVisible(dryVoiceButton.get());
        addAndMakeVisible(tooLoudButton.get());
        addAndMakeVisible(stereoMonoButton.get());
        
        // 오디오 입출력 드롭다운 메뉴 생성
        inputDeviceBox = std::make_unique<juce::ComboBox>();
        outputDeviceBox = std::make_unique<juce::ComboBox>();
        
        inputDeviceBox->addListener(this);
        outputDeviceBox->addListener(this);
        
        addAndMakeVisible(inputDeviceBox.get());
        addAndMakeVisible(outputDeviceBox.get());
        
        // 오디오 장치 목록 업데이트
        updateAudioDeviceLists();
        
        // 라벨 설정
        inputDeviceLabel.setText("Audio Input:", juce::dontSendNotification);
        inputDeviceLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(inputDeviceLabel);
        
        outputDeviceLabel.setText("Audio Output:", juce::dontSendNotification);
        outputDeviceLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(outputDeviceLabel);
        
        if (clearPlugin) {
            juce::Logger::writeToLog("Creating plugin editor...");
            pluginEditor.reset(clearPlugin->createEditor());
            if (pluginEditor) {
                juce::Logger::writeToLog("Plugin editor created successfully");
                juce::Logger::writeToLog("Editor size: " + juce::String(pluginEditor->getWidth()) + "x" + juce::String(pluginEditor->getHeight()));
                addAndMakeVisible(pluginEditor.get());
                
                // Footer 영역 포함한 전체 크기 설정
                int totalHeight = pluginEditor->getHeight() + 300; // Footer 300px 추가
                setSize(pluginEditor->getWidth(), totalHeight);
                
                // 플러그인 파라미터 리스너 추가
                clearPlugin->addListener(this);
                
                // Clear 플러그인의 모든 파라미터 이름과 값 출력
                juce::Logger::writeToLog("=== Clear Plugin Parameters ===");
                auto params = clearPlugin->getParameters();
                for (int i = 0; i < params.size(); ++i) {
                    if (params[i]) {
                        juce::String paramName = params[i]->getName(100);
                        float value = params[i]->getValue();
                        juce::Logger::writeToLog("Parameter " + juce::String(i) + ": " + paramName + " = " + juce::String(value));
                        
                        // Voice Gain 관련 파라미터 찾기
                        if (paramName.contains("Voice") && (paramName.contains("Gain") || paramName.contains("Level") || paramName.contains("Volume") || paramName.contains("Amount"))) {
                            juce::Logger::writeToLog("*** FOUND VOICE GAIN: Parameter " + juce::String(i) + " = " + paramName + " ***");
                        }
                    }
                }
                juce::Logger::writeToLog("=== End Parameters ===");
                
                // 초기 노브 값들을 플러그인 파라미터와 동기화
                updateKnobsFromPlugin();
                
                // Mono/Stereo 파라미터를 Stereo로 설정 (Parameter 13)
                if (params.size() > 13 && params[13]) {
                    params[13]->setValueNotifyingHost(1.0f); // Stereo = 1.0
                    juce::Logger::writeToLog("Set Mono/Stereo parameter to Stereo (1.0)");
                    
                    // 스테레오/모노 버튼 초기 상태 설정
                    if (stereoMonoButton) {
                        stereoMonoButton->setButtonText("Stereo");
                    }
                }
            } else {
                juce::Logger::writeToLog("Failed to create plugin editor");
                setSize(400, 500); // 기본 크기 + Footer
            }
        } else {
            juce::Logger::writeToLog("No plugin loaded, setting default window size");
            setSize(400, 500); // 기본 크기 + Footer
        }
    }
    ~ClearHostApp() override {
        if (midiInput) midiInput->stop();
        stopTimer();
        shutdownAudio();
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        if (clearPlugin) clearPlugin->prepareToPlay(sampleRate, samplesPerBlockExpected);
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (clearPlugin) {
            juce::MidiBuffer midiMessages;
            clearPlugin->processBlock(*bufferToFill.buffer, midiMessages);
            
            // 디버깅: 오디오 레벨 확인
            static int debugCounter = 0;
            if (++debugCounter % 4410 == 0) { // 1초마다 한 번씩
                float maxLevel = 0.0f;
                for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch) {
                    auto* in = bufferToFill.buffer->getReadPointer(ch);
                    for (int i = 0; i < bufferToFill.numSamples; ++i) {
                        maxLevel = juce::jmax(maxLevel, std::abs(in[i]));
                    }
                }
                if (maxLevel > 0.001f) {
                    juce::Logger::writeToLog("Audio input detected! Max level: " + juce::String(maxLevel));
                }
            }
        } else {
            for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch) {
                auto* in = bufferToFill.buffer->getReadPointer(ch);
                auto* out = bufferToFill.buffer->getWritePointer(ch);
                if (ch < bufferToFill.buffer->getNumChannels())
                    juce::FloatVectorOperations::copy(out, in, bufferToFill.numSamples);
                else
                    juce::FloatVectorOperations::clear(out, bufferToFill.numSamples);
            }
        }
    }
    void releaseResources() override {
        if (clearPlugin) clearPlugin->releaseResources();
    }
    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& message) override {
        if (message.isController() && clearPlugin) mapMidiCCToClearParameter(message);
    }
    void resized() override {
        try {
            if (pluginEditor) {
                // 플러그인 에디터는 상단에 배치
                pluginEditor->setBounds(0, 0, getWidth(), getHeight() - 350);
            }
            
            // Footer 영역에 노브들 배치
            int footerY = getHeight() - 350;
            int knobWidth = 120;
            int knobHeight = 120;
            int spacing = (getWidth() - (3 * knobWidth)) / 4;
            
            for (int i = 0; i < knobs.size(); ++i) {
                int x = spacing + i * (knobWidth + spacing);
                int y = footerY + 50;
                knobs[i]->setBounds(x, y, knobWidth, knobHeight);
            }
            
            // 프리셋 버튼들 (노브 아래쪽) - 3줄로 배치
            int presetY = footerY + 180;
            int buttonWidth = 100;
            int buttonHeight = 30;
            int buttonSpacing = (getWidth() - (3 * buttonWidth)) / 4;
            
            // 첫 번째 줄: 3개 버튼 (Bypass, S* up**, Too Loud)
            if (bypassButton) bypassButton->setBounds(buttonSpacing, presetY, buttonWidth, buttonHeight);
            if (ambientRoomButton) ambientRoomButton->setBounds(buttonSpacing + buttonWidth + buttonSpacing, presetY, buttonWidth, buttonHeight);
            if (tooLoudButton) tooLoudButton->setBounds(buttonSpacing + 2 * (buttonWidth + buttonSpacing), presetY, buttonWidth, buttonHeight);
            
            // 두 번째 줄: 3개 버튼 (Silence, Clear Voice, Dry Voice)
            int secondRowY = presetY + buttonHeight + 10;
            int secondRowSpacing = (getWidth() - (3 * buttonWidth)) / 4;
            if (muteButton) muteButton->setBounds(secondRowSpacing, secondRowY, buttonWidth, buttonHeight);
            if (clearVoiceButton) clearVoiceButton->setBounds(secondRowSpacing + buttonWidth + secondRowSpacing, secondRowY, buttonWidth, buttonHeight);
            if (dryVoiceButton) dryVoiceButton->setBounds(secondRowSpacing + 2 * (buttonWidth + secondRowSpacing), secondRowY, buttonWidth, buttonHeight);
            
            // 세 번째 줄: 1개 버튼 (Vocal Reference - 중앙 정렬)
            int thirdRowY = secondRowY + buttonHeight + 10;
            int thirdRowX = (getWidth() - buttonWidth) / 2;
            if (vocalReferenceButton) vocalReferenceButton->setBounds(thirdRowX, thirdRowY, buttonWidth, buttonHeight);
            
            // 오디오 장치 선택 영역 (프리셋 버튼 아래쪽)
            int deviceY = footerY + 280; // 3줄 버튼을 위해 위치 조정
            int deviceWidth = getWidth() / 2;
            
            // 입력 장치
            inputDeviceLabel.setBounds(20, deviceY, deviceWidth - 40, 20);
            if (inputDeviceBox) inputDeviceBox->setBounds(20, deviceY + 25, deviceWidth - 40, 25);
            
            // 출력 장치
            outputDeviceLabel.setBounds(deviceWidth + 20, deviceY, deviceWidth - 40, 20);
            if (outputDeviceBox) outputDeviceBox->setBounds(deviceWidth + 20, deviceY + 25, deviceWidth - 40, 25);
            
            // 스테레오/모노 버튼 (footer 제일 아래)
            int stereoMonoY = deviceY + 60;
            int stereoMonoWidth = 120;
            int stereoMonoHeight = 30;
            int stereoMonoX = (getWidth() - stereoMonoWidth) / 2;
            if (stereoMonoButton) stereoMonoButton->setBounds(stereoMonoX, stereoMonoY, stereoMonoWidth, stereoMonoHeight);
        } catch (...) {
            juce::Logger::writeToLog("Error in resized()");
        }
    }
    
    void sliderValueChanged(juce::Slider* slider) override {
        if (!clearPlugin) return;
        
        // 어떤 노브가 변경되었는지 찾기
        int knobIndex = -1;
        for (int i = 0; i < knobs.size(); ++i) {
            if (knobs[i] == slider) {
                knobIndex = i;
                break;
            }
        }
        
        if (knobIndex >= 0 && knobIndex < 3) {
            // Clear 플러그인의 파라미터를 인덱스로 직접 매핑
            auto params = clearPlugin->getParameters();
            int targetParamIndex = -1;
            
            switch (knobIndex) {
                case 0: targetParamIndex = 1; break;  // Ambience Gain
                case 1: targetParamIndex = 14; break; // Voice Gain
                case 2: targetParamIndex = 12; break; // Voice Reverb Gain
            }
            
            if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                float value = (float)slider->getValue();
                params[targetParamIndex]->setValueNotifyingHost(value);
                juce::Logger::writeToLog("Knob " + juce::String(knobIndex) + " -> Parameter " + juce::String(targetParamIndex) + " (" + params[targetParamIndex]->getName(100) + ") = " + juce::String(value));
            }
        }
    }
    
    void audioProcessorParameterChanged(juce::AudioProcessor*, int parameterIndex, float newValue) override {
        // 플러그인 파라미터가 변경되면 노브 업데이트 (인덱스 기반)
        if (!clearPlugin) return;
        
        // 특정 파라미터 인덱스에 해당하는 노브 업데이트
        switch (parameterIndex) {
            case 1:  // Ambience Gain
                knobs[0]->setValue(newValue, juce::dontSendNotification);
                break;
            case 14: // Voice Gain
                knobs[1]->setValue(newValue, juce::dontSendNotification);
                break;
            case 12: // Voice Reverb Gain
                knobs[2]->setValue(newValue, juce::dontSendNotification);
                break;
            case 13: // Stereo/Mono
                if (stereoMonoButton) {
                    juce::String buttonText = (newValue > 0.5f) ? "Stereo" : "Mono";
                    stereoMonoButton->setButtonText(buttonText);
                }
                break;
        }
    }
    
    void audioProcessorChanged(juce::AudioProcessor*, const juce::AudioProcessorListener::ChangeDetails&) override {
        // 플러그인이 변경되면 노브 업데이트
        updateKnobsFromPlugin();
    }
    
    void comboBoxChanged(juce::ComboBox* comboBox) override {
        if (comboBox == inputDeviceBox.get()) {
            int selectedId = inputDeviceBox->getSelectedId();
            if (selectedId > 0) {
                juce::String deviceName = inputDeviceBox->getText();
                juce::Logger::writeToLog("Selected input device: " + deviceName);
                
                // 비활성화된 BlackHole 옵션 선택 시 처리
                if (deviceName.contains("Not Installed")) {
                    juce::Logger::writeToLog("BlackHole not installed - showing info");
                    // 여기에 나중에 설치 안내 다이얼로그를 추가할 수 있습니다
                    // 현재는 기본 입력으로 되돌리기
                    inputDeviceBox->setSelectedId(1, juce::dontSendNotification);
                    return;
                }
                
                // OS Sound (BlackHole) 입력 선택 시 현재 시스템 출력 소스 저장
                if (deviceName.contains("OS Sound (BlackHole)")) {
                    juce::Logger::writeToLog("OS Sound input selected - saving current system output device");
                    saveCurrentSystemOutputDevice();
                } else {
                    // 다른 입력 소스 선택 시 저장된 시스템 출력 소스로 복구
                    juce::Logger::writeToLog("Non-OS Sound input selected - restoring original system output device");
                    restoreSystemOutputDevice();
                }
                
                // 실제 오디오 입력 장치 변경
                changeAudioInputDevice(deviceName);
            }
        } else if (comboBox == outputDeviceBox.get()) {
            int selectedId = outputDeviceBox->getSelectedId();
            if (selectedId > 0) {
                juce::String deviceName = outputDeviceBox->getText();
                juce::Logger::writeToLog("Selected output device: " + deviceName);
                // 실제 오디오 출력 장치 변경
                changeAudioOutputDevice(deviceName);
            }
        }
    }
    
    void buttonClicked(juce::Button* button) override {
        if (!clearPlugin) return;
        
        if (button == bypassButton.get()) {
            // Bypass: 모든 값 0.5
            startAnimation({0.5, 0.5, 0.5});
        } else if (button == muteButton.get()) {
            // Mute: 모든 값 0
            startAnimation({0.0, 0.0, 0.0});
        } else if (button == clearVoiceButton.get()) {
            // Clear Voice: Voice 만 1, 나머지 0
            startAnimation({0.0, 0.5, 0.5});
        } else if (button == ambientRoomButton.get()) {
            // Ambient Room: Ambience 만 0.5, 나머지 0
            startAnimation({0.5, 0.0, 0.0});
        } else if (button == vocalReferenceButton.get()) {
            // Vocal Reference Track: Ambience 0.5, Voice 0.1, Voice Reverb 0.1
            startAnimation({0.5, 0.1, 0.1});
        } else if (button == dryVoiceButton.get()) {
            // Dry Voice: Ambience 0, Voice 0.5, Voice Reverb 0
            startAnimation({0.0, 0.5, 0.0});
        } else if (button == tooLoudButton.get()) {
            // Too Loud: Ambience 0.5, Voice 0.25, Voice Reverb 0.25
            startAnimation({0.5, 0.2, 0.2});
        } else if (button == stereoMonoButton.get()) {
            // Stereo/Mono 토글
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                if (params.size() > 13 && params[13]) {
                    float currentValue = params[13]->getValue();
                    float newValue = (currentValue > 0.5f) ? 0.0f : 1.0f; // 토글
                    params[13]->setValueNotifyingHost(newValue);
                    
                    // 버튼 텍스트 업데이트
                    juce::String buttonText = (newValue > 0.5f) ? "Stereo" : "Mono";
                    stereoMonoButton->setButtonText(buttonText);
                    
                    juce::Logger::writeToLog("Toggled Stereo/Mono to: " + buttonText + " (value: " + juce::String(newValue) + ")");
                }
            }
        }
    }
    
    void startAnimation(const std::array<double, 3>& targetValues) {
        // 현재 값들을 시작 값으로 저장
        for (int i = 0; i < 3; ++i) {
            animationStartValues[i] = knobs[i]->getValue();
            animationTargetValues[i] = targetValues[i];
        }
        
        // 애니메이션 시작
        animationStartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        isAnimating = true;
        startTimer(animationTimerInterval);
        
        juce::Logger::writeToLog("Animation started: " + juce::String(animationStartValues[0]) + "," + 
                                juce::String(animationStartValues[1]) + "," + juce::String(animationStartValues[2]) + 
                                " -> " + juce::String(targetValues[0]) + "," + juce::String(targetValues[1]) + "," + juce::String(targetValues[2]));
    }
    
    void setKnobValue(int knobIndex, double value) {
        if (knobIndex >= 0 && knobIndex < knobs.size()) {
            knobs[knobIndex]->setValue(value, juce::dontSendNotification);
            
            // 플러그인 파라미터도 업데이트
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                int targetParamIndex = -1;
                
                switch (knobIndex) {
                    case 0: targetParamIndex = 1; break;  // Ambience Gain
                    case 1: targetParamIndex = 14; break; // Voice Gain
                    case 2: targetParamIndex = 12; break; // Voice Reverb Gain
                }
                
                if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                    params[targetParamIndex]->setValueNotifyingHost((float)value);
                }
            }
        }
    }
    
    void timerCallback() override {
        if (!isAnimating) return;
        
        double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        double elapsedTime = currentTime - animationStartTime;
        double progress = juce::jlimit(0.0, 1.0, elapsedTime / animationDuration);
        
        // 이징 함수 (부드러운 애니메이션)
        double easedProgress = 1.0 - std::pow(1.0 - progress, 3.0); // ease-out cubic
        
        // 각 노브 값 업데이트
        for (int i = 0; i < 3; ++i) {
            double currentValue = animationStartValues[i] + (animationTargetValues[i] - animationStartValues[i]) * easedProgress;
            setKnobValue(i, currentValue);
        }
        
        // 애니메이션 완료 체크
        if (progress >= 1.0) {
            isAnimating = false;
            stopTimer();
            juce::Logger::writeToLog("Animation completed");
        }
    }
    
    void updateKnobsFromPlugin() {
        if (!clearPlugin) return;
        
        auto params = clearPlugin->getParameters();
        
        // 특정 파라미터 인덱스의 값으로 노브 초기화
        if (params.size() > 1 && params[1]) {
            knobs[0]->setValue(params[1]->getValue(), juce::dontSendNotification); // Ambience Gain
        }
        if (params.size() > 14 && params[14]) {
            knobs[1]->setValue(params[14]->getValue(), juce::dontSendNotification); // Voice Gain
        }
        if (params.size() > 12 && params[12]) {
            knobs[2]->setValue(params[12]->getValue(), juce::dontSendNotification); // Voice Reverb Gain
        }
    }
    
    void restoreSystemOutputDevice() {
        if (originalSystemOutputDevice.isEmpty()) {
            juce::Logger::writeToLog("No saved system output device to restore");
            return;
        }
        
        juce::Logger::writeToLog("Restoring system output device to: " + originalSystemOutputDevice);
        
        // 시스템 출력을 저장된 장치로 복구하는 명령어들 시도 (AppleScript 제거)
        juce::StringArray commands = {
            "/opt/homebrew/bin/SwitchAudioSource -s \"" + originalSystemOutputDevice + "\"",
            "/usr/local/bin/SwitchAudioSource -s \"" + originalSystemOutputDevice + "\"",
            "/opt/homebrew/Cellar/switchaudio-osx/1.2.2/bin/SwitchAudioSource -s \"" + originalSystemOutputDevice + "\"",
            "SwitchAudioSource -s \"" + originalSystemOutputDevice + "\""
        };
        
        bool success = false;
        for (auto& command : commands) {
            int result = system(command.toRawUTF8());
            if (result == 0) {
                juce::Logger::writeToLog("Successfully restored system output using: " + command);
                success = true;
                break;
            }
        }
        
        if (!success) {
            juce::Logger::writeToLog("Failed to restore system output with all methods");
        }
    }
    
    // 시스템 출력 소스 복구 함수 (public으로 노출)

private:
    juce::AudioPluginFormatManager pluginManager;
    std::unique_ptr<juce::AudioPluginInstance> clearPlugin;
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::AudioProcessorEditor> pluginEditor;
    juce::OwnedArray<juce::Slider> knobs;
    std::unique_ptr<juce::TextButton> bypassButton;
    std::unique_ptr<juce::TextButton> muteButton;
    std::unique_ptr<juce::TextButton> clearVoiceButton;
    std::unique_ptr<juce::TextButton> ambientRoomButton;
    std::unique_ptr<juce::TextButton> vocalReferenceButton;
    std::unique_ptr<juce::TextButton> dryVoiceButton;
    std::unique_ptr<juce::TextButton> tooLoudButton;
    std::unique_ptr<juce::TextButton> stereoMonoButton;
    std::unique_ptr<juce::ComboBox> inputDeviceBox;
    std::unique_ptr<juce::ComboBox> outputDeviceBox;
    juce::Label inputDeviceLabel;
    juce::Label outputDeviceLabel;
    
    // 애니메이션 관련 변수들
    double animationDuration; // 애니메이션 지속 시간 (초)
    int animationTimerInterval; // 타이머 간격 (ms)
    bool isAnimating = false;
    double animationStartTime = 0.0;
    std::array<double, 3> animationStartValues = {0.0, 0.0, 0.0}; // 시작 값들
    std::array<double, 3> animationTargetValues = {0.0, 0.0, 0.0}; // 목표 값들
    
    // 시스템 사운드 출력 소스 저장
    juce::String originalSystemOutputDevice;
    
    // 첫 실행 여부 확인용 파일 경로
    juce::File firstRunFile;
    
    void loadClearVST3() {
        // VST3를 우선적으로 시도 (권한 문제 해결 후)
        juce::Logger::writeToLog("Trying to load Clear as VST3 (with permission fix)...");
        
        juce::File vst3Dir("/Library/Audio/Plug-Ins/VST3");
        juce::File clearVST3 = vst3Dir.getChildFile("Clear.vst3");
        if (clearVST3.exists()) {
            juce::Logger::writeToLog("Found Clear VST3: " + clearVST3.getFullPathName());
            juce::AudioPluginFormat* vst3Format = nullptr;
            for (int i = 0; i < pluginManager.getNumFormats(); ++i) {
                auto* format = pluginManager.getFormat(i);
                if (format && format->getName().contains("VST3")) {
                    vst3Format = format;
                    juce::Logger::writeToLog("Found VST3 format at index " + juce::String(i));
                    break;
                }
            }
            if (vst3Format) {
                juce::PluginDescription desc;
                desc.fileOrIdentifier = clearVST3.getFullPathName();
                desc.pluginFormatName = "VST3";
                desc.name = "Clear";
                desc.descriptiveName = "Clear";
                desc.manufacturerName = "Clear";
                desc.category = "Effect";
                desc.isInstrument = false;
                
                juce::String errorMessage;
                clearPlugin = vst3Format->createInstanceFromDescription(desc, 44100.0, 512, errorMessage);
                if (clearPlugin) {
                    juce::Logger::writeToLog("Clear VST3 loaded successfully!");
                } else {
                    juce::Logger::writeToLog("Failed to load Clear VST3: " + errorMessage);
                    
                    // VST3 실패시 AU로 폴백
                    juce::Logger::writeToLog("Falling back to AudioUnit...");
                    juce::File auDir("/Library/Audio/Plug-Ins/Components");
                    juce::File clearAU = auDir.getChildFile("Clear.component");
                    if (clearAU.exists()) {
                        juce::Logger::writeToLog("Found Clear AU: " + clearAU.getFullPathName());
                        juce::AudioPluginFormat* auFormat = nullptr;
                        for (int i = 0; i < pluginManager.getNumFormats(); ++i) {
                            auto* format = pluginManager.getFormat(i);
                            if (format && format->getName().contains("AudioUnit")) {
                                auFormat = format;
                                juce::Logger::writeToLog("Found AudioUnit format at index " + juce::String(i));
                                break;
                            }
                        }
                        if (auFormat) {
                            juce::PluginDescription auDesc;
                            auDesc.fileOrIdentifier = clearAU.getFullPathName();
                            auDesc.pluginFormatName = "AudioUnit";
                            auDesc.name = "Clear";
                            auDesc.descriptiveName = "Clear";
                            auDesc.manufacturerName = "Clear";
                            auDesc.category = "Effect";
                            auDesc.isInstrument = false;
                            
                            juce::String auError;
                            clearPlugin = auFormat->createInstanceFromDescription(auDesc, 44100.0, 512, auError);
                            if (clearPlugin) {
                                juce::Logger::writeToLog("Clear AU loaded successfully as fallback");
                            } else {
                                juce::Logger::writeToLog("Failed to load Clear AU: " + auError);
                            }
                        }
                    }
                }
            } else {
                juce::Logger::writeToLog("VST3 format not found");
            }
        } else {
            juce::Logger::writeToLog("Clear.vst3 not found");
        }
    }
    void updateAudioDeviceLists() {
        // 현재 활성화된 오디오 장치 타입 사용
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        
        // 입력 장치 목록 업데이트
        inputDeviceBox->clear();
        inputDeviceBox->addItem("Default Input", 1);
        
        bool blackHoleFound = false;
        
        if (deviceType) {
            try {
                auto inputNames = deviceType->getDeviceNames(true); // true = input devices
                juce::Logger::writeToLog("Found " + juce::String(inputNames.size()) + " input devices");
                
                for (int i = 0; i < inputNames.size(); ++i) {
                    juce::String deviceName = inputNames[i];
                    juce::Logger::writeToLog("Input device " + juce::String(i) + ": " + deviceName);
                    
                    // BlackHole을 "OS Sound"로 표시
                    if (deviceName.contains("BlackHole") || deviceName.contains("blackhole")) {
                        inputDeviceBox->addItem("OS Sound (BlackHole)", i + 2);
                        juce::Logger::writeToLog("Added OS Sound (BlackHole) option");
                        blackHoleFound = true;
                    } else {
                        inputDeviceBox->addItem(deviceName, i + 2);
                    }
                }
            } catch (...) {
                juce::Logger::writeToLog("Error getting input device names");
            }
        }
        
        // BlackHole이 없으면 비활성화된 옵션 추가
        if (!blackHoleFound) {
            int disabledItemId = 999; // 고유한 ID
            inputDeviceBox->addItem("OS Sound (BlackHole) - Not Installed", disabledItemId);
            inputDeviceBox->setItemEnabled(disabledItemId, false);
            juce::Logger::writeToLog("BlackHole not found - added disabled option");
        }
        
        // 출력 장치 목록 업데이트
        outputDeviceBox->clear();
        outputDeviceBox->addItem("Default Output", 1);
        
        if (deviceType) {
            try {
                auto outputNames = deviceType->getDeviceNames(false); // false = output devices
                juce::Logger::writeToLog("Found " + juce::String(outputNames.size()) + " output devices");
                
                for (int i = 0; i < outputNames.size(); ++i) {
                    juce::String deviceName = outputNames[i];
                    juce::Logger::writeToLog("Output device " + juce::String(i) + ": " + deviceName);
                    outputDeviceBox->addItem(deviceName, i + 2);
                }
                
                // 외장 헤드폰이 목록에 없으면 강제로 추가
                bool externalHeadphonesFound = false;
                for (int i = 0; i < outputNames.size(); ++i) {
                    if (outputNames[i].contains("외장 헤드폰") || 
                        outputNames[i].contains("External Headphones") ||
                        outputNames[i].contains("Headphones")) {
                        externalHeadphonesFound = true;
                        break;
                    }
                }
                
                if (!externalHeadphonesFound) {
                    juce::Logger::writeToLog("External headphones not found in device list - adding manually");
                    int manualId = 1000; // 고유한 ID
                    outputDeviceBox->addItem("외장 헤드폰 (Manual)", manualId);
                    juce::Logger::writeToLog("Added external headphones manually");
                }
                
            } catch (...) {
                juce::Logger::writeToLog("Error getting output device names");
            }
        }
        
        // 현재 선택된 장치 설정
        inputDeviceBox->setSelectedId(1, juce::dontSendNotification);
        outputDeviceBox->setSelectedId(1, juce::dontSendNotification);
    }
    
    void changeAudioInputDevice(const juce::String& deviceName) {
        juce::Logger::writeToLog("Changing input device to: " + deviceName);
        
        if (deviceName == "Default Input") {
            // 기본 입력 장치로 설정
            deviceManager.setCurrentAudioDeviceType("Core Audio", true);
            auto currentSetup = deviceManager.getAudioDeviceSetup();
            currentSetup.inputDeviceName = "";
            auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
            if (result.isNotEmpty()) {
                juce::Logger::writeToLog("Failed to set default input: " + result);
            } else {
                juce::Logger::writeToLog("Successfully set default input device");
            }
        } else if (deviceName == "OS Sound (BlackHole)") {
            // BlackHole 장치 찾기
            auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
            if (deviceType) {
                try {
                    auto inputNames = deviceType->getDeviceNames(true);
                    juce::Logger::writeToLog("Searching for BlackHole device among " + juce::String(inputNames.size()) + " input devices");
                    
                    for (int i = 0; i < inputNames.size(); ++i) {
                        juce::String actualDeviceName = inputNames[i];
                        juce::Logger::writeToLog("Checking device: " + actualDeviceName);
                        
                        if (actualDeviceName.contains("BlackHole") || actualDeviceName.contains("blackhole")) {
                            // 실제 BlackHole 장치로 설정
                            auto currentSetup = deviceManager.getAudioDeviceSetup();
                            currentSetup.inputDeviceName = actualDeviceName;
                            
                            // 오디오 재시작을 위해 일시 중지
                            deviceManager.closeAudioDevice();
                            
                            auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
                            if (result.isNotEmpty()) {
                                juce::Logger::writeToLog("Failed to change input device to BlackHole: " + result);
                            } else {
                                juce::Logger::writeToLog("Successfully connected to OS Sound (BlackHole)");
                                
                                // 시스템 출력 장치를 BlackHole로 설정
                                setSystemOutputToBlackHole();
                                
                                // 오디오 재시작
                                deviceManager.restartLastAudioDevice();
                            }
                            return;
                        }
                    }
                    juce::Logger::writeToLog("BlackHole device not found in available devices");
                } catch (...) {
                    juce::Logger::writeToLog("Error while searching for BlackHole device");
                }
            }
        } else {
            // 특정 입력 장치로 설정
            auto currentSetup = deviceManager.getAudioDeviceSetup();
            currentSetup.inputDeviceName = deviceName;
            
            // 오디오 재시작을 위해 일시 중지
            deviceManager.closeAudioDevice();
            
            auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
            if (result.isNotEmpty()) {
                juce::Logger::writeToLog("Failed to change input device to: " + deviceName + " (" + result + ")");
            } else {
                juce::Logger::writeToLog("Successfully changed input device to: " + deviceName);
                
                // 오디오 재시작
                deviceManager.restartLastAudioDevice();
            }
        }
    }
    
    void changeAudioOutputDevice(const juce::String& deviceName) {
        juce::Logger::writeToLog("Changing output device to: " + deviceName);
        
        if (deviceName == "Default Output") {
            // 기본 출력 장치로 설정
            deviceManager.setCurrentAudioDeviceType("Core Audio", true);
            auto currentSetup = deviceManager.getAudioDeviceSetup();
            currentSetup.outputDeviceName = "";
            auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
            if (result.isNotEmpty()) {
                juce::Logger::writeToLog("Failed to set default output: " + result);
            } else {
                juce::Logger::writeToLog("Successfully set default output device");
            }
        } else if (deviceName == "외장 헤드폰 (Manual)") {
            // 수동으로 추가된 외장 헤드폰 처리
            juce::Logger::writeToLog("Attempting to set external headphones manually");
            
            // 가능한 외장 헤드폰 이름들 시도
            juce::StringArray possibleNames = {
                "외장 헤드폰",
                "External Headphones", 
                "Headphones",
                "외장 헤드폰 (Built-in)",
                "External Headphones (Built-in)"
            };
            
            auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
            if (deviceType) {
                try {
                    auto outputNames = deviceType->getDeviceNames(false);
                    juce::Logger::writeToLog("Searching for external headphones among " + juce::String(outputNames.size()) + " output devices");
                    
                    for (int i = 0; i < outputNames.size(); ++i) {
                        juce::String actualDeviceName = outputNames[i];
                        juce::Logger::writeToLog("Checking output device: " + actualDeviceName);
                        
                        for (auto& possibleName : possibleNames) {
                            if (actualDeviceName.contains(possibleName)) {
                                // 실제 외장 헤드폰 장치로 설정
                                auto currentSetup = deviceManager.getAudioDeviceSetup();
                                currentSetup.outputDeviceName = actualDeviceName;
                                
                                // 오디오 재시작을 위해 일시 중지
                                deviceManager.closeAudioDevice();
                                
                                auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
                                if (result.isNotEmpty()) {
                                    juce::Logger::writeToLog("Failed to change output device to external headphones: " + result);
                                } else {
                                    juce::Logger::writeToLog("Successfully connected to external headphones: " + actualDeviceName);
                                    
                                    // 오디오 재시작
                                    deviceManager.restartLastAudioDevice();
                                }
                                return;
                            }
                        }
                    }
                    juce::Logger::writeToLog("External headphones device not found in available devices");
                } catch (...) {
                    juce::Logger::writeToLog("Error while searching for external headphones device");
                }
            }
        } else {
            // 특정 출력 장치로 설정
            auto currentSetup = deviceManager.getAudioDeviceSetup();
            currentSetup.outputDeviceName = deviceName;
            
            // 오디오 재시작을 위해 일시 중지
            deviceManager.closeAudioDevice();
            
            auto result = deviceManager.setAudioDeviceSetup(currentSetup, true);
            if (result.isNotEmpty()) {
                juce::Logger::writeToLog("Failed to change output device to: " + deviceName + " (" + result + ")");
            } else {
                juce::Logger::writeToLog("Successfully changed output device to: " + deviceName);
                
                // 오디오 재시작
                deviceManager.restartLastAudioDevice();
            }
        }
    }
    
    void performAutoSetup() {
        juce::Logger::writeToLog("=== Starting Auto Setup ===");
        
        // 1. JUCE 초기화 확인
        juce::Logger::writeToLog("JUCE initialized successfully");
        
        // 2. 오디오 디바이스 매니저 초기화 확인
        juce::Logger::writeToLog("Audio Device Manager will be initialized");
        
        // 3. macOS 확인
        #if JUCE_MAC
        juce::Logger::writeToLog("macOS detected - Core Audio will be used");
        #endif
        
        // 4. 플러그인 포맷 확인
        juce::Logger::writeToLog("Plugin formats will be initialized");
        
        // 5. 첫 실행 시 필요한 도구들 설치
        checkAndInstallRequiredTools();
        
        // 6. 현재 시스템 사운드 출력 소스 저장
        saveCurrentSystemOutputDevice();
        
        juce::Logger::writeToLog("=== Auto Setup Complete ===");
    }
    
    void setSystemOutputToBlackHole() {
        juce::Logger::writeToLog("Setting system output to BlackHole...");
        
        // 시스템 출력을 BlackHole로 변경하는 명령어들 시도 (AppleScript 제거)
        juce::StringArray commands = {
            "/opt/homebrew/bin/SwitchAudioSource -s \"BlackHole 2ch\"",
            "/usr/local/bin/SwitchAudioSource -s \"BlackHole 2ch\"",
            "/opt/homebrew/Cellar/switchaudio-osx/1.2.2/bin/SwitchAudioSource -s \"BlackHole 2ch\"",
            "SwitchAudioSource -s \"BlackHole 2ch\""
        };
        
        bool success = false;
        for (auto& command : commands) {
            int result = system(command.toRawUTF8());
            if (result == 0) {
                juce::Logger::writeToLog("Successfully set system output to BlackHole using: " + command);
                success = true;
                break;
            }
        }
        
        if (!success) {
            juce::Logger::writeToLog("Failed to set system output to BlackHole with all methods");
        }
    }
    
    void saveCurrentSystemOutputDevice() {
        juce::Logger::writeToLog("Saving current system output device...");
        
        // SwitchAudioSource를 사용하여 현재 시스템 출력 장치를 가져오기
        juce::StringArray commands = {
            "/opt/homebrew/bin/SwitchAudioSource -c",
            "/usr/local/bin/SwitchAudioSource -c",
            "/opt/homebrew/Cellar/switchaudio-osx/1.2.2/bin/SwitchAudioSource -c",
            "SwitchAudioSource -c"
        };
        
        for (auto& command : commands) {
            FILE* pipe = popen(command.toRawUTF8(), "r");
            if (pipe) {
                char buffer[256];
                juce::String result = "";
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }
                pclose(pipe);
                
                result = result.trim();
                if (result.isNotEmpty()) {
                    originalSystemOutputDevice = result;
                    juce::Logger::writeToLog("Saved current system output device: " + originalSystemOutputDevice);
                    return;
                }
            }
        }
        
        // 실패 시 기본값 사용
        originalSystemOutputDevice = "MacBook Pro 스피커";
        juce::Logger::writeToLog("Failed to get current device, using default: " + originalSystemOutputDevice);
    }
    

    
    void checkAndInstallRequiredTools() {
        // 첫 실행 파일 경로 설정
        firstRunFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("ClearHost")
                      .getChildFile("first_run_completed.txt");
        
        // 첫 실행이 완료되었는지 확인
        if (firstRunFile.existsAsFile()) {
            juce::Logger::writeToLog("First run already completed, skipping tool installation");
            return;
        }
        
        juce::Logger::writeToLog("=== First Run - Installing Required Tools ===");
        
        // 1. BlackHole 설치 확인 및 설치
        checkAndInstallBlackHole();
        
        // 2. switchaudio-osx 설치 확인 및 설치
        checkAndInstallSwitchAudioOSX();
        
        // 3. 시스템 설정 접근 권한 확인 및 안내
        checkSystemPreferencesAccess();
        
        // 4. 첫 실행 완료 표시
        firstRunFile.getParentDirectory().createDirectory();
        firstRunFile.create();
        juce::Logger::writeToLog("First run setup completed");
    }
    
    void checkAndInstallBlackHole() {
        juce::Logger::writeToLog("Checking BlackHole installation...");
        
        // BlackHole이 설치되어 있는지 확인
        juce::File blackHoleComponent("/Library/Audio/Plug-Ins/Components/BlackHole2ch.component");
        juce::File blackHoleVST("/Library/Audio/Plug-Ins/VST/BlackHole.vst");
        
        if (blackHoleComponent.exists() || blackHoleVST.exists()) {
            juce::Logger::writeToLog("BlackHole is already installed");
            return;
        }
        
        juce::Logger::writeToLog("BlackHole not found - installing...");
        
        // Homebrew를 통해 BlackHole 설치
        juce::String command = "brew install blackhole-2ch";
        int result = system(command.toRawUTF8());
        
        if (result == 0) {
            juce::Logger::writeToLog("BlackHole installed successfully");
        } else {
            juce::Logger::writeToLog("Failed to install BlackHole via Homebrew");
            
            // 대안: 수동 설치 안내
            juce::Logger::writeToLog("Please install BlackHole manually from: https://existential.audio/blackhole/");
        }
    }
    
    void checkAndInstallSwitchAudioOSX() {
        juce::Logger::writeToLog("Checking SwitchAudioSource installation...");
        
        // SwitchAudioSource가 설치되어 있는지 확인 (전체 경로 포함)
        juce::StringArray possiblePaths = {
            "/opt/homebrew/bin/SwitchAudioSource",
            "/usr/local/bin/SwitchAudioSource",
            "/opt/homebrew/Cellar/switchaudio-osx/1.2.2/bin/SwitchAudioSource",
            "SwitchAudioSource"
        };
        
        bool found = false;
        for (auto& path : possiblePaths) {
            juce::String command = "which " + path;
            FILE* pipe = popen(command.toRawUTF8(), "r");
            if (pipe) {
                char buffer[256];
                juce::String result = "";
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }
                pclose(pipe);
                
                if (result.trim().isNotEmpty()) {
                    juce::Logger::writeToLog("SwitchAudioSource found at: " + result.trim());
                    found = true;
                    break;
                }
            }
        }
        
        if (found) {
            juce::Logger::writeToLog("SwitchAudioSource is already installed");
            return;
        }
        
        juce::Logger::writeToLog("SwitchAudioSource not found - installing...");
        
        // Homebrew를 통해 switchaudio-osx 설치
        juce::String command = "brew install switchaudio-osx";
        int result = system(command.toRawUTF8());
        
        if (result == 0) {
            juce::Logger::writeToLog("SwitchAudioSource installed successfully");
        } else {
            juce::Logger::writeToLog("Failed to install SwitchAudioSource via Homebrew");
        }
    }
    
    void checkSystemPreferencesAccess() {
        juce::Logger::writeToLog("Checking System Preferences access...");
        
        // 시스템 설정 접근 권한 확인
        juce::String command = "osascript -e 'tell application \"System Events\" to get name of current location of (get volume settings)'";
        FILE* pipe = popen(command.toRawUTF8(), "r");
        if (pipe) {
            char buffer[256];
            juce::String result = "";
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            
            if (result.trim().isNotEmpty()) {
                juce::Logger::writeToLog("System Preferences access is working");
            } else {
                juce::Logger::writeToLog("System Preferences access may be restricted");
                juce::Logger::writeToLog("Please grant accessibility permissions in System Preferences > Security & Privacy > Privacy > Accessibility");
            }
        }
    }
    
    void mapMidiCCToClearParameter(const juce::MidiMessage& message) {
        int cc = message.getControllerNumber();
        float value = message.getControllerValue() / 127.0f;
        int paramIdx = -1;
        if (cc == 21) paramIdx = 0;
        else if (cc == 22) paramIdx = 1;
        else if (cc == 23) paramIdx = 2;
        if (clearPlugin && paramIdx >= 0) {
            auto params = clearPlugin->getParameters();
            if (paramIdx < params.size() && params[paramIdx])
                params[paramIdx]->setValueNotifyingHost(value);
        }
    }
};

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(juce::String name) : juce::DocumentWindow(name,
        juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
        juce::DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setContentOwned(new ClearHostApp(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }
    void closeButtonPressed() override {
        // 앱 종료 시 시스템 출력 소스 복구 (안전하게)
        try {
            if (auto* app = dynamic_cast<ClearHostApp*>(getContentComponent())) {
                app->restoreSystemOutputDevice();
            }
        } catch (...) {
            juce::Logger::writeToLog("Error during system output restoration on exit");
        }
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class ClearHostApplication : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "ClearHost"; }
    const juce::String getApplicationVersion() override { return "1.0"; }
    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    void shutdown() override {
        mainWindow = nullptr;
    }
private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(ClearHostApplication)
