#include <JuceHeader.h>
#include <map>

// 기본 투명도 설정 (80%)
static constexpr float DEFAULT_ALPHA = 0.8f;
// 폰트 크기 상수 정의
static constexpr float DEFAULT_FONT_SIZE = 16.0f;
// 노브 주변 점 표시 여부
static constexpr bool KnobDot = false;

void drawDropdownList(juce::Graphics& g,
                      const std::vector<juce::String>& items,
                      const juce::String& selectedItem,
                      juce::Rectangle<int> dropdownRect,
                      int scrollOffset,
                      int maxVisible,
                      int itemHeight,
                      std::vector<juce::Rectangle<int>>& outRects,
                      juce::Justification textJustify,
                      int textPad = 0,
                      float alpha = DEFAULT_ALPHA,
                      juce::Colour textColor = juce::Colours::black) {
    outRects.clear();
    int visibleCount = std::min(maxVisible, (int)items.size() - scrollOffset);
    g.setFont(juce::Font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain));
    for (int i = 0; i < visibleCount; ++i) {
        int actualIndex = i + scrollOffset;
        juce::Rectangle<int> itemRect(dropdownRect.getX(), dropdownRect.getY() + i * itemHeight, dropdownRect.getWidth(), itemHeight);
        outRects.push_back(itemRect);
        bool isSelected = (items[actualIndex] == selectedItem);
        g.setColour(textColor.withAlpha(alpha));
        g.drawText(items[actualIndex].toLowerCase(), itemRect.reduced(textPad, 0), textJustify);
        if (isSelected) {
            int textWidth = g.getCurrentFont().getStringWidth(items[actualIndex].toLowerCase());
            int underlineY = itemRect.getY() + 15;
            int underlineX;
            if (textJustify == juce::Justification::centredRight) {
                underlineX = itemRect.getX() + itemRect.getWidth() - textWidth - textPad;
            } else if (textJustify == juce::Justification::centred) {
                underlineX = itemRect.getX() + (itemRect.getWidth() - textWidth) / 2;
            } else {
                underlineX = itemRect.getX() + textPad;
            }
            g.setColour(textColor.withAlpha(alpha));
            g.drawLine(underlineX, underlineY, underlineX + textWidth, underlineY, 1.0f);
        }
    }
}

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
    Face() : color(juce::Colour(0xFFFF9625)) {
        loadSavedColor();
        updateMode();
    }
    
    void draw(juce::Graphics& g) const {
        g.setColour(color);
        g.fillRect(0, 0, 160, 265); // 160x265 크기, 좌상단 (0,0) 시작
        
        // 세퍼레이터 그리기
        // 크기: 145x4, 중앙정렬 센터 좌표 (80,126), 모서리 라운드 2px, 흰색 투명도 20%
        int separatorWidth = 145;
        int separatorHeight = 4;
        int separatorX = 80 - separatorWidth / 2; // 중앙정렬
        int separatorY = 121 - separatorHeight / 2; // 중앙정렬 (5px 아래로 이동)
        
        g.setColour(juce::Colours::white.withAlpha(0.2f)); // 흰색, 투명도 20%
        g.fillRoundedRectangle(separatorX, separatorY, separatorWidth, separatorHeight, 2.0f);
    }
    
    juce::Colour getColor() const {
        return color;
    }
    
    void setColor(juce::Colour newColor) {
        color = newColor;
        updateMode();
        saveColor();
    }
    
    bool isDarkMode() const {
        return darkMode;
    }
    
    juce::Colour getTextColor() const {
        return darkMode ? juce::Colours::white : juce::Colours::black;
    }
    
private:
    void updateMode() {
        // 컬러의 실제 명도(luminance) 계산 (0.0~1.0)
        // RGB to Luminance: 0.299*R + 0.587*G + 0.114*B
        float r = color.getRed() / 255.0f;
        float g = color.getGreen() / 255.0f;
        float b = color.getBlue() / 255.0f;
        float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
        darkMode = luminance <= 0.3f; // 30% 이하이면 다크모드
        
        // 디버깅용 로그
        juce::Logger::writeToLog("Face color: " + color.toString() + 
                                ", Luminance: " + juce::String(luminance, 3) + 
                                ", Dark mode: " + (darkMode ? "ON" : "OFF"));
    }
    
    void saveColor() {
        juce::File colorFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                              .getChildFile("ClearHost")
                              .getChildFile("face_color.conf");
        colorFile.getParentDirectory().createDirectory();
        
        juce::PropertiesFile settings(colorFile, juce::PropertiesFile::Options());
        settings.setValue("faceColor", color.toString());
        settings.saveIfNeeded();
    }
    
    void loadSavedColor() {
        juce::File colorFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                              .getChildFile("ClearHost")
                              .getChildFile("face_color.conf");
        
        if (colorFile.existsAsFile()) {
            juce::PropertiesFile settings(colorFile, juce::PropertiesFile::Options());
            juce::String savedColor = settings.getValue("faceColor", "");
            if (savedColor.isNotEmpty()) {
                color = juce::Colour::fromString(savedColor);
            }
        }
    }
    
    juce::Colour color;
    bool darkMode = false;
};

class KnobWithLabel {
public:
    KnobWithLabel(juce::Rectangle<int> area, float value, const juce::String& labelText, juce::Colour knobColour, juce::Colour labelColour, juce::Colour indicatorColour, bool showValue = false)
        : area(area), value(value), labelText(labelText), knobColour(knobColour), labelColour(labelColour), indicatorColour(indicatorColour), showValue(showValue) {}

    void draw(juce::Graphics& g, juce::Point<int> mousePos = juce::Point<int>(-1, -1)) const {
        // 노브 중심점
        float cx = area.getCentreX();
        float cy = area.getCentreY();
        float radius = 19.0f;
        
        // 마우스 위치가 유효한 경우 그림자 계산
        if (mousePos.x >= 0 && mousePos.y >= 0) {
            // 마우스와 노브 중심 간의 벡터 계산
            float dx = mousePos.x - cx;
            float dy = mousePos.y - cy;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            float maxDistance = 100.0f; // 최대 거리 100px
            float normalizedDistance = std::min(distance / maxDistance, 1.0f);
            
            // 그림자 offset: 1px ~ 6.6px (기존의 60%)
            float shadowOffset = 1.0f + normalizedDistance * 6.0f;
            // 그림자 스케일: 고정값 1.05
            float shadowScale = 1.05f;
            // 그림자 알파: 고정값 0.2(20%)
            float shadowAlpha = 0.2f;
            
            if (distance > 0) {
                float normalizedDx = -dx / distance; // 반대 방향
                float normalizedDy = -dy / distance; // 반대 방향
                float shadowOffsetX = normalizedDx * shadowOffset;
                float shadowOffsetY = normalizedDy * shadowOffset;
                // 그림자 그리기
                g.setColour(juce::Colours::black.withAlpha(shadowAlpha));
                float shadowRadius = radius * shadowScale;
                g.fillEllipse(cx - shadowRadius + shadowOffsetX, cy - shadowRadius + shadowOffsetY, shadowRadius * 2, shadowRadius * 2);
            }
        } else {
            // 음수 좌표 처리: 노브 중심부터 마우스까지의 거리 계산
            float dx, dy;
            
            // X축 음수 좌표 처리
            if (mousePos.x < 0) {
                // 노브 중심부터 마우스까지의 거리 (음수 좌표는 절댓값으로 처리)
                dx = mousePos.x - cx;
            } else {
                dx = mousePos.x - cx;
            }
            
            // Y축 음수 좌표 처리
            if (mousePos.y < 0) {
                // 노브 중심부터 마우스까지의 거리 (음수 좌표는 절댓값으로 처리)
                dy = mousePos.y - cy;
            } else {
                dy = mousePos.y - cy;
            }
            
            float distance = std::sqrt(dx * dx + dy * dy);
            
            float maxDistance = 100.0f; // 최대 거리 100px
            float normalizedDistance = std::min(distance / maxDistance, 1.0f);
            
            // 그림자 offset: 1px ~ 6.6px (기존의 60%)
            float shadowOffset = 1.0f + normalizedDistance * 6.0f;
            // 그림자 스케일: 고정값 1.05
            float shadowScale = 1.05f;
            // 그림자 알파: 고정값 0.2(20%)
            float shadowAlpha = 0.2f;
            
            if (distance > 0) {
                float normalizedDx = -dx / distance; // 반대 방향
                float normalizedDy = -dy / distance; // 반대 방향
                float shadowOffsetX = normalizedDx * shadowOffset;
                float shadowOffsetY = normalizedDy * shadowOffset;
                // 그림자 그리기
                g.setColour(juce::Colours::black.withAlpha(shadowAlpha));
                float shadowRadius = radius * shadowScale;
                g.fillEllipse(cx - shadowRadius + shadowOffsetX, cy - shadowRadius + shadowOffsetY, shadowRadius * 2, shadowRadius * 2);
            }
        }
        
        // 노브 원
        g.setColour(knobColour);
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
        
        // 인디케이터 - Face 색상 사용
        // 270도 범위로 확장: minAngle을 15도 더 반시계, maxAngle을 15도 더 시계방향
        float minAngle = juce::MathConstants<float>::pi * 5.0f/6.0f - juce::MathConstants<float>::pi * 15.0f/180.0f; // 15도 반시계 추가
        float maxAngle = juce::MathConstants<float>::pi * 13.0f/6.0f + juce::MathConstants<float>::pi * 15.0f/180.0f; // 15도 시계방향 추가
        // 0~2 범위를 0~1로 변환하여 각도 계산 (270도 회전 범위)
        float normalizedValue = value / 2.0f;
        float angle = minAngle + normalizedValue * (maxAngle - minAngle);
        float startX = cx + std::cos(angle) * (radius + 0.0f); // 시작점을 노브 중심쪽으로 1px 이동
        float startY = cy + std::sin(angle) * (radius + 0.0f);
        float endX = cx + std::cos(angle) * (radius - DEFAULT_FONT_SIZE - 1.0f); // 끝점을 노브 중심쪽으로 1px 이동
        float endY = cy + std::sin(angle) * (radius - DEFAULT_FONT_SIZE - 1.0f);
        g.setColour(indicatorColour); // Face 색상으로 변경
        g.drawLine(startX, startY, endX, endY, 4.0f);
        
        // 노브 주변 점 2개 그리기 (KnobDot 변수에 따라 조건부 표시)
        if (KnobDot) {
            float dotDistance = 24.0f; // 점과 노브 중심간의 거리 (+1px)
            float dotRadius = 1.5f; // 점 크기 (지름 3px, -1px)
            
            // 1번점: 225도 - 90도 = 135도
            float dot1Angle = juce::MathConstants<float>::pi + juce::MathConstants<float>::pi * 45.0f / 180.0f - juce::MathConstants<float>::pi * 90.0f / 180.0f; // 135도
            float dot1X = cx + std::cos(dot1Angle) * dotDistance;
            float dot1Y = cy + std::sin(dot1Angle) * dotDistance;
            
            // 2번점: 135도 - 90도 = 45도
            float dot2Angle = juce::MathConstants<float>::pi - juce::MathConstants<float>::pi * 45.0f / 180.0f - juce::MathConstants<float>::pi * 90.0f / 180.0f; // 45도
            float dot2X = cx + std::cos(dot2Angle) * dotDistance;
            float dot2Y = cy + std::sin(dot2Angle) * dotDistance;
            
            // 점 그리기 (노브와 동일한 컬러, 알파값 적용)
            g.setColour(knobColour);
            g.fillEllipse(dot1X - dotRadius, dot1Y - dotRadius, dotRadius * 2, dotRadius * 2);
            g.fillEllipse(dot2X - dotRadius, dot2Y - dotRadius, dotRadius * 2, dotRadius * 2);
        }
        
        // 레이블 또는 값 표시
        int labelW = area.getWidth();
        int labelH = 20;
        int labelX = area.getX();
        int labelY = static_cast<int>(cy + radius - 1); // 노브 원 아래 -1px (2px 아래)
        g.setColour(labelColour);
        g.setFont(juce::Font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain));
        
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
    KnobCluster(juce::Point<int> center, std::vector<float>& values, std::vector<juce::Rectangle<int>>& rects, juce::Colour faceColor, std::vector<bool>& showValues, float alpha = 1.0f, juce::Colour textColor = juce::Colours::black)
        : center(center), values(values), knobRects(rects), faceColor(faceColor), showValues(showValues), alpha(alpha), textColor(textColor) {}
    void draw(juce::Graphics& g, juce::Point<int> mousePos = juce::Point<int>(-1, -1)) const {
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
                textColor.withAlpha(alpha),
                textColor.withAlpha(alpha),
                faceColor, // 인디케이터는 알파값 적용 안함
                showValues[i]
            );
            knob.draw(g, mousePos);
        }
    }
    // 값을 외부에서 바꿀 수 있도록 참조로 보관
    std::vector<float>& values;
    std::vector<juce::Rectangle<int>>& knobRects;
    juce::Point<int> center;
    juce::Colour faceColor;
    std::vector<bool>& showValues;
    float alpha;
    juce::Colour textColor;
};

