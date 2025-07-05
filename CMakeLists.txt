cmake_minimum_required(VERSION 3.15)
project(ClearHost VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# JUCE 플러그인 호스트 매크로 정의 (AudioUnit과 VST3만)
add_definitions(-DJUCE_PLUGINHOST_AU=1)
add_definitions(-DJUCE_PLUGINHOST_VST3=1)

find_package(JUCE CONFIG REQUIRED)

juce_add_console_app(ClearHost
    PRODUCT_NAME "ClearHost"
    VERSION 0.1.0
)

juce_generate_juce_header(ClearHost)

# 소스 파일 추가
target_sources(ClearHost PRIVATE
    src/main.cpp
)

# JUCE 모듈 추가
target_link_libraries(ClearHost PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_audio_processors
)

# Resources 디렉토리 복사 (폰트 파일 포함)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
    add_custom_command(TARGET ClearHost POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
        "$<TARGET_FILE_DIR:ClearHost>/Resources"
    )
endif()
