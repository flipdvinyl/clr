#include <JuceHeader.h>
#include <map>

// 기본 투명도 설정 (80%)
static constexpr float DEFAULT_ALPHA = 0.8f;

class EuclidLookAndFeel : public juce::LookAndFeel_V4 {
public:
    EuclidLookAndFeel() { loadAllFonts(); }

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override {
        auto key = getKey(font);
        auto it = typefaceMap.find(key);
        if (it != typefaceMap.end())
            return it->second;
        // fallback: Regular
        auto reg = typefaceMap.find({400, false});
        if (reg != typefaceMap.end())
            return reg->second;
        return juce::LookAndFeel_V4::getTypefaceForFont(font);
    }

    void loadAllFonts() {
        juce::File appDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
        juce::File fontsDir = appDir.getChildFile("Resources").getChildFile("Fonts");
        struct FontInfo { const char* name; int weight; bool italic; };
        FontInfo fonts[] = {
            {"EuclidCircularB-Light", 300, false},
            {"EuclidCircularB-LightItalic", 300, true},
            {"EuclidCircularB-Regular", 400, false},
            {"EuclidCircularB-RegularItalic", 400, true},
            {"EuclidCircularB-Medium", 500, false},
            {"EuclidCircularB-MediumItalic", 500, true},
            {"EuclidCircularB-Semibold", 600, false},
            {"EuclidCircularB-SemiboldItalic", 600, true},
            {"EuclidCircularB-Bold", 700, false},
            {"EuclidCircularB-BoldItalic", 700, true},
        };
        for (const auto& info : fonts) {
            juce::File fontFile;
            #if JUCE_MAC
                fontFile = fontsDir.getChildFile(juce::String(info.name) + ".otf");
                if (!fontFile.existsAsFile())
                    fontFile = fontsDir.getChildFile(juce::String(info.name) + ".ttf");
            #else
                fontFile = fontsDir.getChildFile(juce::String(info.name) + ".ttf");
                if (!fontFile.existsAsFile())
                    fontFile = fontsDir.getChildFile(juce::String(info.name) + ".otf");
            #endif
            if (fontFile.existsAsFile()) {
                juce::MemoryBlock fontData;
                if (fontFile.loadFileAsData(fontData)) {
                    auto tf = juce::Typeface::createSystemTypefaceFor(fontData.getData(), fontData.getSize());
                    if (tf != nullptr) {
                        typefaceMap[{info.weight, info.italic}] = tf;
                        juce::Logger::writeToLog("Loaded font: " + fontFile.getFileName());
                    }
                }
            }
        }
    }

private:
    std::map<std::pair<int, bool>, juce::Typeface::Ptr> typefaceMap;
    std::pair<int, bool> getKey(const juce::Font& font) const {
        int weight = 400;
        bool italic = font.isItalic();
        if (font.isBold()) weight = 700;
        else if (font.getTypefaceStyle().containsIgnoreCase("Light")) weight = 300;
        else if (font.getTypefaceStyle().containsIgnoreCase("Medium")) weight = 500;
        else if (font.getTypefaceStyle().containsIgnoreCase("Semibold")) weight = 600;
        return {weight, italic};
    }
};

class Face {
public:
    Face() : color(juce::Colour(0xFFFF9625)) {} // FF9625 색상
    
    void draw(juce::Graphics& g) const {
        g.setColour(color);
        g.fillRect(0, 0, 160, 270); // 160x270 크기, 좌상단 (0,0) 시작
        
        // 세퍼레이터 그리기
        // 크기: 145x4, 중앙정렬 센터 좌표 (80,126), 모서리 라운드 2px, 흰색 투명도 20%
        int separatorWidth = 145;
        int separatorHeight = 4;
        int separatorX = 80 - separatorWidth / 2; // 중앙정렬
        int separatorY = 126 - separatorHeight / 2; // 중앙정렬 (위로 2px 이동)
        
        g.setColour(juce::Colours::white.withAlpha(0.2f)); // 흰색, 투명도 20%
        g.fillRoundedRectangle(separatorX, separatorY, separatorWidth, separatorHeight, 2.0f);
    }
    
    juce::Colour getColor() const {
        return color;
    }
    
    void setColor(juce::Colour newColor) {
        color = newColor;
    }
    
private:
    juce::Colour color;
};

class KnobWithLabel {
public:
    KnobWithLabel(juce::Rectangle<int> area, float value, const juce::String& labelText, juce::Colour knobColour, juce::Colour labelColour, juce::Colour indicatorColour, bool showValue = false)
        : area(area), value(value), labelText(labelText), knobColour(knobColour), labelColour(labelColour), indicatorColour(indicatorColour), showValue(showValue) {}