class TextButtonLike {
public:
    TextButtonLike(const juce::String& text, juce::Point<int> position, float alpha = DEFAULT_ALPHA, juce::Colour textColor = juce::Colours::black)
        : text(text), position(position), alpha(alpha), textColor(textColor) {}
    void draw(juce::Graphics& g) const {
        g.setColour(textColor.withAlpha(alpha));
        g.setFont(juce::Font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain));
        
        // 텍스트 크기 계산
        int textW = g.getCurrentFont().getStringWidth(text);
        int textH = (int)g.getCurrentFont().getHeight();
        
        // 텍스트 그리기
        g.drawText(text, position.x, position.y, textW, textH, juce::Justification::centred);
        
        // Underline - 텍스트 기준으로 위치 계산
        int underlineY = position.y + textH - 1; // 텍스트 아래 0px (8px 아래로 이동)
        int underlineW = textW;
        int underlineX = position.x;
        g.setColour(textColor.withAlpha(alpha));
        g.drawLine(underlineX, underlineY, underlineX + underlineW, underlineY, 1.0f);
    }
    bool hitTest(juce::Point<int> pos) const { 
        // 텍스트 크기에 맞춰서 상하좌우 3px 확장
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int textW = font.getStringWidth(text);
        int textH = (int)font.getHeight();
        return juce::Rectangle<int>(position.x - 3, position.y - 3, textW + 6, textH + 6).contains(pos); 
    }
    juce::String text;
    juce::Point<int> position;
    float alpha;
    juce::Colour textColor;
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
                ledColour = juce::Colour(0x4D000000); // 검정색, 30% 알파값 (0x4D = 77/255 ≈ 30%)
                break;
            case LEDState::PLUGIN_ON:
                ledColour = juce::Colour(0xFF61C554); // 밝은 녹색(#61c554)
                break;
            case LEDState::PLUGIN_OFF:
                ledColour = juce::Colour(0xFFB23636); // 빨간색
                break;
            case LEDState::BYPASS_ON:
                ledColour = juce::Colour(0x4D000000); // 검정색, 30% 알파값 (preset LED OFF와 동일)
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

class RecButton {
public:
    RecButton() : isOn(false), blinkTimer(0), blinkState(false) {}
    
    void toggle() { 
        isOn = !isOn; 
        if (isOn) {
            blinkTimer = 0;
            blinkState = true;
        }
    }
    
    void update() {
        if (isOn) {
            blinkTimer++;
            if (blinkTimer >= 30) { // 0.5초 (60fps 기준 30프레임)
                blinkState = !blinkState;
                blinkTimer = 0;
            }
        }
    }
    
    void draw(juce::Graphics& g, juce::Point<int> center, bool bypassActive = false) const {
        if (!isOn) {
            // OFF 상태: 빨간색 (RGB 255,0,0)
            g.setColour(juce::Colour(255, 0, 0).withAlpha(bypassActive ? 0.3f : 1.0f));
        } else {
            // ON 상태: 깜빡임 효과 - preset LED OFF 색과 동일한 컬러 사용
            if (blinkState) {
                g.setColour(juce::Colour(255, 0, 0).withAlpha(bypassActive ? 0.3f : 1.0f)); // bypass 상태에 따른 알파값 적용
            } else {
                // 검정색, 30% 알파값 (preset LED OFF와 동일) - bypass 상태일 때는 추가 알파값 적용
                float alpha = bypassActive ? 0.3f * 0.3f : 0.3f; // bypass on일 때 30% * 30% = 9%, off일 때 30%
                g.setColour(juce::Colour(0, 0, 0).withAlpha(alpha));
            }
        }
        
        // LED와 동일한 모양 (6px 원)
        g.fillEllipse(center.x - 3, center.y - 3, 6, 6);
    }
    
    bool hitTest(juce::Point<int> pos) const {
        // separator 우측 상단 기준으로 위로 13px, 좌로 13px 위치 계산
        int separatorWidth = 145;
        int separatorX = 80 - separatorWidth / 2; // 중앙정렬
        int separatorY = 121 - 4 / 2; // 중앙정렬 (5px 아래로 이동)
        juce::Point<int> recCenter(separatorX + separatorWidth - 13, separatorY - 13); // separator 우측 상단에서 위로 13px, 좌로 13px
        
        float distance = pos.getDistanceFrom(recCenter);
        return distance <= 8.0f; // 6px 원의 반지름 + 5px 확장 = 8px
    }
    
    bool isActive() const { return isOn; }
    
    void setCenter(juce::Point<int> newCenter) { center = newCenter; }
    
    juce::Point<int> center;
    
private:
    bool isOn;
    int blinkTimer;
    bool blinkState;
};

class Panel {
public:
    Panel(juce::Point<int> center, std::vector<float>& knobValues, std::vector<juce::Rectangle<int>>& knobRects, std::unique_ptr<LED>& statusLED, juce::Colour faceColor, std::vector<bool>& showValues, juce::Colour textColor = juce::Colours::black)
        : center(center), knobValues(knobValues), knobRects(knobRects), statusLED(statusLED), faceColor(faceColor), showValues(showValues), textColor(textColor), bypassActive(false) {}
    
    void setBypassState(bool bypassOn) {
        bypassActive = bypassOn;
    }
    
    void setTextColor(juce::Colour color) { textColor = color; }

    void draw(juce::Graphics& g, juce::Point<int> mousePos = juce::Point<int>(-1, -1)) const {
        // bypass 상태에 따른 알파값 설정 (LED와 노브 인디케이터 제외)
        float alpha = bypassActive ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
        
        // 1. 노브 3개 그리기 (알파값 적용, 단 인디케이터는 제외)
        KnobCluster cluster(center, knobValues, knobRects, faceColor, showValues, alpha, textColor);
        cluster.draw(g, mousePos);
        
        // 2. LED 그리기 (2번 노브 중앙 기준, 위로 69px) - LED는 알파값 적용 안함
        if (knobRects.size() >= 2 && statusLED) {
            juce::Point<int> ledCenter = knobRects[1].getCentre(); // 2번 노브(voice) 중앙
            ledCenter.y -= 64; // 위로 64px (5px 아래로 이동)
            statusLED->center = ledCenter;
            statusLED->draw(g);
        }
        
        // 3. Rec 버튼 그리기 (separator 우측 상단 기준으로 위로 13px, 좌로 13px)
        int separatorWidth = 145;
        int separatorX = 80 - separatorWidth / 2; // 중앙정렬
        int separatorY = 121 - 4 / 2; // 중앙정렬 (5px 아래로 이동)
        juce::Point<int> recCenter(separatorX + separatorWidth - 13, separatorY - 13); // separator 우측 상단에서 위로 13px, 좌로 13px
        recButton.draw(g, recCenter, bypassActive);
        
        // 4. Stereo/Mono 토글 버튼 그리기 (알파값 적용)
        // stereoText는 외부에서 updateStereoText()로 업데이트됨
        
        int stereoX = knobRects[1].getCentreX();
        int stereoY = center.y - 51; // 노브 클러스터 중앙에서 위로 51px (5px 아래로 이동)
        
        // 텍스트 크기에 맞춰서 위치 계산
        g.setFont(juce::Font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain));
        int textW = g.getCurrentFont().getStringWidth(stereoText);
        int textH = (int)g.getCurrentFont().getHeight();
        int stereoStartX = stereoX - textW/2;
        
        stereoMonoRect = juce::Rectangle<int>(stereoStartX, stereoY, textW, textH);
        TextButtonLike stereoBtn(stereoText, juce::Point<int>(stereoStartX, stereoY), alpha, textColor);
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
    
    void setFaceColor(juce::Colour color) {
        faceColor = color;
    }
    
    // Rec 버튼 관련 메서드들
    void updateRecButton() {
        recButton.update();
    }
    
    bool hitTestRecButton(juce::Point<int> pos) const {
        return recButton.hitTest(pos);
    }
    
    void toggleRecButton() {
        recButton.toggle();
    }
    
    bool isRecButtonActive() const {
        return recButton.isActive();
    }
    
    // LED hit test를 위한 함수
    bool hitTestLED(juce::Point<int> pos) const {
        if (knobRects.size() >= 2 && statusLED) {
            juce::Point<int> ledCenter = knobRects[1].getCentre(); // 2번 노브(voice) 중앙
            ledCenter.y -= 64; // 위로 64px (5px 아래로 이동)
            float distance = pos.getDistanceFrom(ledCenter);
            return distance <= 3.0f; // 6px 원의 반지름
        }
        return false;
    }
    
    juce::Point<int> center;
    std::vector<float>& knobValues;
    std::vector<juce::Rectangle<int>>& knobRects;
    std::unique_ptr<LED>& statusLED;
    juce::Colour faceColor;
    std::vector<bool>& showValues;
    juce::Colour textColor;
    mutable juce::Rectangle<int> stereoMonoRect;
    juce::String stereoText = "stereo";
    bool bypassActive;
    RecButton recButton;
};

class Preset {
public:
    Preset(juce::Colour faceColor, juce::Colour textColor = juce::Colours::black) : faceColor(faceColor), textColor(textColor), ledOn(false), labelText("preset") {}
    void setLedOn(bool on) { ledOn = on; }
    void setLabelText(const juce::String& text) { labelText = text; }
    void setTextColor(juce::Colour color) { textColor = color; }
    juce::String getLabelText() const { return labelText; }
    
