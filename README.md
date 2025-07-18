# clr - 오디오 플러그인 호스트

## 개요
clr는 macOS용 오디오 플러그인 호스트입니다. BlackHole 2ch와 switchaudio-osx를 활용하여 시스템 오디오를 캡처하고 처리할 수 있습니다.

## 시스템 요구사항
- macOS 10.14 (Mojave) 이상
- 최소 500MB 여유 공간
- 인터넷 연결 (첫 설치 시)

## 설치 방법

### 1. DMG 파일 다운로드
- `clr_Installer.dmg` 파일을 다운로드합니다.

### 2. 설치 실행
1. DMG 파일을 더블클릭하여 마운트합니다.
2. `clr.app`을 Applications 폴더로 드래그합니다.
3. Applications 폴더에서 `clr`를 실행합니다.

### 3. 자동 설치
- clr를 처음 실행하면 자동으로 설치 마법사가 시작됩니다.
- "다음" 버튼만 누르면 모든 필수 구성 요소가 자동으로 설치됩니다:
  - Command Line Tools
  - Homebrew
  - BlackHole 2ch
  - switchaudio-osx
  - 시스템 권한 설정

## 개발자용 빌드

### 필수 도구
- Xcode Command Line Tools
- CMake 3.15 이상
- JUCE Framework

### 빌드 방법
```bash
# 저장소 클론
git clone <repository-url>
cd clr

# 빌드
mkdir build
cd build
cmake ..
make

# DMG 생성
cd ..
./create_clr_installer_dmg.sh
```

## 기능
- 오디오 플러그인 호스팅 (VST3, AudioUnit)
- 시스템 오디오 캡처 (BlackHole 2ch)
- 실시간 오디오 처리
- 오디오 녹음 기능
- MIDI 컨트롤 지원

## 라이선스
[라이선스 정보를 여기에 추가하세요]

## 지원
문제가 발생하면 이슈를 등록해주세요. 