    void draw(juce::Graphics& g) const {
        // 노브 원
        float cx = area.getCentreX();
        float cy = area.getCentreY();
        float radius = 20.0f;
        g.setColour(knobColour);
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
        // 인디케이터 - Face 색상 사용
        // 270도 범위로 확장: minAngle을 15도 더 반시계, maxAngle을 15도 더 시계방향
        float minAngle = juce::MathConstants<float>::pi * 5.0f/6.0f - juce::MathConstants<float>::pi * 15.0f/180.0f; // 15도 반시계 추가
        float maxAngle = juce::MathConstants<float>::pi * 13.0f/6.0f + juce::MathConstants<float>::pi * 15.0f/180.0f; // 15도 시계방향 추가
        // 0~2 범위를 0~1로 변환하여 각도 계산 (270도 회전 범위)
        float normalizedValue = value / 2.0f;
        float angle = minAngle + normalizedValue * (maxAngle - minAngle);
        float startX = cx + std::cos(angle) * (radius + 1.0f);
        float startY = cy + std::sin(angle) * (radius + 1.0f);
        float endX = cx + std::cos(angle) * (radius - 15.0f);
        float endY = cy + std::sin(angle) * (radius - 15.0f);
        g.setColour(indicatorColour); // Face 색상으로 변경
        g.drawLine(startX, startY, endX, endY, 4.0f);
        
        // 레이블 또는 값 표시
        int labelW = area.getWidth();
        int labelH = 20;
        int labelX = area.getX();
        int labelY = static_cast<int>(cy + radius - 1); // 노브 원 아래 -1px (2px 아래)
        g.setColour(labelColour);
        g.setFont(juce::Font("Euclid Circular B", 16.0f, juce::Font::plain));
        
        juce::String displayText;
        if (showValue) {
            // 소숫점 두자리까지 표시
            displayText = juce::String(value, 2);
        } else {
            displayText = labelText;
        }
        
        g.drawText(displayText, labelX, labelY, labelW, labelH, juce::Justification::centred);
    }
    
    void setShowValue(bool shouldShow) {
        showValue = shouldShow;
    }
    
    juce::Rectangle<int> area;
    float value;
    juce::String labelText;
    juce::Colour knobColour;
    juce::Colour labelColour;
    juce::Colour indicatorColour;
    bool showValue;
};

class KnobCluster {
public:
    KnobCluster(juce::Point<int> center, std::vector<float>& values, std::vector<juce::Rectangle<int>>& rects, juce::Colour faceColor, std::vector<bool>& showValues, float alpha = 1.0f)
        : center(center), values(values), knobRects(rects), faceColor(faceColor), showValues(showValues), alpha(alpha) {}
    void draw(juce::Graphics& g) const {
        struct KnobPos { int dx, dy; const char* label; };
        KnobPos knobPos[3] = {
            {-46, -37, "amb"},
            {  0,  -2, "vox"}, // 노브 2번 위로 2px 이동
            { 46, -37, "v. rev"}
        };
        for (int i = 0; i < 3; ++i) {
            int cx = center.x + knobPos[i].dx;
            int cy = center.y + knobPos[i].dy;
            knobRects[i] = juce::Rectangle<int>(cx - 20, cy - 20, 40, 40); // 40x40로 축소
            KnobWithLabel knob(
                knobRects[i],
                values[i],
                knobPos[i].label,
                juce::Colours::black.withAlpha(alpha),
                juce::Colours::black.withAlpha(alpha),
                faceColor, // 인디케이터는 알파값 적용 안함
                showValues[i]
            );
            knob.draw(g);
        }
    }
    // 값을 외부에서 바꿀 수 있도록 참조로 보관
    std::vector<float>& values;
    std::vector<juce::Rectangle<int>>& knobRects;
    juce::Point<int> center;
    juce::Colour faceColor;
    std::vector<bool>& showValues;
    float alpha;
};

class TextButtonLike {
public:
    TextButtonLike(const juce::String& text, juce::Point<int> position, float alpha = DEFAULT_ALPHA)
        : text(text), position(position), alpha(alpha) {}
    void draw(juce::Graphics& g) const {
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.setFont(juce::Font("Euclid Circular B", 15.0f, juce::Font::plain));
        
        // 텍스트 크기 계산
        int textW = g.getCurrentFont().getStringWidth(text);
        int textH = (int)g.getCurrentFont().getHeight();
        
        // 텍스트 그리기
        g.drawText(text, position.x, position.y, textW, textH, juce::Justification::centred);
        
        // Underline - 텍스트 기준으로 위치 계산
        int underlineY = position.y + textH - 1; // 텍스트 아래 0px (8px 아래로 이동)
        int underlineW = textW;
        int underlineX = position.x;
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.drawLine(underlineX, underlineY, underlineX + underlineW, underlineY, 1.0f);
    }
    bool hitTest(juce::Point<int> pos) const { 
        // 텍스트 크기에 맞춰서 상하좌우 3px 확장
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int textW = font.getStringWidth(text);
        int textH = (int)font.getHeight();
        return juce::Rectangle<int>(position.x - 3, position.y - 3, textW + 6, textH + 6).contains(pos); 
    }
    juce::String text;
    juce::Point<int> position;
    float alpha;
};

enum class LEDState {
    OFF,           // 꺼짐 (어두운 녹색)
    PLUGIN_ON,     // 플러그인 로딩 성공 (밝은 녹색)
    PLUGIN_OFF,    // 플러그인 로딩 실패 (빨간색)
    BYPASS_ON      // 바이패스 활성화 (어두운 녹색)
};

class LED {
public:
    LED(juce::Point<int> center, LEDState state = LEDState::OFF)
        : center(center), state(state) {}
    