    void draw(juce::Graphics& g, int buttonY, float alpha = DEFAULT_ALPHA) const {
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        
        // 동적 레이블 텍스트와 언더라인 그리기
        int presetTextWidth = font.getStringWidth(labelText);
        int presetX = 80 - presetTextWidth / 2; // 80px 중앙에서 텍스트 중앙 정렬
        
        // preset 텍스트 그리기 (bypass 상태에 따른 알파값 적용)
        g.setColour(textColor.withAlpha(alpha));
        g.setFont(font);
        g.drawText(labelText, presetX, buttonY, presetTextWidth, 20, juce::Justification::centredLeft);
        
        // preset 언더라인 그리기 (bypass 상태에 따른 알파값 적용)
        g.setColour(textColor.withAlpha(alpha));
        g.drawLine(presetX, buttonY + 17, presetX + presetTextWidth, buttonY + 17, 1.0f);
        
        // preset 글자 왼쪽에 LED 배치 (제일 왼쪽 글자에서 왼쪽으로 8px, 아래로 2px 이동)
        int presetLeftX = presetX; // preset 텍스트의 왼쪽 끝
        int ledX = presetLeftX - 8; // preset 왼쪽 끝에서 왼쪽으로 8px
        int ledY = buttonY + 10 + 2; // 텍스트 중앙 높이에 맞춤 + 아래로 2px
        
        // LED 그리기 (6px 원) - 제일 위 상태 LED와 동일한 색상 사용
        g.setColour(ledOn ? juce::Colour(0xFF61C554) : juce::Colour(0x4D000000));
        g.fillEllipse(ledX - 3, ledY - 3, 6, 6);
        
        // preset 글자 오른쪽에 풀다운 버튼 배치 (제일 오른쪽 글자에서 오른쪽으로 8px, 그리고 왼쪽으로 6px 이동, 그리고 오른쪽으로 2px, 그리고 왼쪽으로 1px 이동, 아래로 5px 이동)
        // int presetRightX = presetX + presetTextWidth; // preset 텍스트의 오른쪽 끝
        // int dropdownX = presetRightX + 8 - 6 + 2 - 1; // preset 오른쪽 끝에서 오른쪽으로 8px, 그리고 왼쪽으로 6px 이동, 그리고 오른쪽으로 2px 이동, 그리고 왼쪽으로 1px 이동
        
        // preset 텍스트의 x-height 기준으로 세로 중앙 정렬, 그리고 아래로 5px 이동
        // x-height는 보통 폰트 높이의 약 60% 정도
        // float xHeight = font.getHeight() * 0.6f;
        // int dropdownY = buttonY + (20 - xHeight) / 2 + 5; // 20px 버튼 높이에서 x-height 기준 중앙 정렬, 그리고 아래로 5px 이동
        
        // 풀다운 버튼 그리기 (벡터 도형으로 V 모양 삼각형, 40% 스케일, 아래쪽으로 뾰족하게) - 히든 처리
        // g.setColour(juce::Colours::black);
        // juce::Path dropdownPath;
        // 원래 크기: 16px 너비, 8px 높이
        // 40% 스케일: 6.4px 너비, 3.2px 높이
        // float scale = 0.4f;
        // float width = 16.0f * scale; // 6.4px
        // float height = 8.0f * scale; // 3.2px
        
        // dropdownPath.startNewSubPath(dropdownX, dropdownY); // 왼쪽 상단
        // dropdownPath.lineTo(dropdownX + width/2, dropdownY + height); // 중앙 하단
        // dropdownPath.lineTo(dropdownX + width, dropdownY); // 오른쪽 상단
        // g.strokePath(dropdownPath, juce::PathStrokeType(1.0f));
    }
    
    bool hitTestPresetButton(juce::Point<int> pos, int buttonY) const {
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int presetTextWidth = font.getStringWidth(labelText);
        int presetX = 80 - presetTextWidth / 2;
        return juce::Rectangle<int>(presetX - 5, buttonY - 5, presetTextWidth + 10, 20 + 10).contains(pos);
    }
    
    bool hitTestPresetDropdownButton(juce::Point<int> pos, int buttonY) const {
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int presetTextWidth = font.getStringWidth(labelText);
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
    juce::Colour textColor;
    bool ledOn;
    juce::String labelText;
};

class ColorPicker {
public:
    ColorPicker() : isOpen(false), selectedColor(juce::Colour(0xFFFF9625)), paletteY(265) { // 5px 아래로 이동
        generatePalette();
    }
    void setPosition(juce::Point<int> pos) { position = pos; }
    void setPaletteY(int) { paletteY = 265 - paletteH; } // 팔레트 아래쪽이 y=265에 맞게 (5px 아래로 이동)
    void draw(juce::Graphics& g, juce::Colour textColor = juce::Colours::black) const {
        if (isOpen) drawPalette(g);
        
        // 라이트/다크모드에 따라 SVG 파일 선택 (로고와 동일한 조건)
        bool isDarkMode = (textColor == juce::Colours::white);
        juce::String svgFileName = isDarkMode ? "picker_w.svg" : "picker_b.svg";
        // 실행 파일 위치 기준으로 Resources 디렉토리 찾기
        juce::File execFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        juce::File resourcesDir = execFile.getParentDirectory().getParentDirectory().getChildFile("Resources");
        juce::File svgFile = resourcesDir.getChildFile(svgFileName);
        
        // SVG 아이콘 그리기 (8px x 1.3 = 10.4px, 반올림하여 10px)
        int iconSize = 10;
        if (svgFile.existsAsFile()) {
            std::unique_ptr<juce::Drawable> drawable = juce::Drawable::createFromSVGFile(svgFile);
            if (drawable) {
                g.setColour(juce::Colours::white); // SVG 자체 색상 사용, 투명도만 적용
                drawable->drawWithin(g, juce::Rectangle<float>(position.x - iconSize/2, position.y - iconSize/2, iconSize, iconSize), juce::RectanglePlacement::centred, 1.0f);
            } else {
                // SVG 로드 실패 시 기본 원형 아이콘
                float radius = 4.0f;
                g.setColour(selectedColor);
                g.fillEllipse(position.x - radius, position.y - radius, radius * 2, radius * 2);
            }
        } else {
            // SVG 파일이 없으면 기본 원형 아이콘
            float radius = 4.0f;
            g.setColour(selectedColor);
            g.fillEllipse(position.x - radius, position.y - radius, radius * 2, radius * 2);
        }
    }
          static constexpr int paletteX = -1; // 오른쪽으로 1px 이동
      static constexpr int cellSizeY = 16;
      static constexpr int cellSizeX = 18;
      static constexpr int cols = 9;
      static constexpr int rows = 7;
      static constexpr int paletteW = 159; // 160 - 1 = 159픽셀
      static constexpr int paletteH = rows * cellSizeY; // 전체 높이 10px 줄임
    bool hitTest(juce::Point<int> pos) const {
        if (isOpen && hitTestPalette(pos)) return false; // 팔레트 클릭시 버튼은 hit 안됨
        float radius = 4.0f;
        float distance = pos.getDistanceFrom(position);
        return distance <= radius;
    }
    bool hitTestPalette(juce::Point<int> pos) const {
        if (!isOpen) return false;
        juce::Rectangle<int> paletteRect(paletteX, paletteY, paletteW, paletteH);
        return paletteRect.contains(pos);
    }
    juce::Colour getColorAtPosition(juce::Point<int> pos) const {
        if (!isOpen) return selectedColor;
        juce::Rectangle<int> paletteRect(paletteX, paletteY, paletteW, paletteH);
        if (paletteRect.contains(pos)) {
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    int x = paletteX + col * cellSizeX;
                    int y = paletteY + row * cellSizeY;
                    juce::Rectangle<int> colorRect(x, y, cellSizeX, cellSizeY);
                    if (colorRect.contains(pos)) {
                        return colors[row][col];
                    }
                }
            }
        }
        return selectedColor;
    }
    void toggle() { isOpen = !isOpen; }
    void close() { isOpen = false; }
    juce::Colour getSelectedColor() const { return selectedColor; }
    void setSelectedColor(juce::Colour color) { selectedColor = color; }
    bool isPaletteOpen() const { return isOpen; }
private:
    void generatePalette() {
        // 첨부 이미지에서 각 네모의 좌10~우20% 영역의 RGB값을 추정하여 9x7 배열로 지정
        colors = {
            { juce::Colour(0xffededed), juce::Colour(0xffe1e1e1), juce::Colour(0xffcccccc), juce::Colour(0xffa6a6a6), juce::Colour(0xff8e8e8e), juce::Colour(0xff6e6e6e), juce::Colour(0xff4a4a4a), juce::Colour(0xff2c2c2c), juce::Colour(0xff191919) },
            { juce::Colour(0xffffe0ef), juce::Colour(0xfff3b3c9), juce::Colour(0xfff07cae), juce::Colour(0xffe94d8c), juce::Colour(0xffc13a6b), juce::Colour(0xffa02c4e), juce::Colour(0xff7a1d3a), juce::Colour(0xff5a1530), juce::Colour(0xff3d0e22) },
            { juce::Colour(0xffffe6e6), juce::Colour(0xfff7b3b3), juce::Colour(0xfff07c7c), juce::Colour(0xffe94d4d), juce::Colour(0xffc13a3a), juce::Colour(0xffa02c2c), juce::Colour(0xff7a1d1d), juce::Colour(0xff5a1515), juce::Colour(0xff3d0e0e) },
            { juce::Colour(0xfffff0e0), juce::Colour(0xfff3d3b3), juce::Colour(0xfff0b87c), juce::Colour(0xffe99d4d), juce::Colour(0xffc17a3a), juce::Colour(0xffa05e2c), juce::Colour(0xff7a471d), juce::Colour(0xff5a3515), juce::Colour(0xff3d220e) },
            { juce::Colour(0xffe6ffe6), juce::Colour(0xffb3f7b3), juce::Colour(0xff7cf07c), juce::Colour(0xff4de94d), juce::Colour(0xff3ac13a), juce::Colour(0xff2ca02c), juce::Colour(0xff1d7a1d), juce::Colour(0xff155a15), juce::Colour(0xff0e3d0e) },
            { juce::Colour(0xffe0f7ff), juce::Colour(0xffb3e3f7), juce::Colour(0xff7cccf0), juce::Colour(0xff4db3e9), juce::Colour(0xff3a8ec1), juce::Colour(0xff2c6ea0), juce::Colour(0xff1d4a7a), juce::Colour(0xff15355a), juce::Colour(0xff0e223d) },
            { juce::Colour(0xffe6e6ff), juce::Colour(0xffb3b3f7), juce::Colour(0xff7c7cf0), juce::Colour(0xff4d4de9), juce::Colour(0xff3a3ac1), juce::Colour(0xff2c2ca0), juce::Colour(0xff1d1d7a), juce::Colour(0xff15155a), juce::Colour(0xff0e0e3d) }
        };
    }
    void drawPalette(juce::Graphics& g) const {
        // 팔레트 배경 (face 내부 bottom 오버레이, 검정색)
        g.setColour(juce::Colours::black.withAlpha(0.97f));
        g.fillRoundedRectangle(paletteX, paletteY, paletteW, paletteH, 8.0f);
        // 컬러 그리기 (9x7, 네모, 간격 없음)
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                int x = paletteX + col * cellSizeX;
                int y = paletteY + row * cellSizeY;
                g.setColour(colors[row][col]);
                g.fillRect(x, y, cellSizeX, cellSizeY);
            }
        }
    }
    juce::Point<int> position;
    bool isOpen;
    juce::Colour selectedColor;
    int paletteY;
    std::vector<std::vector<juce::Colour>> colors;
};

