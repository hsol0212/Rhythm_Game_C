

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h> 
#include <time.h>   

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int TARGET_Y = 500;
const int TARGET_HEIGHT = 80;

#define MAX_NOTES 10 

// 노트(화살표) 구조체 정의
typedef struct {
    float y;        // 현재 Y 좌표
    int lane;       // 떨어지는 레인 번호 (0:왼, 1:위, 2:아래, 3:오른)
    bool active;    // 화면에 존재하는지 여부
} Note;

int main(int argc, char* argv[]) {
    // 1. 초기 세팅 및 리소스 로드
    srand((unsigned int)time(NULL)); // 랜덤 시드 설정

    // 비디오(그래픽) 및 이미지 라이브러리 초기화
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1; 
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return 1;

    // 게임 창 생성
    SDL_Window* window = SDL_CreateWindow("4-Key Rhythm Drop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return 1;

    // GPU 가속 렌더러 생성
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 화살표 이미지 텍스처 4개 로드 (배열 사용)
    SDL_Texture* arrowTextures[4];
    arrowTextures[0] = IMG_LoadTexture(renderer, "left.png");
    arrowTextures[1] = IMG_LoadTexture(renderer, "up.png");
    arrowTextures[2] = IMG_LoadTexture(renderer, "down.png");
    arrowTextures[3] = IMG_LoadTexture(renderer, "right.png");

    // 게임 상태 변수들
    bool isRunning = true;
    SDL_Event event;

    // 노트 배열 초기화 (오브젝트 풀링)
    Note notes[MAX_NOTES]; 
    for (int i = 0; i < MAX_NOTES; i++) notes[i].active = false; 
    
    float noteSpeed = 4.0f;       // 초기 떨어지는 속도
    int score = 50;               // 시작 체력(점수) 
    int spawnTimer = 0;           // 생성 타이머
    
    int laneFeedbackTimer[4] = {0, 0, 0, 0}; // 레인별 판정 이펙트 타이머
    int laneFeedbackType[4] = {0, 0, 0, 0};  // 1: Hit(초록), 2: Miss(빨강)
    int laneX[4] = { 200, 300, 400, 500 };   // 레인별 X 좌표

    printf("====================================\n");
    printf(" 🎮 화살표 리듬 게임 시작!\n");
    printf(" - 조작: ←(Left)  ↑(Up)  ↓(Down)  →(Right)\n");
    printf(" - 목표: 150점 달성 시 클리어 🌟\n");
    printf("====================================\n\n");

    // 2. 메인 게임 루프 시작
    while (isRunning) {
        
        // --- [입력 처리] ---
        while (SDL_PollEvent(&event)) {
            // 강제 종료 (X 버튼)
            if (event.type == SDL_QUIT) {
                isRunning = false;
                score = 50; // 대기실 로직 패스
            }
            
            // 키보드 눌림 감지
            if (event.type == SDL_KEYDOWN) {
                int pressedLane = -1; 
                if (event.key.keysym.sym == SDLK_LEFT) pressedLane = 0;
                else if (event.key.keysym.sym == SDLK_UP) pressedLane = 1;
                else if (event.key.keysym.sym == SDLK_DOWN) pressedLane = 2;
                else if (event.key.keysym.sym == SDLK_RIGHT) pressedLane = 3;

                // 4개의 방향키 중 하나를 눌렀다면
                if (pressedLane != -1) { 
                    bool hitSomething = false;
                    
                    // 해당 레인의 살아있는 노트들과 충돌(타이밍) 검사
                    for (int i = 0; i < MAX_NOTES; i++) {
                        if (notes[i].active && notes[i].lane == pressedLane) {
                            // 판정선 안에 노트가 들어왔는지 AABB 충돌 체크
                            if (notes[i].y + 80 >= TARGET_Y && notes[i].y <= TARGET_Y + TARGET_HEIGHT) {
                                score += 10;
                                laneFeedbackType[pressedLane] = 1; // 초록불
                                laneFeedbackTimer[pressedLane] = 15;
                                hitSomething = true;
                                notes[i].active = false; // 맞춘 노트 제거
                                
                                printf("[HIT!] 점수: %d / 150\n", score);
                                break; 
                            }
                        }
                    }
                    
                    // 타이밍이 틀렸거나 허공에 눌렀을 때
                    if (!hitSomething) {
                        score -= 5;
                        laneFeedbackType[pressedLane] = 2; // 빨간불
                        laneFeedbackTimer[pressedLane] = 15;
                        printf("[BAD!] 잘못 눌렀습니다. 점수: %d / 150\n", score);
                    }

                    // 승패 조건 확인
                    if (score >= 150) { 
                        printf("\n 🎉 [ 게임 클리어! ] 150점 달성! 🎉\n");
                        printf(" (게임을 종료하려면 창의 X 버튼이나 ESC 키를 누르세요)\n\n");
                        isRunning = false; 
                    } else if (score <= 0) {
                        printf("\n 💀 [ 게임 오버... ] 점수가 0점이 되었습니다. 💀\n");
                        printf(" (게임을 종료하려면 창의 X 버튼이나 ESC 키를 누르세요)\n\n");
                        isRunning = false; 
                    }
                }
            }
        }

        // --- [로직 업데이트] ---
        if (isRunning) {
            
            // 🔥 점진적 가속 로직: 매 프레임마다 속도를 아주 미세하게 증가 (선형 보간)
            if (noteSpeed < 15.0f) {
                noteSpeed += 0.003f; 
            }

            spawnTimer++;
            
            // 🔥 동적 난이도 조절: 속도가 빨라지면 화살표 생성 주기(대기 시간)도 짧아짐
            int currentSpawnRate = 60 - (int)((noteSpeed - 4.0f) * 5);
            if (currentSpawnRate < 15) currentSpawnRate = 15; // 생성 주기 하한선 방어 (너무 안 겹치게)

            // 타이머가 다 차면 새로운 화살표 생성
            if (spawnTimer >= currentSpawnRate) {
                spawnTimer = 0;
                for (int i = 0; i < MAX_NOTES; i++) {
                    // 비어있는 노트 슬롯을 찾아 재활용
                    if (!notes[i].active) {
                        notes[i].active = true;
                        notes[i].y = 0;
                        notes[i].lane = rand() % 4; // 랜덤한 레인에서 스폰
                        break;
                    }
                }
            }

            // 모든 노트 물리 이동 및 화면 이탈(Miss) 처리
            for (int i = 0; i < MAX_NOTES; i++) {
                if (notes[i].active) {
                    notes[i].y += noteSpeed; // 속도만큼 아래로 이동
                    
                    // 바닥에 떨어뜨렸을 때
                    if (notes[i].y > WINDOW_HEIGHT) {
                        notes[i].active = false;
                        score -= 5;
                        laneFeedbackType[notes[i].lane] = 2; 
                        laneFeedbackTimer[notes[i].lane] = 15;
                        
                        // 현재 가속도가 얼마나 붙었는지 확인용 출력
                        printf("[MISS!] 노트를 놓쳤습니다. 점수: %d / 150 (현재 속도: %.2f)\n", score, noteSpeed);
                        
                        if (score <= 0) {
                            printf("\n 💀 [ 게임 오버... ] 점수가 0점이 되었습니다. 💀\n");
                            printf(" (게임을 종료하려면 창의 X 버튼이나 ESC 키를 누르세요)\n\n");
                            isRunning = false; 
                        }
                    }
                }
            }
        }

        // 판정선 색상 유지 타이머 감소
        for (int i = 0; i < 4; i++) {
            if (laneFeedbackTimer[i] > 0) laneFeedbackTimer[i]--;
        }

        // --- [화면 렌더링 (더블 버퍼링)] ---
        
        // 1. 배경을 파스텔 톤 파란색으로 지우기
        SDL_SetRenderDrawColor(renderer, 240, 248, 255, 255);
        SDL_RenderClear(renderer);

        // 2. 판정선 4개 그리기
        for (int i = 0; i < 4; i++) {
            if (laneFeedbackTimer[i] > 0) {
                if (laneFeedbackType[i] == 1) SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);      // 초록 (Hit)
                else if (laneFeedbackType[i] == 2) SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255); // 빨강 (Miss/Bad)
            } else {
                SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // 기본 색상 (회색)
            }
            SDL_Rect targetRect = { laneX[i], TARGET_Y, 80, TARGET_HEIGHT };
            SDL_RenderFillRect(renderer, &targetRect); 
        }

        // 3. 살아있는 화살표 텍스처 그리기
        for (int i = 0; i < MAX_NOTES; i++) {
            if (notes[i].active) {
                SDL_Rect noteRect = { laneX[notes[i].lane], (int)notes[i].y, 80, 80 };
                SDL_RenderCopy(renderer, arrowTextures[notes[i].lane], NULL, &noteRect);
            }
        }
        
        // 4. 모니터로 송출 및 프레임 제한 (약 60FPS)
        SDL_RenderPresent(renderer);
        SDL_Delay(16); 
    }

    // 3. 게임 종료 후 대기 및 자원 해제
    
    // 점수 조건(승/패)으로 종료된 경우, 창이 얼어붙지 않도록 입력을 대기하는 루프
    if (score >= 150 || score <= 0) {
        bool waitExit = true;
        while (waitExit) {
            while (SDL_PollEvent(&event)) {
                // X 버튼이나 ESC 키를 누르면 최종 종료
                if (event.type == SDL_QUIT || 
                   (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                    waitExit = false;
                }
            }
            SDL_Delay(50); // CPU 과부하 방지
        }
    }

    // 메모리 반환 (누수 방지)
    for(int i = 0; i < 4; i++) SDL_DestroyTexture(arrowTextures[i]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit(); 
    SDL_Quit();

    return 0;
}