    void draw(juce::Graphics& g) const {
        // 6px 반지름의 원
        float radius = 3.0f;
        
        // 상태에 따른 색상
        juce::Colour ledColour;
        switch (state) {
            case LEDState::OFF:
                ledColour = juce::Colour(0xFF4E564E); // 어두운 녹색(4e564e)
                break;
            case LEDState::PLUGIN_ON:
                ledColour = juce::Colour(0xFF36B24A); // 밝은 녹색
                break;
            case LEDState::PLUGIN_OFF:
                ledColour = juce::Colour(0xFFB23636); // 빨간색
                break;
            case LEDState::BYPASS_ON:
                ledColour = juce::Colour(0xFF4E564E); // 어두운 녹색(4e564e, 바이패스 시)
                break;
        }
        
        g.setColour(ledColour);
        g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);
    }
    
    void setState(LEDState newState) {
        state = newState;
    }
    
    LEDState getState() const {
        return state;
    }
    
    // 기존 bool 인터페이스 호환성을 위한 메서드들
    void setState(bool on) {
        state = on ? LEDState::PLUGIN_ON : LEDState::OFF;
    }
    
    bool isOn() const {
        return state == LEDState::PLUGIN_ON;
    }
    
    juce::Point<int> center;
    LEDState state;
};

class Panel {
public:
    Panel(juce::Point<int> center, std::vector<float>& knobValues, std::vector<juce::Rectangle<int>>& knobRects, std::unique_ptr<LED>& statusLED, juce::Colour faceColor, std::vector<bool>& showValues)
        : center(center), knobValues(knobValues), knobRects(knobRects), statusLED(statusLED), faceColor(faceColor), showValues(showValues), bypassActive(false) {}
    
    void setBypassState(bool bypassOn) {
        bypassActive = bypassOn;
    }
    
    void draw(juce::Graphics& g) const {
        // bypass 상태에 따른 알파값 설정 (LED와 노브 인디케이터 제외)
        float alpha = bypassActive ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
        
        // 1. 노브 3개 그리기 (알파값 적용, 단 인디케이터는 제외)
        KnobCluster cluster(center, knobValues, knobRects, faceColor, showValues, alpha);
        cluster.draw(g);
        
        // 2. LED 그리기 (2번 노브 중앙 기준, 위로 69px) - LED는 알파값 적용 안함
        if (knobRects.size() >= 2 && statusLED) {
            juce::Point<int> ledCenter = knobRects[1].getCentre(); // 2번 노브(voice) 중앙
            ledCenter.y -= 69; // 위로 69px (1px 아래로 이동)
            statusLED->center = ledCenter;
            statusLED->draw(g);
        }
        
        // 3. Stereo/Mono 토글 버튼 그리기 (알파값 적용)
        // stereoText는 외부에서 updateStereoText()로 업데이트됨
        
        int stereoX = knobRects[1].getCentreX();
        int stereoY = center.y - 51; // 노브 클러스터 중앙에서 위로 51px (1px 더 위로 이동)
        
        // 텍스트 크기에 맞춰서 위치 계산
        g.setFont(juce::Font("Euclid Circular B", 15.0f, juce::Font::plain));
        int textW = g.getCurrentFont().getStringWidth(stereoText);
        int textH = (int)g.getCurrentFont().getHeight();
        int stereoStartX = stereoX - textW/2;
        
        stereoMonoRect = juce::Rectangle<int>(stereoStartX, stereoY, textW, textH);
        TextButtonLike stereoBtn(stereoText, juce::Point<int>(stereoStartX, stereoY), alpha);
        stereoBtn.draw(g);
    }
    
    // Stereo/Mono 버튼의 hit test를 위한 함수
    bool hitTestStereoButton(juce::Point<int> pos) const {
        juce::String stereoText = "stereo";
        int stereoStartX = stereoMonoRect.getX();
        int stereoStartY = stereoMonoRect.getY();
        TextButtonLike tempStereoBtn(stereoText, juce::Point<int>(stereoStartX, stereoStartY));
        return tempStereoBtn.hitTest(pos);
    }
    
    // 노브 hit test를 위한 함수
    int hitTestKnob(juce::Point<int> pos) const {
        for (int i = 0; i < knobRects.size(); ++i) {
            if (knobRects[i].contains(pos)) {
                return i;
            }
        }
        return -1;
    }
    
    // Stereo/Mono 버튼의 현재 텍스트를 업데이트하는 함수
    void updateStereoText(const juce::String& text) {
        stereoText = text;
    }
    
    juce::Point<int> center;
    std::vector<float>& knobValues;
    std::vector<juce::Rectangle<int>>& knobRects;
    std::unique_ptr<LED>& statusLED;
    juce::Colour faceColor;
    std::vector<bool>& showValues;
    mutable juce::Rectangle<int> stereoMonoRect;
    juce::String stereoText = "stereo";
    bool bypassActive;
};

class Preset {
public:
    Preset(juce::Colour faceColor) : faceColor(faceColor), ledOn(false) {}
    void setLedOn(bool on) { ledOn = on; }
    