class Bottom {
public:
    Bottom(juce::Colour faceColor, juce::Colour textColor = juce::Colours::black) : faceColor(faceColor), textColor(textColor), bypassOn(false), preset(faceColor, textColor), arrowVisible(true) {}
    
    void setBypassState(bool on) {
        bypassOn = on;
    }
    
    void draw(juce::Graphics& g, juce::Drawable* arrowDrawableB = nullptr, juce::Drawable* arrowDrawableW = nullptr, const Face* face = nullptr) const {
        // bypass 상태에 따른 알파값 설정 (Panel과 동일하게)
        float alpha = bypassOn ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
        
        // 세퍼레이터 기준 아래로 23px 위치에서 위로 10px 이동, 그리고 위로 8px 더 이동, 그리고 위로 2px 더 이동 (세퍼레이터 Y=126, 높이 4px)
        int buttonY = 121 + 4 + 23 - 10 - 8 - 2; // 세퍼레이터 아래 끝 + 23px - 10px - 8px - 2px (5px 아래로 이동)
        
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        
        if (arrowVisible) {
            // 화살표 모드: 텍스트와 언더라인 숨기고 화살표 표시
            // in 화살표 - 좌측 정렬
            int inX = 8 + 2;
            int arrowSize = 15; // 20px에서 15px로 줄임 (75%)
            int arrowY = buttonY + 4; // 5px 아래로 이동에서 4px로 조정 (위로 1px)
            
            // 다크모드 여부에 따라 화살표 SVG 선택 (Face의 isDarkMode 기준)
            bool isDarkMode = face ? face->isDarkMode() : false;
            juce::Drawable* arrowDrawable = isDarkMode ? arrowDrawableW : arrowDrawableB;
            
            if (arrowDrawable) {
                // SVG 화살표 그리기 (투명도 적용)
                g.setOpacity(alpha);
                // in 화살표 그리기
                arrowDrawable->drawWithin(g, juce::Rectangle<float>(inX, arrowY, arrowSize, arrowSize), juce::RectanglePlacement::centred, alpha);
                // out 화살표 그리기 (우측 정렬)
                int outX = 152 - arrowSize - 1; // 오른쪽으로 1px 이동
                arrowDrawable->drawWithin(g, juce::Rectangle<float>(outX, arrowY, arrowSize, arrowSize), juce::RectanglePlacement::centred, alpha);
                g.setOpacity(1.0f);
            } else {
                // SVG 로드 실패 시 간단한 삼각형으로 대체
                juce::Colour arrowColor = isDarkMode ? juce::Colours::white : juce::Colours::black;
                g.setColour(arrowColor.withAlpha(alpha));
                // in 화살표 (간단한 삼각형)
                juce::Path arrowPath;
                arrowPath.addTriangle(inX, arrowY + 5, inX + 8, arrowY + 10, inX, arrowY + 15);
                g.fillPath(arrowPath);
                // out 화살표 (간단한 삼각형)
                int outX = 152 - arrowSize - 1; // 오른쪽으로 1px 이동
                juce::Path outArrowPath;
                outArrowPath.addTriangle(outX + 8, arrowY + 5, outX, arrowY + 10, outX + 8, arrowY + 15);
                g.fillPath(outArrowPath);
            }
        } else {
            // 텍스트 모드: 기존 로직 유지
            int inX = 8 + 2; // 세퍼레이터 왼쪽 끝 + 2px
            int inTextWidth = font.getStringWidth("in");
            g.setColour(textColor.withAlpha(alpha * 0.5f));
            g.setFont(font);
            g.drawText("in", inX, buttonY, inTextWidth, 20, juce::Justification::centredLeft);
            g.setColour(textColor.withAlpha(alpha));
            g.drawLine(inX, buttonY + 17, inX + inTextWidth, buttonY + 17, 1.0f);
            int outTextWidth = font.getStringWidth("out");
            int outX = 152 - outTextWidth - 2;
            g.setColour(textColor.withAlpha(alpha * 0.5f));
            g.drawText("out", outX, buttonY, outTextWidth, 20, juce::Justification::centredLeft);
            g.setColour(textColor.withAlpha(alpha));
            g.drawLine(outX, buttonY + 17, outX + outTextWidth, buttonY + 17, 1.0f);
        }
        // preset 클래스 사용하여 그리기 (bypass 상태 전달)
        preset.draw(g, buttonY, alpha);
        int bypassY = 265 - 4 - 20; // 5px 아래로 이동
        int bypassTextWidth = font.getStringWidth("bypass");
        int bypassX = 80 - bypassTextWidth / 2;
        float bypassAlpha = bypassOn ? 1.0f : 0.2f;
        g.setColour(textColor.withAlpha(bypassAlpha));
        g.setFont(font);
        g.drawText("bypass", bypassX, bypassY, bypassTextWidth, 20, juce::Justification::centred);
    }
    
    bool hitTestInButton(juce::Point<int> pos) const {
        int buttonY = 121 + 4 + 23 - 10 - 8 - 2; // 5px 아래로 이동
        int inX = 8 + 2;
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int inTextWidth = font.getStringWidth("in");
        return juce::Rectangle<int>(inX - 5, buttonY - 5, inTextWidth + 10, 20 + 10).contains(pos);
    }
    
    bool hitTestOutButton(juce::Point<int> pos) const {
        int buttonY = 121 + 4 + 23 - 10 - 8 - 2; // 5px 아래로 이동
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int outTextWidth = font.getStringWidth("out");
        int outX = 152 - outTextWidth - 2;
        return juce::Rectangle<int>(outX - 5, buttonY - 5, outTextWidth + 10, 20 + 10).contains(pos);
    }
    
    bool hitTestPresetButton(juce::Point<int> pos) const {
        int buttonY = 126 + 4 + 23 - 10 - 8 - 2;
        return preset.hitTestPresetButton(pos, buttonY);
    }
    
    bool hitTestPresetDropdownButton(juce::Point<int> pos) const {
        int buttonY = 121 + 4 + 23 - 10 - 8 - 2; // 5px 아래로 이동
        return preset.hitTestPresetDropdownButton(pos, buttonY);
    }
    
    bool hitTestBypassButton(juce::Point<int> pos) const {
        int bypassY = 265 - 4 - 20; // 5px 아래로 이동
        juce::Font font("Euclid Circular B", DEFAULT_FONT_SIZE, juce::Font::plain);
        int bypassTextWidth = font.getStringWidth("bypass");
        int bypassX = 80 - bypassTextWidth / 2;
        return juce::Rectangle<int>(bypassX, bypassY, bypassTextWidth, 20).contains(pos);
    }
    
    Preset& getPreset() { return preset; }
    
    void setFaceColor(juce::Colour color) {
        faceColor = color;
    }
    
    void setTextColor(juce::Colour color) {
        textColor = color;
        preset.setTextColor(color);
    }
    
    void setArrowVisible(bool visible) {
        arrowVisible = visible;
    }
    
    bool getArrowVisible() const {
        return arrowVisible;
    }
    
private:
    juce::Colour faceColor;
    juce::Colour textColor;
    bool bypassOn;
    Preset preset;
    bool arrowVisible;
};

