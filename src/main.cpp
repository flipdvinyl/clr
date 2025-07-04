#include <JuceHeader.h>

class ClearHostApp : public juce::AudioAppComponent, public juce::AudioProcessorPlayer, public juce::AudioProcessorListener, public juce::Slider::Listener, public juce::ComboBox::Listener, public juce::Button::Listener, public juce::Timer {
public:
    ClearHostApp() {
        animationDuration = 1.0;
        animationTimerInterval = 16;
        originalSystemOutputDevice = "";
        performAutoSetup();
        pluginManager.addDefaultFormats();
        loadClearVST3();
        setAudioChannels(2, 2);
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        if (!midiInputs.isEmpty()) {
            midiInput = juce::MidiInput::openDevice(midiInputs[0].identifier, this);
            if (midiInput) midiInput->start();
        }
        setProcessor(clearPlugin.get());
        // 플러그인 에디터만 addAndMakeVisible
        if (clearPlugin) {
            pluginEditor.reset(clearPlugin->createEditor());
            if (pluginEditor) {
                addAndMakeVisible(pluginEditor.get());
            }
            juce::Logger::writeToLog("Registering parameter listener...");
            clearPlugin->addListener(this);
            isAnimating = false;
            startTimer(100); // 파라미터 동기화용
        }
        setSize(1200, 700);
        
        // 노브 컨트롤들 생성
        for (int i = 0; i < 3; ++i) {
            auto knob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            knob->setRange(0.0, 1.0, 0.01);
            knob->setValue(0.5);
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            knob->addListener(this);
            knobs.add(knob.release());
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
        
        // 드롭다운용 장치 리스트 초기화
        updateInputDeviceList();
        updateOutputDeviceList();
        
        // 라벨 설정
        inputDeviceLabel.setText("Audio Input:", juce::dontSendNotification);
        inputDeviceLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(inputDeviceLabel);
        
        outputDeviceLabel.setText("Audio Output:", juce::dontSendNotification);
        outputDeviceLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(outputDeviceLabel);
        
        // 추가된 멤버 변수 초기화
        knobRects.resize(3);
        knobValues.resize(3);
        buttonRects.resize(7); // 7개 버튼으로 확장
        buttonLabels.resize(7);
        stereoMonoRect = juce::Rectangle<int>();
        inputDeviceRect = juce::Rectangle<int>();
        outputDeviceRect = juce::Rectangle<int>();
        draggingKnob = -1;
        lastDragY = 0;
        
        // 드롭다운 관련 변수들
        inputDropdownOpen = false;
        outputDropdownOpen = false;
        inputDeviceList.clear();
        outputDeviceList.clear();
        inputDropdownRect = juce::Rectangle<int>();
        outputDropdownRect = juce::Rectangle<int>();
        inputDeviceRects.clear();
        outputDeviceRects.clear();
    }
    ~ClearHostApp() override {
        if (clearPlugin) {
            juce::Logger::writeToLog("Removing parameter listener...");
            clearPlugin->removeListener(this);
            clearPlugin = nullptr;
        }
        knobRects.clear();
        knobValues.clear();
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
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        int w = bounds.getWidth();
        int h = bounds.getHeight();
        int col1 = w * 0.25;
        int col2 = w * 0.35;
        int col3 = w - col1 - col2;
        // 플러그인 관련 멤버 접근 전 nullptr/size 체크
        if (knobRects.size() < 3 || knobValues.size() < 3) {
            juce::Logger::writeToLog("paint: knobRects/knobValues size error");
            return;
        }
        // 1단: 오렌지 배경
        juce::Rectangle<int> leftArea(0, 0, col1, h);
        g.setColour(juce::Colours::orange);
        g.fillRect(leftArea);
        // 좌상단 160x280 박스
        juce::Rectangle<int> box(20, 20, 160, 280);
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(box);
        // 박스 중앙에 파란 원
        int cx = box.getCentreX();
        int cy = box.getCentreY();
        int radius = 60;
        g.setColour(juce::Colours::deepskyblue);
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
        // 2단: 가운데 영역 (footer 내용)
        juce::Rectangle<int> centerArea(col1, 0, col2, h);
        g.setColour(juce::Colours::darkgrey.darker());
        g.fillRect(centerArea);
        // 노브 3개 (원)
        int knobY = centerArea.getY() + 60;
        for (int i = 0; i < 3; ++i) {
            int knobX = centerArea.getX() + 60 + i * 160;
            knobRects[i] = juce::Rectangle<int>(knobX, knobY, 80, 80);
            g.setColour(juce::Colours::white);
            g.drawEllipse(knobRects[i].toFloat(), 3.0f);
            // 노브 인디케이터 (10시~2시, 값=0~1, min~max 시계 방향)
            float v = knobValues[i];
            float minAngle = juce::MathConstants<float>::pi * 5.0f/6.0f; // 10시
            float maxAngle = juce::MathConstants<float>::pi * 13.0f/6.0f; // 2시 (시계 방향)
            float angle = minAngle + v * (maxAngle - minAngle);
            float cx = knobRects[i].getCentreX();
            float cy = knobRects[i].getCentreY();
            float r = 40.0f;
            float ex = cx + std::cos(angle) * r;
            float ey = cy + std::sin(angle) * r;
            g.setColour(juce::Colours::deepskyblue);
            g.drawLine(cx, cy, ex, ey, 4.0f);
            g.setColour(juce::Colours::white);
            g.drawText("Knob " + juce::String(i+1), knobX, knobY + 90, 80, 20, juce::Justification::centred);
        }
        // 프리셋 버튼 7개 (2줄)
        int btnW = 100, btnH = 30, btnSpacing = 20;
        int btnY1 = centerArea.getY() + 180;
        int btnY2 = btnY1 + btnH + 10;
        juce::String btnNames[7] = {"Bypass","S* up**","Too Loud","Silence","Clear Voice","Dry Voice","Vocal Reference"};
        buttonRects.resize(7); // 7개 버튼으로 확장
        for (int i = 0; i < 7; ++i) {
            int row = i < 4 ? 0 : 1;
            int col = row == 0 ? i : i-4;
            int btnX = centerArea.getX() + 30 + col * (btnW + btnSpacing);
            int btnY = row == 0 ? btnY1 : btnY2;
            buttonRects[i] = juce::Rectangle<int>(btnX, btnY, btnW, btnH);
            g.setColour(juce::Colours::grey);
            g.fillRect(btnX, btnY, btnW, btnH);
            g.setColour(juce::Colours::white);
            g.drawText(btnNames[i], btnX, btnY, btnW, btnH, juce::Justification::centred);
        }
        // 스테레오/모노 버튼
        int stereoY = btnY2 + btnH + 20;
        stereoMonoRect = juce::Rectangle<int>(centerArea.getX() + 60, stereoY, 120, 30);
        g.setColour(juce::Colours::darkorange);
        g.fillRect(stereoMonoRect);
        g.setColour(juce::Colours::white);
        g.drawText("Stereo/Mono", stereoMonoRect, juce::Justification::centred);
        // 장치 선택 박스
        int deviceY = stereoY + 50;
        inputDeviceRect = juce::Rectangle<int>(centerArea.getX() + 30, deviceY, 180, 30);
        outputDeviceRect = juce::Rectangle<int>(centerArea.getX() + 230, deviceY, 180, 30);
        
        // 입력 장치 버튼
        g.setColour(juce::Colours::lightblue);
        g.fillRect(inputDeviceRect);
        g.setColour(juce::Colours::black);
        juce::String currentInput = inputDeviceList.empty() ? "Default Input" : inputDeviceList[0];
        g.drawText(currentInput, inputDeviceRect, juce::Justification::centred);
        
        // 출력 장치 버튼
        g.setColour(juce::Colours::lightgreen);
        g.fillRect(outputDeviceRect);
        g.setColour(juce::Colours::black);
        juce::String currentOutput = outputDeviceList.empty() ? "Default Output" : outputDeviceList[0];
        g.drawText(currentOutput, outputDeviceRect, juce::Justification::centred);
        
        // 입력 드롭다운이 열려있으면 장치 리스트 표시
        if (inputDropdownOpen) {
            inputDropdownRect = juce::Rectangle<int>(inputDeviceRect.getX(), inputDeviceRect.getBottom(), inputDeviceRect.getWidth(), 150);
            g.setColour(juce::Colours::white);
            g.fillRect(inputDropdownRect);
            g.setColour(juce::Colours::black);
            g.drawRect(inputDropdownRect);
            
            inputDeviceRects.clear();
            for (int i = 0; i < inputDeviceList.size(); ++i) {
                juce::Rectangle<int> itemRect(inputDropdownRect.getX(), inputDropdownRect.getY() + i * 25, inputDropdownRect.getWidth(), 25);
                inputDeviceRects.push_back(itemRect);
                g.setColour(juce::Colours::lightgrey);
                g.fillRect(itemRect);
                g.setColour(juce::Colours::black);
                g.drawText(inputDeviceList[i], itemRect, juce::Justification::centredLeft);
            }
        }
        
        // 출력 드롭다운이 열려있으면 장치 리스트 표시
        if (outputDropdownOpen) {
            outputDropdownRect = juce::Rectangle<int>(outputDeviceRect.getX(), outputDeviceRect.getBottom(), outputDeviceRect.getWidth(), 150);
            g.setColour(juce::Colours::white);
            g.fillRect(outputDropdownRect);
            g.setColour(juce::Colours::black);
            g.drawRect(outputDropdownRect);
            
            outputDeviceRects.clear();
            for (int i = 0; i < outputDeviceList.size(); ++i) {
                juce::Rectangle<int> itemRect(outputDropdownRect.getX(), outputDropdownRect.getY() + i * 25, outputDropdownRect.getWidth(), 25);
                outputDeviceRects.push_back(itemRect);
                g.setColour(juce::Colours::lightgrey);
                g.fillRect(itemRect);
                g.setColour(juce::Colours::black);
                g.drawText(outputDeviceList[i], itemRect, juce::Justification::centredLeft);
            }
        }
        
        // 3단: 오른쪽 플러그인 에디터 영역
        juce::Rectangle<int> rightArea(col1 + col2, 0, col3, h);
        g.setColour(juce::Colours::darkslategrey);
        g.fillRect(rightArea);
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Clear Plugin Editor", rightArea.reduced(20), juce::Justification::centred);
        if (pluginEditor) {
            pluginEditor->setBounds(col1 + col2, 0, col3, h);
        }
    }
    void resized() override {
        auto bounds = getLocalBounds();
        int w = bounds.getWidth();
        int col1 = w * 0.25;
        int col2 = w * 0.35;
        int col3 = w - col1 - col2;
        if (pluginEditor) {
            pluginEditor->setBounds(col1 + col2, 0, col3, bounds.getHeight());
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
        juce::Logger::writeToLog("audioProcessorParameterChanged: enter idx=" + juce::String(parameterIndex));
        if (!clearPlugin) return;
        auto params = clearPlugin->getParameters();
        if (parameterIndex == 1 && params.size() > 1 && params[1]) knobValues[0] = newValue;
        else if (parameterIndex == 14 && params.size() > 14 && params[14]) knobValues[1] = newValue;
        else if (parameterIndex == 12 && params.size() > 12 && params[12]) knobValues[2] = newValue;
        repaint();
        juce::Logger::writeToLog("audioProcessorParameterChanged: exit idx=" + juce::String(parameterIndex));
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
        for (int i = 0; i < 3; ++i) {
            animationStartValues[i] = knobValues[i];
            animationTargetValues[i] = targetValues[i];
        }
        animationStartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        isAnimating = true;
        startTimer(animationTimerInterval);
    }
    
    void setKnobValue(int knobIndex, double value) {
        if (knobIndex >= 0 && knobIndex < knobValues.size()) {
            knobValues[knobIndex] = value;
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                int targetParamIndex = -1;
                switch (knobIndex) {
                    case 0: targetParamIndex = 1; break;
                    case 1: targetParamIndex = 14; break;
                    case 2: targetParamIndex = 12; break;
                }
                if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                    params[targetParamIndex]->setValueNotifyingHost((float)value);
                }
            }
        }
    }
    