    void draw(juce::Graphics& g, int buttonY) const {
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        
        // preset 텍스트와 언더라인 그리기
        int presetTextWidth = font.getStringWidth("preset");
        int presetX = 80 - presetTextWidth / 2; // 80px 중앙에서 텍스트 중앙 정렬
        
        // preset 텍스트 그리기 (기본 알파값 적용)
        g.setColour(juce::Colours::black.withAlpha(DEFAULT_ALPHA));
        g.setFont(font);
        g.drawText("preset", presetX, buttonY, presetTextWidth, 20, juce::Justification::centredLeft);
        
        // preset 언더라인 그리기 (100% 투명도)
        g.setColour(juce::Colours::black);
        g.drawLine(presetX, buttonY + 17, presetX + presetTextWidth, buttonY + 17, 1.0f);
        
        // preset 글자 왼쪽에 LED 배치 (제일 왼쪽 글자에서 왼쪽으로 8px, 아래로 2px 이동)
        int presetLeftX = presetX; // preset 텍스트의 왼쪽 끝
        int ledX = presetLeftX - 8; // preset 왼쪽 끝에서 왼쪽으로 8px
        int ledY = buttonY + 10 + 2; // 텍스트 중앙 높이에 맞춤 + 아래로 2px
        
        // LED 그리기 (6px 원)
        g.setColour(ledOn ? juce::Colours::green : juce::Colour(0xFF4E564E));
        g.fillEllipse(ledX - 3, ledY - 3, 6, 6);
        
        // preset 글자 오른쪽에 풀다운 버튼 배치 (제일 오른쪽 글자에서 오른쪽으로 8px, 그리고 왼쪽으로 6px 이동, 그리고 오른쪽으로 2px, 그리고 왼쪽으로 1px 이동, 아래로 5px 이동)
        int presetRightX = presetX + presetTextWidth; // preset 텍스트의 오른쪽 끝
        int dropdownX = presetRightX + 8 - 6 + 2 - 1; // preset 오른쪽 끝에서 오른쪽으로 8px, 그리고 왼쪽으로 6px 이동, 그리고 오른쪽으로 2px 이동, 그리고 왼쪽으로 1px 이동
        
        // preset 텍스트의 x-height 기준으로 세로 중앙 정렬, 그리고 아래로 5px 이동
        // x-height는 보통 폰트 높이의 약 60% 정도
        float xHeight = font.getHeight() * 0.6f;
        int dropdownY = buttonY + (20 - xHeight) / 2 + 5; // 20px 버튼 높이에서 x-height 기준 중앙 정렬, 그리고 아래로 5px 이동
        
        // 풀다운 버튼 그리기 (벡터 도형으로 V 모양 삼각형, 40% 스케일, 아래쪽으로 뾰족하게)
        g.setColour(juce::Colours::black);
        juce::Path dropdownPath;
        // 원래 크기: 16px 너비, 8px 높이
        // 40% 스케일: 6.4px 너비, 3.2px 높이
        float scale = 0.4f;
        float width = 16.0f * scale; // 6.4px
        float height = 8.0f * scale; // 3.2px
        
        dropdownPath.startNewSubPath(dropdownX, dropdownY); // 왼쪽 상단
        dropdownPath.lineTo(dropdownX + width/2, dropdownY + height); // 중앙 하단
        dropdownPath.lineTo(dropdownX + width, dropdownY); // 오른쪽 상단
        g.strokePath(dropdownPath, juce::PathStrokeType(1.0f));
    }
    
    bool hitTestPresetButton(juce::Point<int> pos, int buttonY) const {
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int presetTextWidth = font.getStringWidth("preset");
        int presetX = 80 - presetTextWidth / 2;
        return juce::Rectangle<int>(presetX, buttonY, presetTextWidth, 20).contains(pos);
    }
    
    bool hitTestPresetDropdownButton(juce::Point<int> pos, int buttonY) const {
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int presetTextWidth = font.getStringWidth("preset");
        int presetX = 80 - presetTextWidth / 2;
        int presetRightX = presetX + presetTextWidth;
        int dropdownX = presetRightX + 8 - 6 + 2 - 1; // 왼쪽으로 6px 이동, 그리고 오른쪽으로 2px 이동, 그리고 왼쪽으로 1px 이동
        
        // 40% 스케일된 크기로 hit test 영역 조정
        float scale = 0.4f;
        float width = 16.0f * scale; // 6.4px
        float height = 8.0f * scale; // 3.2px
        
        // x-height 기준으로 세로 중앙 정렬, 그리고 아래로 5px 이동
        float xHeight = font.getHeight() * 0.6f;
        int dropdownY = buttonY + (20 - xHeight) / 2 + 5;
        
        return juce::Rectangle<int>(dropdownX, dropdownY, width, height).contains(pos);
    }
    
private:
    juce::Colour faceColor;
    bool ledOn;
};

class Bottom {
public:
    Bottom(juce::Colour faceColor) : faceColor(faceColor), bypassOn(false), preset(faceColor) {}
    
    void setBypassState(bool on) {
        bypassOn = on;
    }
    