class ClearHostApp : public juce::AudioAppComponent, public juce::AudioProcessorPlayer, public juce::AudioProcessorListener, public juce::Slider::Listener, public juce::ComboBox::Listener, public juce::Button::Listener, public juce::Timer {
public:
    ClearHostApp() {
        animationDuration = 1.0;
        animationTimerInterval = 16;
        originalSystemOutputDevice = "";
        
        // 장치 설정 파일 초기화
        deviceSettingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("ClearHost")
                            .getChildFile("device_settings.conf");
        
        static EuclidLookAndFeel euclidLF;
        juce::LookAndFeel::setDefaultLookAndFeel(&euclidLF);
        
        performAutoSetup();
        pluginManager.addDefaultFormats();
        
        // LED 초기화 (플러그인 로드 전에 먼저 생성)
        pluginStatusLED = std::make_unique<LED>(juce::Point<int>(0, 0), LEDState::OFF);
        
        // Face 초기화
        face = std::make_unique<Face>();
        
        // Panel 초기화 (LED 초기화 후에 생성)
        controlPanel = std::make_unique<Panel>(juce::Point<int>(0, 0), knobValues, knobRects, pluginStatusLED, face->getColor(), knobShowValues, face->getTextColor());
        
        // Bottom 초기화
        bottom = std::make_unique<Bottom>(face->getColor(), face->getTextColor());
        
        // ColorPicker 초기화 (오른쪽 아래 위치)
        colorPicker = std::make_unique<ColorPicker>();
        colorPicker->setPosition(juce::Point<int>(139, 253)); // 5px 아래로 이동
        
        loadClearVST3();
        setAudioChannels(2, 2);
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        if (!midiInputs.isEmpty()) {
            midiInput = juce::MidiInput::openDevice(midiInputs[0].identifier, this);
            if (midiInput) midiInput->start();
        }
        setProcessor(clearPlugin.get());
        // 플러그인 에디터 생성 및 표시
        if (clearPlugin) {
            pluginEditor.reset(clearPlugin->createEditor());
            if (pluginEditor) {
                pluginEditor->setOpaque(true); // 완전히 불투명하게
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
        setSize(160, 265); // 창 크기를 160x265로 설정 (5px 줄임)
        
        // 노브 컨트롤들 생성
        for (int i = 0; i < 3; ++i) {
            auto knob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            knob->setRange(0.0, 1.0, 0.01);
            knob->setValue(0.5);
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            knob->addListener(this);
            knobs.add(knob.release());
        }
        
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
        updatePresetList();
        
        // 저장된 장치 설정 로드
        // loadDeviceSettings();
        
        // 저장된 설정 적용 또는 기본 장치 설정
        // if (currentInputDevice.isEmpty() && !inputDeviceList.empty()) {
        //     currentInputDevice = inputDeviceList[0];
        // }
        // if (currentOutputDevice.isEmpty() && !outputDeviceList.empty()) {
        //     currentOutputDevice = outputDeviceList[0];
        // }
        
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
    buttonRects.resize(6); // 6개 버튼으로 확장
    // buttonLabels.resize(7); // 필요 없음
    stereoMonoRect = juce::Rectangle<int>();
    inputDeviceRect = juce::Rectangle<int>();
    outputDeviceRect = juce::Rectangle<int>();
    draggingKnob = -1;
    lastDragY = 0;
        
        // 드롭다운 관련 변수들
        inputDropdownOpen = false;
        outputDropdownOpen = false;
        presetDropdownOpen = false;
        inputDeviceList.clear();
        outputDeviceList.clear();
        presetList.clear();
        inputDropdownRect = juce::Rectangle<int>();
        outputDropdownRect = juce::Rectangle<int>();
        presetDropdownRect = juce::Rectangle<int>();
        inputDeviceRects.clear();
        outputDeviceRects.clear();
        presetRects.clear();
        
        // Preset 상태 초기화
        presetActive = false;
        activePresetName = "";
        currentPreset = "";
        
        // 초기 preset 표시 업데이트
        updatePresetDisplay();
        
        // 화살표 SVG 로드
        loadArrowSVGs();
        
        // 로고 SVG 로드
        loadLogoSVGs();
    }
    ~ClearHostApp() override {
        // 타이머 중지 (가장 먼저)
        stopTimer();
        
        // 플러그인 리스너 제거 (플러그인이 아직 유효할 때)
        if (clearPlugin) {
            juce::Logger::writeToLog("Removing parameter listener...");
            clearPlugin->removeListener(this);
        }
        
        // 플러그인 에디터 정리
        if (pluginEditor) {
            pluginEditor.reset();
        }
        
        // 플러그인 정리
        if (clearPlugin) {
            clearPlugin.reset();
        }
        
        // MIDI 입력 정리
        if (midiInput) {
            midiInput->stop();
            midiInput.reset();
        }
        
        // 벡터들 정리
        knobRects.clear();
        knobValues.clear();
        knobShowValues.clear();
        knobLastValues.clear();
        buttonRects.clear();
        buttonLabels.clear();
        inputDeviceList.clear();
        outputDeviceList.clear();
        presetList.clear();
        inputDeviceRects.clear();
        outputDeviceRects.clear();
        presetRects.clear();
        
        // SVG 드로어블들 정리
        arrowDrawableB.reset();
        arrowDrawableW.reset();
        logoDrawableB.reset();
        logoDrawableW.reset();
        
        // 컴포넌트들 정리
        controlPanel.reset();
        face.reset();
        bottom.reset();
        colorPicker.reset();
        pluginStatusLED.reset();
        
        // 오디오 정리 (마지막에)
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
        int col3 = w - col1; // 3번 캔버스 (나머지 공간)
        // 플러그인 관련 멤버 접근 전 nullptr/size 체크
        if (knobRects.size() < 3 || knobValues.size() < 3) {
            juce::Logger::writeToLog("paint: knobRects/knobValues size error");
            return;
        }
        // 1단: 검정색 배경
        juce::Rectangle<int> leftArea(0, 0, col1, h);
        g.setColour(juce::Colours::black);
        g.fillRect(leftArea);
        // Face 그리기 (좌상단 0,0에서 160x280)
        if (face) {
            face->draw(g);
        }
        
        // 로고 그리기 (Face 바로 위, Panel 아래)
        drawLogo(g, face->getTextColor());
        // Panel 그리기 (노브 3개 + LED + Stereo 토글) - 1번 캔버스로 이동
        juce::Point<int> panelCenter(80, 78); // 노브2(voice) 중앙을 x=80px, y=78px에 위치 (5px 아래로 이동)
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
            controlPanel->draw(g, currentMousePos);
        }
        
        // Bottom 그리기 (화살표 Drawable 전달)
        if (bottom) {
            bottom->draw(g, arrowDrawableB.get(), arrowDrawableW.get(), face.get());
        }
        
        // ColorPicker 그리기
        if (colorPicker && face) {
            colorPicker->draw(g, face->getTextColor());
        }
        
        // 입력 드롭다운이 열려있으면 장치 리스트 표시 (in 버튼 바로 아래)
        if (inputDropdownOpen) {
            // bypass 상태에 따른 알파값 설정 (Panel과 동일하게)
            float alpha = bypassActive ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
            
            int inButtonY = 121 + 4 + 23 - 10 - 8 - 2; // 5px 아래로 이동
            int dropdownY = inButtonY + 21;
            int dropdownHeight = std::min(MAX_VISIBLE_ITEMS * ITEM_HEIGHT, (int)inputDeviceList.size() * ITEM_HEIGHT);
            inputDropdownRect = juce::Rectangle<int>(10, dropdownY, 140, dropdownHeight);
            
            inputDeviceRects.clear();
            int visibleCount = std::min(MAX_VISIBLE_ITEMS, (int)inputDeviceList.size() - inputScrollOffset);
            
            for (int i = 0; i < visibleCount; ++i) {
                int actualIndex = i + inputScrollOffset;
                juce::Rectangle<int> itemRect(inputDropdownRect.getX(), inputDropdownRect.getY() + i * ITEM_HEIGHT, inputDropdownRect.getWidth(), ITEM_HEIGHT);
                inputDeviceRects.push_back(itemRect);
                
                // 현재 선택된 장치인지 확인
                bool isSelected = (inputDeviceList[actualIndex] == currentInputDevice);
                
                // bypass 상태에 따른 알파값 적용
                g.setColour(face->getTextColor().withAlpha(alpha));
                g.drawText(inputDeviceList[actualIndex].toLowerCase(), itemRect, juce::Justification::centredLeft);
                
                // 현재 선택된 장치에 언더라인 그리기
                if (isSelected) {
                    // 현재 설정된 폰트 사용 (텍스트 그리기와 동일한 폰트)
                    int textWidth = g.getCurrentFont().getStringWidth(inputDeviceList[actualIndex].toLowerCase());
                    int underlineY = itemRect.getY() + 15; // 텍스트 아래 3px (1px 아래로 이동)
                    int underlineX = itemRect.getX(); // 오른쪽으로 1px 이동 (-1px -> 0px)
                    g.setColour(face->getTextColor().withAlpha(alpha));
                    g.drawLine(underlineX, underlineY, underlineX + textWidth, underlineY, 1.0f);
                }
            }
        }
        
        // 출력 드롭다운이 열려있으면 장치 리스트 표시 (out 버튼 바로 아래)
        if (outputDropdownOpen) {
            // bypass 상태에 따른 알파값 설정 (Panel과 동일하게)
            float alpha = bypassActive ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
            
            // out 버튼 바로 아래 위치 계산 (Bottom 클래스의 out 버튼 위치 참조)
            int outButtonY = 121 + 4 + 23 - 10 - 8 - 2; // Bottom 클래스의 실제 out 버튼 Y 위치 (5px 아래로 이동)
            int dropdownY = outButtonY + 21; // out 버튼 바로 아래 20px + 2px - 1px (위로 1px 이동)
            
            // 드롭다운 높이를 최대 5개 아이템으로 제한
            int dropdownHeight = std::min(MAX_VISIBLE_ITEMS * ITEM_HEIGHT, (int)outputDeviceList.size() * ITEM_HEIGHT);
            outputDropdownRect = juce::Rectangle<int>(10, dropdownY, 140, dropdownHeight);
            
            outputDeviceRects.clear();
            int visibleCount = std::min(MAX_VISIBLE_ITEMS, (int)outputDeviceList.size() - outputScrollOffset);
            
            for (int i = 0; i < visibleCount; ++i) {
                int actualIndex = i + outputScrollOffset;
                juce::Rectangle<int> itemRect(outputDropdownRect.getX(), outputDropdownRect.getY() + i * ITEM_HEIGHT, outputDropdownRect.getWidth(), ITEM_HEIGHT);
                outputDeviceRects.push_back(itemRect);
                
                // 현재 선택된 장치인지 확인
                bool isSelected = (outputDeviceList[actualIndex] == currentOutputDevice);
                
                // bypass 상태에 따른 알파값 적용
                g.setColour(face->getTextColor().withAlpha(alpha));
                g.drawText(outputDeviceList[actualIndex].toLowerCase(), itemRect, juce::Justification::centredRight);
                
                // 현재 선택된 장치에 언더라인 그리기
                if (isSelected) {
                    // 현재 설정된 폰트 사용 (텍스트 그리기와 동일한 폰트)
                    int textWidth = g.getCurrentFont().getStringWidth(outputDeviceList[actualIndex].toLowerCase());
                    int underlineY = itemRect.getY() + 15; // 텍스트 아래 3px (1px 아래로 이동)
                    int underlineX = itemRect.getX() + itemRect.getWidth() - textWidth; // 왼쪽으로 1px 이동 (+1px -> 0px)
                    g.setColour(face->getTextColor().withAlpha(alpha));
                    g.drawLine(underlineX, underlineY, underlineX + textWidth, underlineY, 1.0f);
                }
            }
        }
        
        // preset 드롭다운이 열려있으면 프리셋 리스트 표시 (preset 버튼 바로 아래, 중앙 정렬)
        if (presetDropdownOpen) {
            // bypass 상태에 따른 알파값 설정 (Panel과 동일하게)
            float alpha = bypassActive ? 0.3f : DEFAULT_ALPHA; // bypass on일 때 30%, off일 때 기본 알파값
            
            int presetButtonY = 121 + 4 + 23 - 10 - 8 - 2; // 5px 아래로 이동
            int dropdownY = presetButtonY + 21;
            int dropdownHeight = std::min(MAX_VISIBLE_ITEMS * ITEM_HEIGHT, (int)presetList.size() * ITEM_HEIGHT);
            int dropdownWidth = 140;
            int dropdownX = 80 - dropdownWidth / 2; // 중앙 80px 기준으로 정렬
            presetDropdownRect = juce::Rectangle<int>(dropdownX, dropdownY, dropdownWidth, dropdownHeight);
            drawDropdownList(g, presetList, currentPreset, presetDropdownRect, presetScrollOffset, MAX_VISIBLE_ITEMS, ITEM_HEIGHT, presetRects, juce::Justification::centred, 0, alpha, face->getTextColor());
        }
        
        // 3번 캔버스: 배경만 그리기 (불필요한 텍스트, 회색 배경 등 완전 제거)
        juce::Rectangle<int> rightArea(col1, 0, getWidth() - col1, getHeight());
        g.setColour(juce::Colours::black);
        g.fillRect(rightArea);
        // 플러그인 에디터는 addAndMakeVisible로 바로 올림 (paint에서 텍스트 등 제거)
        if (pluginEditor) {
            // 플러그인 에디터의 실제 크기 가져오기
            auto editorBounds = pluginEditor->getBounds();
            int actualEditorWidth = editorBounds.getWidth();
            int actualEditorHeight = editorBounds.getHeight();
            
            // 고정 좌표 설정 (오른쪽 크롭 방지)
            pluginEditor->setBounds(160, 0, actualEditorWidth, actualEditorHeight);
        }
        
        // ColorPicker 팔레트 위치 동적 지정 (bottom 하단에 맞춤)
        if (colorPicker) {
            constexpr int bottomY = 230; // Face 내부 bottom 시작 y (예시)
            constexpr int bottomHeight = 40; // bottom 높이 (예시)
            constexpr int paletteH = ColorPicker::paletteH;
            int paletteY = bottomY + bottomHeight - paletteH;
            colorPicker->setPaletteY(paletteY);
            colorPicker->draw(g);
        }
    }
    void resized() override {
        if (pluginEditor) {
            // 플러그인 에디터의 실제 크기 가져오기
            auto editorBounds = pluginEditor->getBounds();
            int actualEditorWidth = editorBounds.getWidth();
            int actualEditorHeight = editorBounds.getHeight();
            
            // paint()와 동일하게 고정 좌표 사용 (오른쪽 크롭 방지)
            pluginEditor->setBounds(160, 0, actualEditorWidth, actualEditorHeight);
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
        
        if (button == clearVoiceButton.get()) {
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
        static bool firstRun = true;
        
        if (firstRun) {
            // 첫 실행 시 저장된 장치 설정 적용
            // applySavedDeviceSettings();
            firstRun = false;
        }
        
        // Rec 버튼 업데이트 (깜빡임 효과)
        if (controlPanel) {
            controlPanel->updateRecButton();
        }
        
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
                // Rec 버튼이 활성화되어 있으면 타이머 계속 실행, 아니면 정지
                if (controlPanel && controlPanel->isRecButtonActive()) {
                    startTimer(16); // 60fps로 깜빡임 업데이트
                } else {
                    stopTimer();
                    juce::Logger::writeToLog("Animation completed");
                    // 애니메이션 완료 후 0.5초 뒤에 레이블로 돌아가기
                    startTimer(500);
                }
            }
        } else {
            // 애니메이션이 아닌 경우: 노브 표시 상태를 레이블로 되돌리기
            resetKnobDisplayStates();
            repaint();
            
            // Rec 버튼이 활성화되어 있으면 타이머 계속 실행, 아니면 정지
            if (controlPanel && controlPanel->isRecButtonActive()) {
                startTimer(16); // 60fps로 깜빡임 업데이트
            } else {
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
        
        // Panel의 LED 클릭 처리 (테스트용)
        if (controlPanel && controlPanel->hitTestLED(pos)) {
            juce::Logger::writeToLog("LED clicked - Testing Rec button functionality");
            // LED 클릭 시 Rec 버튼도 토글 (테스트용)
            controlPanel->toggleRecButton();
            juce::Logger::writeToLog("Rec button toggled via LED click - State: " + juce::String(controlPanel->isRecButtonActive() ? "ON" : "OFF"));
            
            // Rec 버튼이 활성화되면 타이머 시작 (60fps)
            if (controlPanel->isRecButtonActive()) {
                startTimer(16);
            }
            
            repaint();
            return;
        }
        
        // Panel의 Rec 버튼 클릭 처리
        if (controlPanel && controlPanel->hitTestRecButton(pos)) {
            controlPanel->toggleRecButton();
            juce::Logger::writeToLog("Rec button clicked - State: " + juce::String(controlPanel->isRecButtonActive() ? "ON" : "OFF"));
            
            // Rec 버튼이 활성화되면 타이머 시작 (60fps)
            if (controlPanel->isRecButtonActive()) {
                startTimer(16);
            }
            
            repaint();
            return;
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
        
        // ColorPicker 클릭 처리
        if (colorPicker) {
            if (colorPicker->hitTest(pos)) {
                colorPicker->toggle();
                repaint();
                return;
            }
            if (colorPicker->isPaletteOpen() && colorPicker->hitTestPalette(pos)) {
                juce::Colour selectedColor = colorPicker->getColorAtPosition(pos);
                colorPicker->setSelectedColor(selectedColor);
                if (face) {
                    face->setColor(selectedColor);
                    if (controlPanel) controlPanel->setFaceColor(selectedColor);
                    if (bottom) bottom->setFaceColor(selectedColor);
                    if (controlPanel) controlPanel->setTextColor(face->getTextColor());
                    if (bottom) bottom->setTextColor(face->getTextColor());
                    repaint();
                }
                colorPicker->close();
                return;
            }
            // 팔레트가 열려있고, 팔레트 영역이 아닌 곳 클릭 시 닫기
            if (colorPicker->isPaletteOpen() && !colorPicker->hitTestPalette(pos) && !colorPicker->hitTest(pos)) {
                colorPicker->close();
                repaint();
                return;
            }
        }
        
        // Bottom 버튼들 클릭 처리 (in/out 버튼은 호버로 처리하므로 제거)
        if (bottom) {
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
        // 프리셋 버튼 5개 - TextButtonLike hit test 사용
        juce::String btnNames[6] = {"S* up**","Too Loud","sommers","Clear Voice","Dry Voice","Vocal Reference"};
        for (int i = 0; i < 6; ++i) {
            TextButtonLike tempBtn(btnNames[i], juce::Point<int>(buttonRects[i].getX(), buttonRects[i].getY()), DEFAULT_ALPHA);
            if (tempBtn.hitTest(pos)) {
                // 기존 preset 기능 매핑
                switch (i) {
                    case 0: startAnimation({0.5, 0.0, 0.0}); break; // S* up**
                    case 1: startAnimation({0.5, 0.2, 0.2}); break; // Too Loud
                    case 2: startAnimation({0.5, 1.0, 0.0}); break; // sommers
                    case 3: startAnimation({0.0, 0.5, 0.5}); break; // Clear Voice
                    case 4: startAnimation({0.0, 0.5, 0.0}); break; // Dry Voice
                    case 5: startAnimation({0.5, 0.1, 0.1}); break; // Vocal Reference
                }
                repaint();
                return;
            }
        }

        
        // 입력 드롭다운 아이템 클릭
        if (inputDropdownOpen) {
            for (int i = 0; i < inputDeviceRects.size(); ++i) {
                if (inputDeviceRects[i].contains(pos)) {
                    int actualIndex = i + inputScrollOffset;
                    if (actualIndex < inputDeviceList.size()) {
                        juce::String selectedDevice = inputDeviceList[actualIndex];
                    changeAudioInputDevice(selectedDevice);
                    inputDropdownOpen = false;
                    repaint();
                    return;
                    }
                }
            }
        }
        
        // 출력 드롭다운 아이템 클릭
        if (outputDropdownOpen) {
            for (int i = 0; i < outputDeviceRects.size(); ++i) {
                if (outputDeviceRects[i].contains(pos)) {
                    int actualIndex = i + outputScrollOffset;
                    if (actualIndex < outputDeviceList.size()) {
                        juce::String selectedDevice = outputDeviceList[actualIndex];
                    changeAudioOutputDevice(selectedDevice);
                    outputDropdownOpen = false;
                    repaint();
                    return;
                    }
                }
            }
        }
        
        // preset 드롭다운이 열려있으면 프리셋 리스트 표시 (preset 버튼 바로 아래, 중앙 정렬)
        if (presetDropdownOpen) {
            for (int i = 0; i < presetRects.size(); ++i) {
                if (presetRects[i].contains(pos)) {
                    int actualIndex = i + presetScrollOffset;
                    if (actualIndex < presetList.size()) {
                        juce::String selectedPreset = presetList[actualIndex];
                        currentPreset = selectedPreset;
                        setPresetActive(true, selectedPreset);
                        if (selectedPreset == "s* up**") {
                            startAnimation({0.5, 0.0, 0.0});
                            // stereo 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo
                                }
                            }
                        } else if (selectedPreset == "too loud") {
                            startAnimation({0.5, 0.2, 0.2});
                            // stereo 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo
                                }
                            }
                        } else if (selectedPreset == "sommers") {
                            startAnimation({0.5, 1.0, 0.0}); // amb 0.5, vox 1.0, v.rev 0
                            // mono 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(0.0f); // mono
                                }
                            }
                        } else if (selectedPreset == "clear voice") {
                            startAnimation({0.0, 0.5, 0.5});
                            // stereo 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo
                                }
                            }
                        } else if (selectedPreset == "dry voice") {
                            startAnimation({0.0, 0.5, 0.0});
                            // stereo 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo
                                }
                            }
                        } else if (selectedPreset == "cono") {
                            startAnimation({0.5, 0.1, 0.1});
                            // stereo 설정
                            if (clearPlugin) {
                                auto params = clearPlugin->getParameters();
                                if (params.size() > 13 && params[13]) {
                                    params[13]->setValueNotifyingHost(1.0f); // stereo
                                }
                            }
                        }
                        presetDropdownOpen = false;
                        repaint();
                        return;
                    }
                }
            }
        }
        
        // 드롭다운 외부 클릭 시 드롭다운 닫기
        if (inputDropdownOpen || outputDropdownOpen || presetDropdownOpen) {
            inputDropdownOpen = false;
            outputDropdownOpen = false;
            presetDropdownOpen = false;
            repaint();
            return;
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override {
        // 마우스 위치 업데이트 (그림자 효과를 위해)
        auto pos = event.getPosition();
        currentMousePos = pos;
        
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
            
            // preset이 활성화된 상태에서 노브를 수동으로 조절하면 preset 상태 리셋
            if (presetActive) {
                resetPresetToDefault();
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
    
    void mouseMove(const juce::MouseEvent& event) override {
        // 창 내부의 마우스 위치
        auto pos = event.getPosition();
        
        // 창 밖의 마우스 위치도 추적 (그림자 효과를 위해)
        auto globalPos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
        auto localPos = getLocalPoint(nullptr, globalPos);
        
        // 창 내부에 있을 때는 정확한 위치, 창 밖에 있을 때는 상대적 위치 사용
        if (getLocalBounds().contains(pos)) {
            currentMousePos = pos;
        } else {
            currentMousePos = juce::Point<int>(localPos.x, localPos.y);
        }
        
        // 그림자 효과를 위해 repaint 호출
        repaint();
        
        // in 버튼에 호버
        if (bottom && bottom->hitTestInButton(pos)) {
            if (!inputDropdownOpen) {
                juce::Logger::writeToLog("Mouse hover on in button - opening input dropdown");
                inputDropdownOpen = true;
                outputDropdownOpen = false; // 다른 드롭다운 닫기
                inputScrollOffset = 0; // 드롭다운이 열릴 때 스크롤 오프셋 리셋
                if (inputDeviceList.empty()) {
                    updateInputDeviceList();
                }
                repaint();
            }
            return;
        }
        
        // out 버튼에 호버
        if (bottom && bottom->hitTestOutButton(pos)) {
            if (!outputDropdownOpen) {
                juce::Logger::writeToLog("Mouse hover on out button - opening output dropdown");
                outputDropdownOpen = true;
                inputDropdownOpen = false; // 다른 드롭다운 닫기
                presetDropdownOpen = false; // 다른 드롭다운 닫기
                outputScrollOffset = 0; // 드롭다운이 열릴 때 스크롤 오프셋 리셋
                if (outputDeviceList.empty()) {
                    updateOutputDeviceList();
                }
                repaint();
            }
            return;
        }
        
        // preset 버튼에 호버
        if (bottom && bottom->hitTestPresetButton(pos)) {
            if (!presetDropdownOpen) {
                juce::Logger::writeToLog("Mouse hover on preset button - opening preset dropdown");
                presetDropdownOpen = true;
                inputDropdownOpen = false; // 다른 드롭다운 닫기
                outputDropdownOpen = false; // 다른 드롭다운 닫기
                presetScrollOffset = 0; // 드롭다운이 열릴 때 스크롤 오프셋 리셋
                if (presetList.empty()) {
                    updatePresetList();
                }
                repaint();
            }
            return;
        }
        
        // 드롭다운이 열려있을 때 마우스 위치 확인
        if (inputDropdownOpen) {
            // in 버튼이나 드롭다운 영역에 있지 않으면 드롭다운 닫기
            if (!bottom->hitTestInButton(pos) && !inputDropdownRect.contains(pos)) {
                juce::Logger::writeToLog("Mouse left input area - closing input dropdown");
                inputDropdownOpen = false;
                repaint();
            }
        }
        
        if (outputDropdownOpen) {
            // out 버튼이나 드롭다운 영역에 있지 않으면 드롭다운 닫기
            if (!bottom->hitTestOutButton(pos) && !outputDropdownRect.contains(pos)) {
                juce::Logger::writeToLog("Mouse left output area - closing output dropdown");
                outputDropdownOpen = false;
                repaint();
            }
        }
        
        if (presetDropdownOpen) {
            // preset 버튼이나 드롭다운 영역에 있지 않으면 드롭다운 닫기
            if (!bottom->hitTestPresetButton(pos) && !presetDropdownRect.contains(pos)) {
                juce::Logger::writeToLog("Mouse left preset area - closing preset dropdown");
                presetDropdownOpen = false;
                repaint();
            }
        }
    }
    
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override {
        auto pos = event.getPosition();
        
        // 노브 스크롤 처리 (드롭다운보다 우선순위 높음)
        if (controlPanel) {
            int knobIndex = controlPanel->hitTestKnob(pos);
            if (knobIndex >= 0 && knobIndex < knobValues.size()) {
                // 스크롤 방향에 따른 값 변경
                // wheel.deltaY < 0: 위로 스크롤 (값 증가) - 트랙패드 방향
                // wheel.deltaY > 0: 아래로 스크롤 (값 감소) - 트랙패드 방향
                float delta = (wheel.deltaY < 0) ? 0.01f : -0.01f; // 스크롤당 1% 변화 (원래 민감도)
                float newValue = juce::jlimit(0.0f, 2.0f, knobValues[knobIndex] + delta); // 0~2 범위로 확장
                
                // 노브 값 업데이트
                knobValues[knobIndex] = newValue;
                
                // preset이 활성화된 상태에서 노브를 수동으로 조절하면 preset 상태 리셋
                if (presetActive) {
                    resetPresetToDefault();
                }
                
                // 플러그인 파라미터 업데이트
                if (clearPlugin && pluginLoaded) {
                    // 노브 인덱스에 따른 파라미터 매핑
                    int parameterIndex = -1;
                    switch (knobIndex) {
                        case 0: parameterIndex = 1; break;  // Ambience Gain
                        case 1: parameterIndex = 14; break; // Voice Gain
                        case 2: parameterIndex = 12; break; // Voice Reverb Gain
                    }
                    
                    if (parameterIndex >= 0) {
                        // 노브 값(0~2)을 플러그인 파라미터 값(0~1)으로 정규화
                        float normalizedValue = juce::jlimit(0.0f, 1.0f, newValue / 2.0f);
                        clearPlugin->setParameter(parameterIndex, normalizedValue);
                        juce::Logger::writeToLog("Knob " + juce::String(knobIndex) + " -> Parameter " + juce::String(parameterIndex) + " = " + juce::String(normalizedValue) + " (from " + juce::String(newValue) + ")");
                    }
                }
                
                // 노브 값 표시 상태 업데이트
                updateKnobDisplayState(knobIndex, newValue);
                
                repaint();
                return; // 노브 스크롤이 처리되었으면 다른 스크롤은 무시
            }
        }
        
        // 입력 드롭다운 스크롤
        if (inputDropdownOpen && inputDropdownRect.contains(pos)) {
            if (inputDeviceList.size() > MAX_VISIBLE_ITEMS) {
                int delta = (wheel.deltaY > 0) ? -1 : 1;
                int newOffset = inputScrollOffset + delta;
                int maxOffset = (int)inputDeviceList.size() - MAX_VISIBLE_ITEMS;
                inputScrollOffset = juce::jlimit(0, maxOffset, newOffset);
                repaint();
            }
        }
        
        // 출력 드롭다운 스크롤
        if (outputDropdownOpen && outputDropdownRect.contains(pos)) {
            if (outputDeviceList.size() > MAX_VISIBLE_ITEMS) {
                int delta = (wheel.deltaY > 0) ? -1 : 1;
                int newOffset = outputScrollOffset + delta;
                int maxOffset = (int)outputDeviceList.size() - MAX_VISIBLE_ITEMS;
                outputScrollOffset = juce::jlimit(0, maxOffset, newOffset);
                repaint();
            }
        }
        
        // preset 드롭다운 스크롤
        if (presetDropdownOpen && presetDropdownRect.contains(pos)) {
            if (presetList.size() > MAX_VISIBLE_ITEMS) {
                int delta = (wheel.deltaY > 0) ? -1 : 1;
                int newOffset = presetScrollOffset + delta;
                int maxOffset = (int)presetList.size() - MAX_VISIBLE_ITEMS;
                presetScrollOffset = juce::jlimit(0, maxOffset, newOffset);
                repaint();
            }
        }
    }

    void updateOutputDeviceList() {
        outputDeviceList.clear();
        
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        if (deviceType) {
            try {
                auto outputNames = deviceType->getDeviceNames(false); // false = output devices
                for (int i = 0; i < outputNames.size(); ++i) {
                    // BlackHole 2ch는 숨김 처리
                    if (!outputNames[i].contains("BlackHole 2ch")) {
                        outputDeviceList.push_back(outputNames[i]); // 원본 이름 저장
                }
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
                    outputDeviceList.push_back("외장 헤드폰 (Manual)");
                }
                
            } catch (...) {
                juce::Logger::writeToLog("Error getting output device names");
            }
        }
        
        // 기본 출력 장치를 첫 번째로 설정
        if (!outputDeviceList.empty()) {
            currentOutputDevice = outputDeviceList[0];
        }
    }
    
    void updateInputDeviceList() {
        inputDeviceList.clear();
        
        bool blackHoleFound = false;
        
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        if (deviceType) {
            try {
                auto inputNames = deviceType->getDeviceNames(true); // true = input devices
                for (int i = 0; i < inputNames.size(); ++i) {
                    juce::String deviceName = inputNames[i];
                    if (deviceName.contains("BlackHole") || deviceName.contains("blackhole")) {
                        inputDeviceList.push_back("System Sound / BlackHole");
                        blackHoleFound = true;
                    } else {
                        inputDeviceList.push_back(deviceName); // 원본 이름 저장
                    }
                }
            } catch (...) {
                juce::Logger::writeToLog("Error getting input device names");
            }
        }
        
        // BlackHole이 없으면 비활성화된 옵션 추가
        if (!blackHoleFound) {
            inputDeviceList.push_back("System Sound / BlackHole - Not Installed");
        }
        
        // 기본 입력 장치를 첫 번째로 설정
        if (!inputDeviceList.empty()) {
            currentInputDevice = inputDeviceList[0];
        }
    }
    
    void updatePresetList() {
        presetList.clear();
        presetList.push_back("s* up**");
        presetList.push_back("too loud");
        presetList.push_back("sommers");
        presetList.push_back("clear voice");
        presetList.push_back("dry voice");
        presetList.push_back("cono");
        
        // 기본 프리셋을 첫 번째로 설정
        if (!presetList.empty()) {
            currentPreset = presetList[0];
        }
    }
    
    void saveDeviceSettings() {
        if (!deviceSettingsFile.existsAsFile()) {
            deviceSettingsFile.create();
        }
        
        juce::PropertiesFile settings(deviceSettingsFile, juce::PropertiesFile::Options());
        settings.setValue("lastInputDevice", currentInputDevice);
        settings.setValue("lastOutputDevice", currentOutputDevice);
        settings.save();
        
        juce::Logger::writeToLog("Device settings saved - Input: " + currentInputDevice + ", Output: " + currentOutputDevice);
    }
    
    void loadDeviceSettings() {
        if (deviceSettingsFile.existsAsFile()) {
            juce::PropertiesFile settings(deviceSettingsFile, juce::PropertiesFile::Options());
            juce::String savedInputDevice = settings.getValue("lastInputDevice", "");
            juce::String savedOutputDevice = settings.getValue("lastOutputDevice", "");
            
            if (savedInputDevice.isNotEmpty()) {
                currentInputDevice = savedInputDevice;
                juce::Logger::writeToLog("Loaded saved input device: " + currentInputDevice);
            }
            
            if (savedOutputDevice.isNotEmpty()) {
                currentOutputDevice = savedOutputDevice;
                juce::Logger::writeToLog("Loaded saved output device: " + currentOutputDevice);
            }
        }
    }
    
    void applySavedDeviceSettings() {
        // 현재 시스템의 실제 오디오 장치를 사용하도록 수정
        // 저장된 값 대신 현재 시스템의 실제 장치를 유지
        
        // 장치 리스트 업데이트
        updateInputDeviceList();
        updateOutputDeviceList();
        
        // 현재 시스템 장치를 현재 선택된 장치로 설정
        if (!inputDeviceList.empty()) {
            currentInputDevice = inputDeviceList[0]; // 첫 번째 장치 (시스템 기본)
            juce::Logger::writeToLog("Set current input device to system default: " + currentInputDevice);
        }
        
        if (!outputDeviceList.empty()) {
            currentOutputDevice = outputDeviceList[0]; // 첫 번째 장치 (시스템 기본)
            juce::Logger::writeToLog("Set current output device to system default: " + currentOutputDevice);
        }
        
        // ComboBox도 업데이트
        updateAudioDeviceLists();
    }

private:
    juce::AudioPluginFormatManager pluginManager;
    std::unique_ptr<juce::AudioPluginInstance> clearPlugin;
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::AudioProcessorEditor> pluginEditor;

    juce::OwnedArray<juce::Slider> knobs;
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
    
    // 장치 설정 저장용 파일 경로
    juce::File deviceSettingsFile;
    
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
    juce::Point<int> currentMousePos = juce::Point<int>(-1, -1);
    
    // 드롭다운 관련 변수들
    bool inputDropdownOpen = false;
    bool outputDropdownOpen = false;
    bool presetDropdownOpen = false;
    std::vector<juce::String> inputDeviceList;
    std::vector<juce::String> outputDeviceList;
    std::vector<juce::String> presetList;
    juce::Rectangle<int> inputDropdownRect;
    juce::Rectangle<int> outputDropdownRect;
    juce::Rectangle<int> presetDropdownRect;
    std::vector<juce::Rectangle<int>> inputDeviceRects;
    std::vector<juce::Rectangle<int>> outputDeviceRects;
    std::vector<juce::Rectangle<int>> presetRects;
    
    // 스크롤 관련 변수들
    int inputScrollOffset = 0;
    int outputScrollOffset = 0;
    int presetScrollOffset = 0;
    const int MAX_VISIBLE_ITEMS = 6;
    const int ITEM_HEIGHT = 16;
    
    // 현재 선택된 장치 추적
    juce::String currentInputDevice;
    juce::String currentOutputDevice;
    juce::String currentPreset;
    
    // Preset 상태 관리
    bool presetActive = false; // preset이 활성화되었는지 여부
    juce::String activePresetName = ""; // 현재 활성화된 preset 이름
    
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
    
    // ColorPicker 추가
    std::unique_ptr<ColorPicker> colorPicker;
    
    // ArrowVisible 변수 및 화살표 SVG 캐시
    bool arrowVisible = true; // 기본값은 true
    std::unique_ptr<juce::Drawable> arrowDrawableB;
    std::unique_ptr<juce::Drawable> arrowDrawableW;
    
    // 로고 SVG 캐싱을 위한 멤버 변수들
    std::unique_ptr<juce::Drawable> logoDrawableB;
    std::unique_ptr<juce::Drawable> logoDrawableW;
    bool logoSVGsLoaded = false;
    
    void loadArrowSVGs() {
        // 화살표 SVG 파일들 로드 - 실행 파일 위치 기준으로 Resources 디렉토리 찾기
        juce::File execFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        juce::File resourcesDir = execFile.getParentDirectory().getParentDirectory().getChildFile("Resources");
        juce::File arrowBFile = resourcesDir.getChildFile("arrow_b.svg");
        juce::File arrowWFile = resourcesDir.getChildFile("arrow_w.svg");
        
        if (arrowBFile.existsAsFile()) {
            arrowDrawableB = juce::Drawable::createFromSVGFile(arrowBFile);
            juce::Logger::writeToLog("Arrow SVG loaded successfully from: " + arrowBFile.getFullPathName());
        } else {
            juce::Logger::writeToLog("Arrow SVG file not found: " + arrowBFile.getFullPathName());
        }
        
        if (arrowWFile.existsAsFile()) {
            arrowDrawableW = juce::Drawable::createFromSVGFile(arrowWFile);
            juce::Logger::writeToLog("Arrow SVG loaded successfully from: " + arrowWFile.getFullPathName());
        } else {
            juce::Logger::writeToLog("Arrow SVG file not found: " + arrowWFile.getFullPathName());
        }
    }
    
    void loadLogoSVGs() {
        if (logoSVGsLoaded) return;
        
        // 로고 SVG 파일들 로드 - 실행 파일 위치 기준으로 Resources 디렉토리 찾기
        juce::File execFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        juce::File resourcesDir = execFile.getParentDirectory().getParentDirectory().getChildFile("Resources");
        juce::File logoBFile = resourcesDir.getChildFile("symbol_b.svg");
        juce::File logoWFile = resourcesDir.getChildFile("symbol_w.svg");
        
        if (logoBFile.existsAsFile()) {
            logoDrawableB = juce::Drawable::createFromSVGFile(logoBFile);
            if (logoDrawableB) {
                juce::Logger::writeToLog("SVG logo loaded successfully from: " + logoBFile.getFullPathName());
            }
        } else {
            juce::Logger::writeToLog("Logo SVG file not found: " + logoBFile.getFullPathName());
        }
        
        if (logoWFile.existsAsFile()) {
            logoDrawableW = juce::Drawable::createFromSVGFile(logoWFile);
            if (logoDrawableW) {
                juce::Logger::writeToLog("SVG logo loaded successfully from: " + logoWFile.getFullPathName());
            }
        } else {
            juce::Logger::writeToLog("Logo SVG file not found: " + logoWFile.getFullPathName());
        }
        
        logoSVGsLoaded = true;
    }
    
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
    
    // Preset 상태 관리 메서드들
    void setPresetActive(bool active, const juce::String& presetName = "") {
        presetActive = active;
        activePresetName = presetName;
        updatePresetDisplay();
    }
    
    void setArrowVisible(bool visible) {
        arrowVisible = visible;
        if (bottom) {
            bottom->setArrowVisible(visible);
        }
    }
    
    bool getArrowVisible() const {
        return arrowVisible;
    }
    
    void updatePresetDisplay() {
        if (bottom) {
            Preset& preset = bottom->getPreset();
            if (presetActive && activePresetName.isNotEmpty()) {
                // preset이 활성화된 경우: 레이블을 preset 이름으로 변경, LED ON
                preset.setLabelText(activePresetName);
                preset.setLedOn(true);
            } else {
                // preset이 비활성화된 경우: 레이블을 "preset"으로 변경, LED OFF
                preset.setLabelText("preset");
                preset.setLedOn(false);
            }
            repaint();
        }
    }
    
    void resetPresetToDefault() {
        setPresetActive(false);
    }
    
    void drawLogo(juce::Graphics& g, juce::Colour textColor = juce::Colours::black) {
        // 라이트/다크모드에 따라 SVG 파일 선택
        bool isDarkMode = textColor == juce::Colours::white;
        juce::Drawable* logoDrawable = isDarkMode ? logoDrawableW.get() : logoDrawableB.get();
        
        // bottom 영역의 정중앙 계산
        // 세퍼레이터 아래쪽: Y = 121 + 4 = 125 (5px 아래로 이동)
        // Face 아래쪽: Y = 265 (5px 아래로 이동)
        // Bottom 영역 중앙: Y = (125 + 265) / 2 = 195 (5px 아래로 이동)
        int bottomAreaCenterY = 200; // 5px 아래로 이동
        
        // 박스 크기: 17px x 17px (원본)
        int boxSize = 17;
        
        // 텍스트 크기 계산 (30pt - 2pt = 28pt)
        g.setFont(juce::Font("Euclid Circular B", 28.0f, juce::Font::plain));
        int textHeight = (int)g.getCurrentFont().getHeight();
        int textWidth = g.getCurrentFont().getStringWidth("sup clr");
        
        // 박스와 텍스트 간격: 8px (9px에서 1px 줄임)
        int spacing = 8;
        
        // 로고셋 전체 너비 계산
        int logoSetWidth = boxSize + spacing + textWidth;
        
        // 로고셋 전체를 80px 중앙에 정렬
        int logoSetX = 80 - logoSetWidth / 2;
        
        // 박스 위치 (아래로 24px 이동, 전체 위로 30px 이동)
        int boxX = logoSetX;
        int boxY = bottomAreaCenterY - boxSize/2 + 24 - 30; // bottom 영역 중앙 + 24px 아래로 - 30px 위로
        
        // 모드에 따른 SVG 로고 그리기 (17px x 17px, 10% 투명도)
        if (logoDrawable) {
            g.setColour(juce::Colours::white.withAlpha(0.1f)); // SVG 자체 색상 사용, 투명도만 적용
            logoDrawable->drawWithin(g, juce::Rectangle<float>(boxX, boxY, boxSize, boxSize), juce::RectanglePlacement::centred, 1.0f);
        } else {
            // SVG 로드 실패 시 기존 박스 그리기
            g.setColour(textColor.withAlpha(0.1f));
            g.fillRect(boxX, boxY, boxSize, boxSize);
        }
        
        // 'sup clr' 텍스트 그리기 (28pt, textColor 10% 투명도, 텍스트만 위로 2px 추가 이동)
        g.setColour(textColor.withAlpha(0.1f));
        g.setFont(juce::Font("Euclid Circular B", 28.0f, juce::Font::plain));
        int textX = boxX + boxSize + spacing; // 박스 오른쪽 + 8px 여백
        int textY = boxY + boxSize/2 - textHeight/2 - 2; // 박스 세로 중앙에 텍스트 세로 중앙 정렬 - 2px 위로
        g.drawText("sup clr", textX, textY, textWidth, textHeight, juce::Justification::centredLeft);
    }
    
    void updateAudioDeviceLists() {
        // 현재 활성화된 오디오 장치 타입 사용
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        
        // 입력 장치 목록 업데이트
        inputDeviceBox->clear();
        
        bool blackHoleFound = false;
        
        if (deviceType) {
            try {
                auto inputNames = deviceType->getDeviceNames(true); // true = input devices
                juce::Logger::writeToLog("Found " + juce::String(inputNames.size()) + " input devices");
                
                for (int i = 0; i < inputNames.size(); ++i) {
                    juce::String deviceName = inputNames[i];
                    juce::Logger::writeToLog("Input device " + juce::String(i) + ": " + deviceName);
                    
                    // BlackHole을 "System Sound / BlackHole"로 표시
                    if (deviceName.contains("BlackHole") || deviceName.contains("blackhole")) {
                        inputDeviceBox->addItem("System Sound / BlackHole", i + 1);
                        juce::Logger::writeToLog("Added System Sound / BlackHole option");
                        blackHoleFound = true;
                    } else {
                        inputDeviceBox->addItem(deviceName, i + 1);
                    }
                }
        } catch (...) {
                juce::Logger::writeToLog("Error getting input device names");
            }
        }
        
        // BlackHole이 없으면 비활성화된 옵션 추가
        if (!blackHoleFound) {
            int disabledItemId = 999; // 고유한 ID
            inputDeviceBox->addItem("System Sound / BlackHole - Not Installed", disabledItemId);
            inputDeviceBox->setItemEnabled(disabledItemId, false);
            juce::Logger::writeToLog("BlackHole not found - added disabled option");
        }
        
        // 출력 장치 목록 업데이트
        outputDeviceBox->clear();
        
        if (deviceType) {
            try {
                auto outputNames = deviceType->getDeviceNames(false); // false = output devices
                juce::Logger::writeToLog("Found " + juce::String(outputNames.size()) + " output devices");
                
                for (int i = 0; i < outputNames.size(); ++i) {
                    juce::String deviceName = outputNames[i];
                    juce::Logger::writeToLog("Output device " + juce::String(i) + ": " + deviceName);
                    
                    // BlackHole 2ch는 숨김 처리
                    if (!deviceName.contains("BlackHole 2ch")) {
                        outputDeviceBox->addItem(deviceName, i + 1);
                    }
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
        
        // 현재 선택된 장치 설정 (저장된 장치 또는 첫 번째 장치)
        if (inputDeviceBox->getNumItems() > 0) {
            bool inputDeviceFound = false;
            for (int i = 1; i <= inputDeviceBox->getNumItems(); ++i) {
                if (inputDeviceBox->getItemText(i) == currentInputDevice) {
                    inputDeviceBox->setSelectedId(i, juce::dontSendNotification);
                    inputDeviceFound = true;
                    juce::Logger::writeToLog("Selected saved input device in ComboBox: " + currentInputDevice);
                    break;
                }
            }
            if (!inputDeviceFound) {
        inputDeviceBox->setSelectedId(1, juce::dontSendNotification);
                juce::Logger::writeToLog("Saved input device not found in ComboBox, using first item");
            }
        }
        
        if (outputDeviceBox->getNumItems() > 0) {
            bool outputDeviceFound = false;
            for (int i = 1; i <= outputDeviceBox->getNumItems(); ++i) {
                if (outputDeviceBox->getItemText(i) == currentOutputDevice) {
                    outputDeviceBox->setSelectedId(i, juce::dontSendNotification);
                    outputDeviceFound = true;
                    juce::Logger::writeToLog("Selected saved output device in ComboBox: " + currentOutputDevice);
                    break;
                }
            }
            if (!outputDeviceFound) {
        outputDeviceBox->setSelectedId(1, juce::dontSendNotification);
                juce::Logger::writeToLog("Saved output device not found in ComboBox, using first item");
            }
        }
    }
    
    void changeAudioInputDevice(const juce::String& deviceName) {
        juce::Logger::writeToLog("Changing input device to: " + deviceName);
        currentInputDevice = deviceName; // 현재 선택된 장치 업데이트
        
        // 장치 설정 저장
        saveDeviceSettings();
        
        if (deviceName == "System Sound / BlackHole" || deviceName == "System Sound / BlackHole - Not Installed") {
            // 비활성화된 BlackHole 옵션 선택 시 처리
            if (deviceName.contains("Not Installed")) {
                juce::Logger::writeToLog("BlackHole not installed - showing info");
                // 여기에 나중에 설치 안내 다이얼로그를 추가할 수 있습니다
                // 현재는 기본 입력으로 되돌리기
                return;
            }
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
                                juce::Logger::writeToLog("Successfully connected to System Sound / BlackHole");
                                
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
                
                // BlackHole이 아닌 다른 장치로 변경 시 시스템 출력 복원
                restoreSystemOutputDevice();
                
                // 오디오 재시작
                deviceManager.restartLastAudioDevice();
            }
        }
    }
    
    void changeAudioOutputDevice(const juce::String& deviceName) {
        juce::Logger::writeToLog("Changing output device to: " + deviceName);
        currentOutputDevice = deviceName; // 현재 선택된 장치 업데이트
        
        // 장치 설정 저장
        saveDeviceSettings();
        
        if (deviceName == "외장 헤드폰 (Manual)") {
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
        // 고정 창 크기: 160 x 265 픽셀
        centreWithSize(160, 265);
        
        // 항상 위에 표시되도록 설정 (다른 앱들과 함께 사용할 때 편리함)
        setAlwaysOnTop(true);
        
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
