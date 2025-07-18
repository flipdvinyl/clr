#!/bin/bash

echo "=== clr 설치 파일 생성 ==="

# 빌드 디렉토리 생성
mkdir -p build
cd build

# CMake 설정 및 빌드
echo "CMake 설정 중..."
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake 설정 실패"
    exit 1
fi

echo "빌드 중..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "빌드 실패"
    exit 1
fi

cd ..

# DMG 구조 생성
echo "DMG 구조 생성 중..."
mkdir -p clr_Installer

# 앱 번들 복사
if [ -d "build/clr_artefacts/clr.app" ]; then
    cp -R build/clr_artefacts/clr.app clr_Installer/
    echo "clr.app 복사 완료"
else
    echo "clr.app을 찾을 수 없습니다. 빌드가 성공했는지 확인하세요."
    exit 1
fi

# 배경 이미지 생성 (설치 안내)
cat > background.svg << 'EOF'
<svg width="400" height="300" xmlns="http://www.w3.org/2000/svg">
  <rect width="400" height="300" fill="#f0f0f0"/>
  <text x="200" y="50" text-anchor="middle" font-family="Arial" font-size="24" fill="#333">
    clr 설치
  </text>
  <text x="200" y="80" text-anchor="middle" font-family="Arial" font-size="14" fill="#666">
    1. clr.app을 Applications로 드래그하세요
  </text>
  <text x="200" y="100" text-anchor="middle" font-family="Arial" font-size="14" fill="#666">
    2. clr를 실행하면 자동으로 설치가 시작됩니다
  </text>
  <text x="200" y="120" text-anchor="middle" font-family="Arial" font-size="14" fill="#666">
    3. "다음" 버튼만 누르면 모든 설정이 완료됩니다
  </text>
</svg>
EOF

# DMG 생성
echo "DMG 파일 생성 중..."
hdiutil create -volname "clr Installer" -srcfolder clr_Installer -ov -format UDZO clr_Installer.dmg

if [ $? -eq 0 ]; then
    echo "=== DMG 파일 생성 완료 ==="
    echo "생성된 파일: clr_Installer.dmg"
    echo "파일 크기: $(ls -lh clr_Installer.dmg | awk '{print $5}')"
else
    echo "DMG 파일 생성 실패"
    exit 1
fi

# 정리
rm -rf clr_Installer
rm background.svg

echo "=== 완료 ==="
echo "clr_Installer.dmg 파일이 생성되었습니다."
echo "이 파일을 사용자에게 배포하세요." 