    void draw(juce::Graphics& g) const {
        // 세퍼레이터 기준 아래로 23px 위치에서 위로 10px 이동, 그리고 위로 8px 더 이동, 그리고 위로 2px 더 이동 (세퍼레이터 Y=126, 높이 4px)
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2; // 세퍼레이터 아래 끝 + 23px - 10px - 8px - 2px
        
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        
        // in 버튼 - 좌측 정렬, 세퍼레이터 왼쪽 끝에서 우측으로 2px 이동, 기본 알파값 적용
        int inX = 8 + 2; // 세퍼레이터 왼쪽 끝 + 2px
        int inTextWidth = font.getStringWidth("in");
        g.setColour(juce::Colours::black.withAlpha(DEFAULT_ALPHA));
        g.setFont(font);
        g.drawText("in", inX, buttonY, inTextWidth, 20, juce::Justification::centredLeft);
        // in 언더라인 그리기 (100% 투명도)
        g.setColour(juce::Colours::black);
        g.drawLine(inX, buttonY + 17, inX + inTextWidth, buttonY + 17, 1.0f);
        
        // out 버튼 - 우측 정렬, out 오른쪽 끝을 기준으로 정렬하고 왼쪽으로 2px 이동, 기본 알파값 적용
        int outTextWidth = font.getStringWidth("out");
        int outX = 152 - outTextWidth - 2; // 오른쪽 끝 기준 - 텍스트 너비 - 2px
        g.setColour(juce::Colours::black.withAlpha(DEFAULT_ALPHA));
        g.drawText("out", outX, buttonY, outTextWidth, 20, juce::Justification::centredLeft);
        // out 언더라인 그리기 (100% 투명도)
        g.setColour(juce::Colours::black);
        g.drawLine(outX, buttonY + 17, outX + outTextWidth, buttonY + 17, 1.0f);
        
        // preset 클래스 사용하여 그리기
        preset.draw(g, buttonY);
        
        // bypass 버튼 - 중앙 정렬, Face 제일 아래서 위로 4px 이동, 토글 상태에 따른 투명도
        int bypassY = 270 - 4 - 20; // Face 아래에서 4px 위, 버튼 높이 20px 고려
        int bypassTextWidth = font.getStringWidth("bypass");
        int bypassX = 80 - bypassTextWidth / 2; // 80px 중앙에서 텍스트 중앙 정렬
        
        // bypass 토글 상태에 따른 투명도 설정
        float bypassAlpha = bypassOn ? 1.0f : 0.2f; // on일 때 100%, off일 때 20%
        g.setColour(juce::Colours::black.withAlpha(bypassAlpha));
        g.setFont(font);
        g.drawText("bypass", bypassX, bypassY, bypassTextWidth, 20, juce::Justification::centred);
    }
    
    bool hitTestInButton(juce::Point<int> pos) const {
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2;
        int inX = 8 + 2;
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int inTextWidth = font.getStringWidth("in");
        return juce::Rectangle<int>(inX, buttonY, inTextWidth, 20).contains(pos);
    }
    
    bool hitTestOutButton(juce::Point<int> pos) const {
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2;
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int outTextWidth = font.getStringWidth("out");
        int outX = 152 - outTextWidth - 2;
        return juce::Rectangle<int>(outX, buttonY, outTextWidth, 20).contains(pos);
    }
    
    bool hitTestPresetButton(juce::Point<int> pos) const {
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2;
        return preset.hitTestPresetButton(pos, buttonY);
    }
    
    bool hitTestPresetDropdownButton(juce::Point<int> pos) const {
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2;
        return preset.hitTestPresetDropdownButton(pos, buttonY);
    }
    
    bool hitTestBypassButton(juce::Point<int> pos) const {
        int bypassY = 270 - 4 - 20;
        juce::Font font("Euclid Circular B", 15.0f, juce::Font::plain);
        int bypassTextWidth = font.getStringWidth("bypass");
        int bypassX = 80 - bypassTextWidth / 2;
        return juce::Rectangle<int>(bypassX, bypassY, bypassTextWidth, 20).contains(pos);
    }
    
private:
    juce::Colour faceColor;
    bool bypassOn;
    Preset preset;
};

