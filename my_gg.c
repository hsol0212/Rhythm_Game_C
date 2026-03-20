#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // 🔥 이미지 사용을 위해 추가!
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h> 
#include <time.h>   

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int TARGET_Y = 500;
const int TARGET_HEIGHT = 80; // 화살표 크기에 맞춰 판정선 두께를 조금 키웠습니다

#define MAX_NOTES 10 

typedef struct {
    float y;
    int lane;       
    bool active;
} Note;

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL)); 

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    
    // 🔥 PNG 이미지 로드 기능 초기화
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image 초기화 실패: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("4-Key Rhythm Drop", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return 1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 🔥 화살표 텍스처(이미지) 4개를 저장할 배열
    SDL_Texture* arrowTextures[4];
    arrowTextures[0] = IMG_LoadTexture(renderer, "left.png");
    arrowTextures[1] = IMG_LoadTexture(renderer, "up.png");
    arrowTextures[2] = IMG_LoadTexture(renderer, "down.png");
    arrowTextures[3] = IMG_LoadTexture(renderer, "right.png");

    // 이미지가 잘 불러와졌는지 확인
    for(int i = 0; i < 4; i++) {
        if(!arrowTextures[i]) {
            printf("이미지 로드 실패! 폴더에 left.png, up.png, down.png, right.png가 있는지 확인하세요.\n");
            return 1;
        }
    }

    bool isRunning = true;
    SDL_Event event;

    Note notes[MAX_NOTES]; 
    for (int i = 0; i < MAX_NOTES; i++) notes[i].active = false; 
    
    float noteSpeed = 4.0f;       
    int score = 50;                
    
    int spawnTimer = 0;       
    int spawnRate = 60;       
    
    int laneFeedbackTimer[4] = {0, 0, 0, 0}; 
    int laneFeedbackType[4] = {0, 0, 0, 0};  
    int laneX[4] = { 200, 300, 400, 500 }; 

    printf("====================================\n");
    printf(" 🎮 화살표 그래픽 모드 시작!\n");
    printf("====================================\n\n");

    while (isRunning) {
        // 1. 이벤트 처리
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            
            if (event.type == SDL_KEYDOWN) {
                int pressedLane = -1; 
                if (event.key.keysym.sym == SDLK_LEFT) pressedLane = 0;
                else if (event.key.keysym.sym == SDLK_UP) pressedLane = 1;
                else if (event.key.keysym.sym == SDLK_DOWN) pressedLane = 2;
                else if (event.key.keysym.sym == SDLK_RIGHT) pressedLane = 3;

                if (pressedLane != -1) { 
                    bool hitSomething = false;
                    for (int i = 0; i < MAX_NOTES; i++) {
                        if (notes[i].active && notes[i].lane == pressedLane) {
                            if (notes[i].y + 80 >= TARGET_Y && notes[i].y <= TARGET_Y + TARGET_HEIGHT) {
                                score += 10;
                                laneFeedbackType[pressedLane] = 1; 
                                laneFeedbackTimer[pressedLane] = 15;
                                hitSomething = true;
                                notes[i].active = false; 
                                printf("[HIT!] 점수: %d / 150\n", score);
                                break; 
                            }
                        }
                    }
                    if (!hitSomething) {
                        score -= 5;
                        laneFeedbackType[pressedLane] = 2; 
                        laneFeedbackTimer[pressedLane] = 15;
                    }

                    if (score >= 150 || score <= 0) isRunning = false; 
                }
            }
        }

        // 2. 로직 업데이트
        if (isRunning) {
            spawnTimer++;
            if (spawnTimer >= spawnRate) {
                spawnTimer = 0;
                for (int i = 0; i < MAX_NOTES; i++) {
                    if (!notes[i].active) {
                        notes[i].active = true;
                        notes[i].y = 0;
                        notes[i].lane = rand() % 4; 
                        if (noteSpeed < 12.0f) noteSpeed += 0.2f; 
                        if (spawnRate > 20) spawnRate -= 1;
                        break;
                    }
                }
            }

            for (int i = 0; i < MAX_NOTES; i++) {
                if (notes[i].active) {
                    notes[i].y += noteSpeed;
                    if (notes[i].y > WINDOW_HEIGHT) {
                        notes[i].active = false;
                        score -= 5;
                        laneFeedbackType[notes[i].lane] = 2; 
                        laneFeedbackTimer[notes[i].lane] = 15;
                        if (score <= 0) isRunning = false; 
                    }
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            if (laneFeedbackTimer[i] > 0) laneFeedbackTimer[i]--;
        }

        // 3. 화면 그리기
        SDL_SetRenderDrawColor(renderer, 200, 230, 255, 255);
        SDL_RenderClear(renderer);

        // 판정선 그리기
        for (int i = 0; i < 4; i++) {
            if (laneFeedbackTimer[i] > 0) {
                if (laneFeedbackType[i] == 1) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);      
                else if (laneFeedbackType[i] == 2) SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); 
            } else {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); 
            }
            SDL_Rect targetRect = { laneX[i], TARGET_Y, 80, TARGET_HEIGHT };
            SDL_RenderFillRect(renderer, &targetRect); 
        }

        // 🔥 떨어지는 화살표 이미지 그리기 🔥
        for (int i = 0; i < MAX_NOTES; i++) {
            if (notes[i].active) {
                // 이미지가 그려질 위치와 크기 (가로 80, 세로 80의 정사각형)
                SDL_Rect noteRect = { laneX[notes[i].lane], (int)notes[i].y, 80, 80 };
                // SDL_RenderFillRect 대신 SDL_RenderCopy를 사용하여 이미지를 입힙니다.
                SDL_RenderCopy(renderer, arrowTextures[notes[i].lane], NULL, &noteRect);
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); 
    }

    // 4. 🔥 자원 해제 (이미지도 메모리에서 해제해야 함)
    for(int i = 0; i < 4; i++) {
        SDL_DestroyTexture(arrowTextures[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit(); // 이미지 라이브러리 종료
    SDL_Quit();

    return 0;
}