    void timerCallback() override {
        if (isAnimating) {
            double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
            double elapsedTime = currentTime - animationStartTime;
            double progress = juce::jlimit(0.0, 1.0, elapsedTime / animationDuration);
            double easedProgress = 1.0 - std::pow(1.0 - progress, 3.0);
            for (int i = 0; i < 3; ++i) {
                double currentValue = animationStartValues[i] + (animationTargetValues[i] - animationStartValues[i]) * easedProgress;
                setKnobValue(i, currentValue);
            }
            repaint();
            if (progress >= 1.0) {
                isAnimating = false;
                stopTimer();
                juce::Logger::writeToLog("Animation completed");
            }
        } else {
            stopTimer();
            juce::Logger::writeToLog("timerCallback: updateKnobsFromPlugin() 시도");
            if (!clearPlugin) return;
            auto params = clearPlugin->getParameters();
            if (params.size() > 1 && params[1] && params[1]->getName(100).isNotEmpty()) {
                juce::Logger::writeToLog("timerCallback: 파라미터 접근 OK");
                updateKnobsFromPlugin();
            } else {
                juce::Logger::writeToLog("timerCallback: 파라미터 접근 불가, 재시도");
                startTimer(100);
            }
        }
    }
    
    void updateKnobsFromPlugin() {
        if (!clearPlugin) return;
        auto params = clearPlugin->getParameters();
        juce::Logger::writeToLog("updateKnobsFromPlugin: params.size()=" + juce::String(params.size()));
        if (params.size() > 1) {
            juce::Logger::writeToLog("param[1] ptr=" + juce::String((uint64_t)params[1]));
            if (params[1]) {
                juce::Logger::writeToLog("param[1] getValue() before");
                knobValues[0] = params[1]->getValue();
                juce::Logger::writeToLog("param[1] getValue() after: " + juce::String(knobValues[0]));
            }
        }
        if (params.size() > 14) {
            juce::Logger::writeToLog("param[14] ptr=" + juce::String((uint64_t)params[14]));
            if (params[14]) {
                juce::Logger::writeToLog("param[14] getValue() before");
                knobValues[1] = params[14]->getValue();
                juce::Logger::writeToLog("param[14] getValue() after: " + juce::String(knobValues[1]));
            }
        }
        if (params.size() > 12) {
            juce::Logger::writeToLog("param[12] ptr=" + juce::String((uint64_t)params[12]));
            if (params[12]) {
                juce::Logger::writeToLog("param[12] getValue() before");
                knobValues[2] = params[12]->getValue();
                juce::Logger::writeToLog("param[12] getValue() after: " + juce::String(knobValues[2]));
            }
        }
        repaint();
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

    void mouseDown(const juce::MouseEvent& event) override {
        auto pos = event.getPosition();
        // 노브 클릭 hit test
        for (int i = 0; i < knobRects.size(); ++i) {
            if (knobRects[i].contains(pos)) {
                draggingKnob = i;
                lastDragY = pos.y;
                return;
            }
        }
        // 프리셋 버튼 7개
        for (int i = 0; i < 7; ++i) { // buttonRects.size() 대신 7로 고정
            if (buttonRects[i].contains(pos)) {
                // 기존 preset 기능 매핑
                switch (i) {
                    case 0: startAnimation({0.5, 0.5, 0.5}); break; // Bypass
                    case 1: startAnimation({0.5, 0.0, 0.0}); break; // S* up**
                    case 2: startAnimation({0.5, 0.2, 0.2}); break; // Too Loud
                    case 3: startAnimation({0.0, 0.0, 0.0}); break; // Silence
                    case 4: startAnimation({0.0, 0.5, 0.5}); break; // Clear Voice
                    case 5: startAnimation({0.0, 0.5, 0.0}); break; // Dry Voice
                    case 6: startAnimation({0.5, 0.1, 0.1}); break; // Vocal Reference
                }
                repaint();
                return;
            }
        }
        // 스테레오/모노 버튼
        if (stereoMonoRect.contains(pos)) {
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                if (params.size() > 13 && params[13]) {
                    float currentValue = params[13]->getValue();
                    float newValue = (currentValue > 0.5f) ? 0.0f : 1.0f;
                    params[13]->setValueNotifyingHost(newValue);
                }
            }
            repaint();
            return;
        }
        
        // 입력 장치 버튼 클릭
        if (inputDeviceRect.contains(pos)) {
            inputDropdownOpen = !inputDropdownOpen;
            outputDropdownOpen = false; // 다른 드롭다운 닫기
            if (inputDropdownOpen && inputDeviceList.empty()) {
                updateInputDeviceList();
            }
            repaint();
            return;
        }
        
        // 출력 장치 버튼 클릭
        if (outputDeviceRect.contains(pos)) {
            outputDropdownOpen = !outputDropdownOpen;
            inputDropdownOpen = false; // 다른 드롭다운 닫기
            if (outputDropdownOpen && outputDeviceList.empty()) {
                updateOutputDeviceList();
            }
            repaint();
            return;
        }
        
        // 입력 드롭다운 아이템 클릭
        if (inputDropdownOpen) {
            for (int i = 0; i < inputDeviceRects.size(); ++i) {
                if (inputDeviceRects[i].contains(pos)) {
                    juce::String selectedDevice = inputDeviceList[i];
                    changeAudioInputDevice(selectedDevice);
                    inputDropdownOpen = false;
                    repaint();
                    return;
                }
            }
        }
        
        // 출력 드롭다운 아이템 클릭
        if (outputDropdownOpen) {
            for (int i = 0; i < outputDeviceRects.size(); ++i) {
                if (outputDeviceRects[i].contains(pos)) {
                    juce::String selectedDevice = outputDeviceList[i];
                    changeAudioOutputDevice(selectedDevice);
                    outputDropdownOpen = false;
                    repaint();
                    return;
                }
            }
        }
        
        // 드롭다운 외부 클릭 시 드롭다운 닫기
        if (inputDropdownOpen || outputDropdownOpen) {
            inputDropdownOpen = false;
            outputDropdownOpen = false;
            repaint();
            return;
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override {
        if (draggingKnob >= 0) {
            int dy = lastDragY - event.getPosition().y;
            float delta = dy * 0.005f; // 감도
            knobValues[draggingKnob] = juce::jlimit(0.0f, 1.0f, knobValues[draggingKnob] + delta);
            lastDragY = event.getPosition().y;
            
            // 애니메이션 중에 노브를 조절하면 해당 노브의 애니메이션을 강제 중지
            if (isAnimating) {
                animationTargetValues[draggingKnob] = knobValues[draggingKnob];
                juce::Logger::writeToLog("Knob " + juce::String(draggingKnob) + " animation stopped by user interaction");
            }
            
            // 파라미터 반영
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                int targetParamIndex = -1;
                switch (draggingKnob) {
                    case 0: targetParamIndex = 1; break;
                    case 1: targetParamIndex = 14; break;
                    case 2: targetParamIndex = 12; break;
                }
                if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                    params[targetParamIndex]->setValueNotifyingHost(knobValues[draggingKnob]);
                }
            }
            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent&) override {
        draggingKnob = -1;
    }

    void updateOutputDeviceList() {
        outputDeviceList.clear();
        outputDeviceList.push_back("Default Output");
        
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        if (deviceType) {
            try {
                auto outputNames = deviceType->getDeviceNames(false); // false = output devices
                for (int i = 0; i < outputNames.size(); ++i) {
                    outputDeviceList.push_back(outputNames[i]);
                }
            } catch (...) {
                juce::Logger::writeToLog("Error getting output device names");
            }
        }
    }
    
    void updateInputDeviceList() {
        inputDeviceList.clear();
        inputDeviceList.push_back("Default Input");
        
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        if (deviceType) {
            try {
                auto inputNames = deviceType->getDeviceNames(true); // true = input devices
                for (int i = 0; i < inputNames.size(); ++i) {
                    juce::String deviceName = inputNames[i];
                    if (deviceName.contains("BlackHole") || deviceName.contains("blackhole")) {
                        inputDeviceList.push_back("OS Sound (BlackHole)");
                    } else {
                        inputDeviceList.push_back(deviceName);
                    }
                }
            } catch (...) {
                juce::Logger::writeToLog("Error getting input device names");
            }
        }
    }

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
    
    std::vector<juce::Rectangle<int>> knobRects;
    std::vector<float> knobValues;
    std::vector<juce::Rectangle<int>> buttonRects;
    std::vector<juce::String> buttonLabels;
    juce::Rectangle<int> stereoMonoRect;
    juce::Rectangle<int> inputDeviceRect;
    juce::Rectangle<int> outputDeviceRect;
    int draggingKnob = -1;
    int lastDragY = 0;
    
    // 드롭다운 관련 변수들
    bool inputDropdownOpen = false;
    bool outputDropdownOpen = false;
    std::vector<juce::String> inputDeviceList;
    std::vector<juce::String> outputDeviceList;
    juce::Rectangle<int> inputDropdownRect;
    juce::Rectangle<int> outputDropdownRect;
    std::vector<juce::Rectangle<int>> inputDeviceRects;
    std::vector<juce::Rectangle<int>> outputDeviceRects;
    
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