class ClearHostApp : public juce::AudioAppComponent, public juce::AudioProcessorPlayer, public juce::AudioProcessorListener, public juce::Slider::Listener, public juce::ComboBox::Listener, public juce::Button::Listener, public juce::Timer {
public:
    ClearHostApp() {
        animationDuration = 1.0;
        animationTimerInterval = 16;
        originalSystemOutputDevice = "";
        static EuclidLookAndFeel euclidLF;
        juce::LookAndFeel::setDefaultLookAndFeel(&euclidLF);
        
        performAutoSetup();
        pluginManager.addDefaultFormats();
        
        // LED 초기화 (플러그인 로드 전에 먼저 생성)
        pluginStatusLED = std::make_unique<LED>(juce::Point<int>(0, 0), LEDState::OFF);
        
        // Face 초기화
        face = std::make_unique<Face>();
        
        // Panel 초기화 (LED 초기화 후에 생성)
        controlPanel = std::make_unique<Panel>(juce::Point<int>(0, 0), knobValues, knobRects, pluginStatusLED, face->getColor(), knobShowValues);
        
        // Bottom 초기화
        bottom = std::make_unique<Bottom>(face->getColor());
        
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
            
            // LED 상태를 On으로 설정
            if (pluginStatusLED) {
                pluginStatusLED->setState(true);
                juce::Logger::writeToLog("LED set to ON - plugin loaded successfully");
            }
        } else {
            // 플러그인이 로드되지 않았으면 LED를 Off로 설정
            if (pluginStatusLED) {
                pluginStatusLED->setState(false);
                juce::Logger::writeToLog("LED set to OFF - plugin failed to load");
            }
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
    knobShowValues.resize(3, false); // 노브 값 표시 상태
    knobLastValues.resize(3, 1.0f); // 이전 노브 값들 (0~2 범위에서 중간값)
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
        int col1 = 160; // 1번 캔버스 가로를 160px로 고정
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
        // Face 그리기 (좌상단 0,0에서 160x280)
        if (face) {
            face->draw(g);
        }
        // 2단: 가운데 영역 (footer 내용)
        juce::Rectangle<int> centerArea(col1, 0, col2, h);
        g.setColour(juce::Colours::darkgrey.darker());
        g.fillRect(centerArea);
        // Panel 그리기 (노브 3개 + LED + Stereo 토글) - 1번 캔버스로 이동
        juce::Point<int> panelCenter(80, 83); // 노브2(voice) 중앙을 x=80px, y=83px에 위치
        if (controlPanel) {
            // Stereo/Mono 버튼 텍스트 업데이트
            juce::String stereoText = "stereo";
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                if (params.size() > 13 && params[13] && params[13]->getValue() < 0.5f)
                    stereoText = "mono";
            }
            controlPanel->updateStereoText(stereoText);
            
            controlPanel->center = panelCenter;
            controlPanel->draw(g);
        }
        
        // Bottom 그리기
        if (bottom) {
            bottom->draw(g);
        }
        // 프리셋 버튼 7개 (2줄) - TextButtonLike 스타일로 변경
        int btnW = 100, btnH = 30, btnSpacing = 20;
        int btnY1 = centerArea.getY() + 180;
        int btnY2 = btnY1 + btnH + 10;
        juce::String btnNames[7] = {"Bypass","S* up**","Too Loud","Silence","Clear Voice","Dry Voice","Vocal Reference"};
        buttonRects.resize(7);
        for (int i = 0; i < 7; ++i) {
            int row = i < 4 ? 0 : 1;
            int col = row == 0 ? i : i-4;
            int btnX = centerArea.getX() + 30 + col * (btnW + btnSpacing);
            int btnY = row == 0 ? btnY1 : btnY2;
            buttonRects[i] = juce::Rectangle<int>(btnX, btnY, btnW, btnH);
            TextButtonLike presetBtn(btnNames[i], juce::Point<int>(btnX, btnY), DEFAULT_ALPHA);
            presetBtn.draw(g);
        }
        // 장치 선택 박스
        int deviceY = stereoMonoRect.getBottom() + 20;
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
        
        int knobIndex = -1;
        if (parameterIndex == 1 && params.size() > 1 && params[1]) {
            knobIndex = 0;
            knobValues[0] = newValue * 2.0f; // 0~1을 0~2로 변환
        }
        else if (parameterIndex == 14 && params.size() > 14 && params[14]) {
            knobIndex = 1;
            knobValues[1] = newValue * 2.0f; // 0~1을 0~2로 변환
        }
        else if (parameterIndex == 12 && params.size() > 12 && params[12]) {
            knobIndex = 2;
            knobValues[2] = newValue * 2.0f; // 0~1을 0~2로 변환
        }
        
