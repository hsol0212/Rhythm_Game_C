# Rhythm_Game_C
Rhythm game working on C Linux

🎮 4-Key Audio-Driven Rhythm Game (C & SDL2)

C언어와 SDL2 (Simple DirectMedia Layer) 라이브러리를 활용하여 개발한 2D 그래픽 기반의 아케이드 리듬 게임입니다. 단순한 시간 간격 스폰 방식에서 벗어나, 직접 구현한 고속 푸리에 변환(Cooley-Tukey FFT) 알고리즘을 통해 실시간 오디오 신호의 주파수 대역별 에너지를 분석하고 비트에 맞춰 노트가 동적으로 생성되도록 구현했습니다.

🚀 1. 시스템 아키텍처 및 핵심 기술

🎵 1) 실시간 오디오 주파수 분석 (Custom Cooley-Tukey FFT)

주파수 도메인 변환: SDL_mixer의 포스트 믹스(Post-Mix) 콜백을 통해 스트리밍되는 오디오 샘플 데이터를 받아, 1차원 Cooley-Tukey FFT 알고리즘(재귀 방식)을 직접 구현하여 시간 도메인의 오디오 신호를 주파수 도메인으로 변환했습니다.

4-Lane 대역 분배: FFT 결과 산출된 복소수의 절댓값(볼륨 에너지)을 바탕으로 주파수 대역을 4가지 구간으로 나누어 에너지를 집계합니다.

Lane 0 (저음역대): $20\text{Hz} \sim 150\text{Hz}$
Lane 1 (중저음역대): $150\text{Hz} \sim 600\text{Hz}$
Lane 2 (중고음역대): $600\text{Hz} \sim 2500\text{Hz}$
Lane 3 (고음역대): $2500\text{Hz} \sim 10000\text{Hz}$

비트마스크(Bitmask) 스폰 시스템: 각 대역의 평균 에너지가 설정된 임계값을 초과할 때 비트마스크(spawnMask)를 생성하여, 단일 노트뿐만 아니라 동시 타격 노트까지 유연하게 스폰되도록 설계했습니다.

🖼️ 2) 60 FPS 더블 버퍼링 그래픽 렌더링

더블 버퍼링 (Double Buffering): 모니터 화면에 직접 도형과 이미지를 그리면 발생하는 화면 깜빡임(Flickering) 현상을 방지하기 위해, 백 버퍼(Back Buffer)에 모든 요소를 렌더링한 후 SDL_RenderPresent를 통해 프런트 버퍼와 교체하는 방식을 채택했습니다.

프레임 고정 (SDL_Delay(16)): 메인 루프에 적절한 딜레이를 주어 1초에 약 60번 화면을 갱신하는 안정적인 60 FPS를 유지했습니다.

🎯 3) 정밀 판정 및 동적 가속도 (Dynamic Level Design)

AABB 충돌 판정: 화살표의 위치와 판정선 좌표를 비교하여 실시간으로 HIT / MISS를 판정합니다.
선형 보간 가속: 점수가 오를수록 노트 속도(noteSpeed)가 미세하게 증가하며, 속도가 빨라짐에 따라 생성 주기(Spawn Rate)도 수학적으로 연동되어 지루하지 않은 플레이 환경을 제공합니다.


🕹️ 2. 게임 규칙 및 승패 조건
시작 점수: 50점에서 시작합니다.
타이밍 판정:
HIT (+10점): 판정선 안에서 올바른 방향키 입력 시 판정선이 초록색으로 변하며 점수 획득 및 속도 증가.
MISS / BAD (-5점): 키 입력 미스 또는 노트를 놓칠 경우 판정선이 빨간색으로 변하며 점수 차감.

승패 조건:
Clear: 500점 달성 시 승리 화면 출력.
Game Over: 점수가 0점 이하로 떨어질 경우 게임 오버 처리.


⚙️ 3. 개발 및 실행 환경 (Linux / WSL)
필수 라이브러리 설치
sudo apt update
sudo apt install gcc build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev

컴파일 및 실행 명령어
gcc -o rhythm_game main.c audio.c graphics.c -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm
./rhythm_game


🛠️ 4. 트러블슈팅 (Troubleshooting)
문제 현상 (Linux/WSL 오디오 초기화 에러):
ALSA lib ... Unknown PCM sysdefault 에러가 발생하며 오디오 장치를 열지 못하는 현상 발생.

원인 및 해결:
리눅스 환경에 기본 PulseAudio 플러그인 설정이 누락되어 발생한 문제로, 아래와 같이 설정을 보완하여 해결했습니다.

sudo apt install -y pulseaudio-utils libasound2-plugins
nano ~/.asoundrc

.asoundrc 파일 내부에 아래 내용을 작성 후 저장합니다.
pcm.default pulse
ctl.default pulse


이후 WSL 환경일 경우 wsl --shutdown 명령어로 재시작 후 정상 구동을 확인했습니다.