        // 노브 값이 변경되었으면 표시 상태 업데이트
        if (knobIndex >= 0) {
            updateKnobDisplayState(knobIndex, knobValues[knobIndex]);
        }
        
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
            animationTargetValues[i] = targetValues[i] * 2.0; // 0~1 범위를 0~2로 변환
            // 애니메이션 시작 시 모든 노브의 값을 표시하도록 설정
            knobShowValues[i] = true;
        }
        animationStartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        isAnimating = true;
        startTimer(animationTimerInterval);
    }
    
    void setKnobValue(int knobIndex, double value) {
        if (knobIndex >= 0 && knobIndex < knobValues.size()) {
            knobValues[knobIndex] = value;
            updateKnobDisplayState(knobIndex, value);
            if (clearPlugin) {
                auto params = clearPlugin->getParameters();
                int targetParamIndex = -1;
                switch (knobIndex) {
                    case 0: targetParamIndex = 1; break;
                    case 1: targetParamIndex = 14; break;
                    case 2: targetParamIndex = 12; break;
                }
                if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                    // 0~2 범위를 0~1로 변환하여 플러그인에 전달
                    float pluginValue = (float)(value / 2.0);
                    params[targetParamIndex]->setValueNotifyingHost(pluginValue);
                }
            }
        }
    }
    
    void updateKnobDisplayState(int knobIndex, float newValue) {
        if (knobIndex >= 0 && knobIndex < knobValues.size()) {
            // 값이 변경되었으면 표시 상태를 true로 설정
            if (std::abs(newValue - knobLastValues[knobIndex]) > 0.001f) {
                knobShowValues[knobIndex] = true;
                // 애니메이션 중이 아니고 드래그 중이 아닐 때만 타이머 설정
                if (!isAnimating && draggingKnob == -1) {
                    startTimer(500);
                }
            }
            knobLastValues[knobIndex] = newValue;
        }
    }
    
    void resetKnobDisplayStates() {
        for (int i = 0; i < knobShowValues.size(); ++i) {
            knobShowValues[i] = false;
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
                // 애니메이션 완료 후 0.5초 뒤에 레이블로 돌아가기
                startTimer(500);
            }
        } else {
            // 애니메이션이 아닌 경우: 노브 표시 상태를 레이블로 되돌리기
            resetKnobDisplayStates();
            repaint();
            stopTimer();
            
            // 기존의 파라미터 동기화 로직
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
                knobValues[0] = params[1]->getValue() * 2.0f; // 0~1을 0~2로 변환
                juce::Logger::writeToLog("param[1] getValue() after: " + juce::String(knobValues[0]));
            }
        }
        if (params.size() > 14) {
            juce::Logger::writeToLog("param[14] ptr=" + juce::String((uint64_t)params[14]));
            if (params[14]) {
                juce::Logger::writeToLog("param[14] getValue() before");
                knobValues[1] = params[14]->getValue() * 2.0f; // 0~1을 0~2로 변환
                juce::Logger::writeToLog("param[14] getValue() after: " + juce::String(knobValues[1]));
            }
        }
        if (params.size() > 12) {
            juce::Logger::writeToLog("param[12] ptr=" + juce::String((uint64_t)params[12]));
            if (params[12]) {
                juce::Logger::writeToLog("param[12] getValue() before");
                knobValues[2] = params[12]->getValue() * 2.0f; // 0~1을 0~2로 변환
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
        
        // 노브 더블클릭 처리 (1.0으로 리셋 - 중간값)
        if (event.getNumberOfClicks() >= 2) {
            for (int i = 0; i < knobRects.size(); ++i) {
                if (knobRects[i].contains(pos)) {
                    knobValues[i] = 1.0f; // 0~2 범위에서 중간값
                    // 파라미터 반영
                    if (clearPlugin) {
                        auto params = clearPlugin->getParameters();
                        int targetParamIndex = -1;
                        switch (i) {
                            case 0: targetParamIndex = 1; break;
                            case 1: targetParamIndex = 14; break;
                            case 2: targetParamIndex = 12; break;
                        }
                        if (targetParamIndex >= 0 && targetParamIndex < params.size() && params[targetParamIndex]) {
                            params[targetParamIndex]->setValueNotifyingHost(0.5f); // 0~1 범위에서 중간값
                        }
                    }
                    repaint();
                    return;
                }
            }
        }
        
        // Panel의 Stereo/Mono 버튼 클릭 처리
        if (controlPanel && controlPanel->hitTestStereoButton(pos)) {
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
        // Panel의 노브 클릭 처리
        if (controlPanel) {
            int knobIndex = controlPanel->hitTestKnob(pos);
            if (knobIndex >= 0) {
                draggingKnob = knobIndex;
                lastDragY = pos.y;
                // 드래그 시작 시 해당 노브의 값을 표시
                knobShowValues[knobIndex] = true;
                return;
            }
        }
        
        // Bottom 버튼들 클릭 처리
        if (bottom) {
            if (bottom->hitTestInButton(pos)) {
                juce::Logger::writeToLog("In button clicked");
                // TODO: 입력 장치 관련 기능 구현
                repaint();
                return;
            }
            if (bottom->hitTestOutButton(pos)) {
                juce::Logger::writeToLog("Out button clicked");
                // TODO: 출력 장치 관련 기능 구현
                repaint();
                return;
            }
            if (bottom->hitTestPresetButton(pos)) {
                juce::Logger::writeToLog("Preset button clicked");
                // TODO: 프리셋 관련 기능 구현
                repaint();
                return;
            }
            if (bottom->hitTestBypassButton(pos)) {
                juce::Logger::writeToLog("Bypass button clicked");
                // bypass 토글 기능
                static bool bypassState = false;
                bypassState = !bypassState;
                bottom->setBypassState(bypassState);
                setBypassActive(bypassState);
                
                // Panel의 bypass 상태도 업데이트
                if (controlPanel) {
                    controlPanel->setBypassState(bypassState);
                }
                
                // JUCE 플러그인 바이패스 기능 구현
                if (clearPlugin) {
                    // 먼저 플러그인의 바이패스 파라미터를 찾아서 사용
                    auto params = clearPlugin->getParameters();
                    bool bypassParamFound = false;
                    
                    for (int i = 0; i < params.size(); ++i) {
                        if (params[i] && params[i]->getName(100).toLowerCase().contains("bypass")) {
                            // 바이패스 파라미터를 찾았으면 토글
                            float currentValue = params[i]->getValue();
                            float newValue = (currentValue > 0.5f) ? 0.0f : 1.0f;
                            params[i]->setValueNotifyingHost(newValue);
                            juce::Logger::writeToLog("Plugin bypass parameter " + juce::String(i) + " set to: " + juce::String(newValue));
                            bypassParamFound = true;
                            break;
                        }
                    }
                    
                    // 플러그인에 바이패스 파라미터가 없는 경우, AudioProcessor의 내장 바이패스 기능 사용
                    if (!bypassParamFound) {
                        // AudioProcessor의 바이패스 기능 사용
                        if (clearPlugin->getBypassParameter()) {
                            clearPlugin->getBypassParameter()->setValueNotifyingHost(bypassState ? 1.0f : 0.0f);
                            juce::Logger::writeToLog("AudioProcessor bypass set to: " + juce::String(bypassState ? "ON" : "OFF"));
                        } else {
                            // 바이패스 파라미터가 없는 경우, 플러그인을 직접 바이패스
                            // 이는 플러그인이 JUCE의 표준 바이패스 기능을 지원하지 않는 경우
                            juce::Logger::writeToLog("Plugin does not support bypass functionality");
                        }
                    }
                }
                
                repaint();
                return;
            }
            if (bottom->hitTestPresetDropdownButton(pos)) {
                juce::Logger::writeToLog("Preset dropdown button clicked");
                // TODO: 프리셋 드롭다운 메뉴 구현
                repaint();
                return;
            }
        }
        // 프리셋 버튼 7개 - TextButtonLike hit test 사용
        juce::String btnNames[7] = {"Bypass","S* up**","Too Loud","Silence","Clear Voice","Dry Voice","Vocal Reference"};
        for (int i = 0; i < 7; ++i) {
            TextButtonLike tempBtn(btnNames[i], juce::Point<int>(buttonRects[i].getX(), buttonRects[i].getY()), DEFAULT_ALPHA);
            if (tempBtn.hitTest(pos)) {
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
            float delta = dy * 0.008f; // 감도를 0.005f에서 0.008f로 증가 (약 60% 더 민감하게)
            knobValues[draggingKnob] = juce::jlimit(0.0f, 2.0f, knobValues[draggingKnob] + delta); // 0~2 범위로 변경
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
                    // 0~2 범위를 0~1로 변환하여 플러그인에 전달
                    float pluginValue = knobValues[draggingKnob] / 2.0f;
                    params[targetParamIndex]->setValueNotifyingHost(pluginValue);
                }
            }
            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent&) override {
        if (draggingKnob >= 0) {
            // 드래그가 끝났을 때 0.5초 후에 레이블로 돌아가도록 타이머 설정
            startTimer(500);
        }
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
    std::vector<bool> knobShowValues; // 노브 값 표시 상태
    std::vector<float> knobLastValues; // 이전 노브 값들
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
    
    // LED 상태 표시
    std::unique_ptr<LED> pluginStatusLED;
    bool pluginLoaded = false;
    bool bypassActive = false;
    
    // Panel (노브 3개 + LED + Stereo 토글을 하나로 묶음)
    std::unique_ptr<Panel> controlPanel;
    
    // Face (1번 캔버스 배경)
    std::unique_ptr<Face> face;
    
    // Bottom 클래스 추가
    std::unique_ptr<Bottom> bottom;
    
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
                    setPluginLoaded(true);
                    
                    // 앱 실행 시 stereo/mono 파라미터를 stereo로 설정
                    auto params = clearPlugin->getParameters();
                    if (params.size() > 13 && params[13]) {
                        params[13]->setValueNotifyingHost(1.0f); // stereo로 설정
                        juce::Logger::writeToLog("Set stereo/mono parameter to stereo (VST3)");
                    }
                } else {
                    juce::Logger::writeToLog("Failed to load Clear VST3: " + errorMessage);
                    setPluginLoaded(false);
                    
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
                                setPluginLoaded(true);
                                
                                // 앱 실행 시 stereo/mono 파라미터를 stereo로 설정
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo로 설정
                                    juce::Logger::writeToLog("Set stereo/mono parameter to stereo (AU)");
                                }
                            } else {
                                juce::Logger::writeToLog("Failed to load Clear AU: " + auError);
                                setPluginLoaded(false);
                            }
                        }
                    }
                }
            } else {
                juce::Logger::writeToLog("VST3 format not found");
            }
        } else {
            juce::Logger::writeToLog("Clear.vst3 not found");
            setPluginLoaded(false);
        }
    }
    // LED 상태 업데이트 메서드들
    void updateLEDState() {
        if (!pluginStatusLED) return;
        
        if (!pluginLoaded) {
            // 플러그인이 로딩되지 않았으면 빨간색으로 표시
            pluginStatusLED->setState(LEDState::PLUGIN_OFF);
        } else if (bypassActive) {
            // 플러그인이 로딩되었고 바이패스가 활성화되었으면 어두운 녹색
            pluginStatusLED->setState(LEDState::BYPASS_ON);
        } else {
            // 플러그인이 로딩되었고 바이패스가 비활성화되었으면 밝은 녹색
            pluginStatusLED->setState(LEDState::PLUGIN_ON);
        }
        
        repaint();
    }
    
    void setPluginLoaded(bool loaded) {
        pluginLoaded = loaded;
        updateLEDState();
    }
    
    void setBypassActive(bool active) {
        bypassActive = active;
        updateLEDState();
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
    const juce::String getApplicationName() override { return "clr"; }
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
