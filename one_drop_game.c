#include <SDL2/SDL.h>
#include <stdio.h> 
#include <stdbool.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int TARGET_Y = 500;
const int TARGET_HEIGHT = 50;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    SDL_Window* window = SDL_CreateWindow("Rhythm Drop", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event;

    // --- 게임 상태 변수 ---
    float noteY = 0.0f;           
    float noteSpeed = 5.0f;       
    bool noteActive = true;       
    
    // 🔥 시작 점수(체력) 50점
    int score = 50;                
    
    int feedbackTimer = 0;        
    int feedbackType = 0;         

    printf("====================================\n");
    printf(" 🎮 게임을 시작합니다!\n");
    printf(" - 현재 점수: 50점\n");
    printf(" - 목표: 150점 달성 시 클리어 🌟\n"); 
    printf(" - 주의: 0점 달성 시 게임 오버 💀\n");
    printf("====================================\n\n");

    while (isRunning) {
        // 1. 이벤트 처리 (키보드 입력 감지)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            // 스페이스바가 눌렸을 때
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (noteActive) {
                    if (noteY + 50 >= TARGET_Y && noteY <= TARGET_Y + TARGET_HEIGHT) {
                        score += 10;
                        feedbackType = 1; // Hit!
                        printf("[HIT!] 현재 점수: %d / 150 (속도: %.1f)\n", score, noteSpeed); 
                    } else {
                        score -= 5;
                        feedbackType = 2; // Miss!
                        printf("[BAD!] 너무 빠르거나 늦었습니다. 점수: %d / 150\n", score); 
                    }
                    noteActive = false; 
                    feedbackTimer = 20; 
                    
                    // 150점 클리어 및 0점 게임 오버 판정
                    if (score >= 150) { 
                        printf("\n====================================\n");
                        printf(" 🎉 [ 게임 클리어! ] 150점 달성! 🎉\n"); 
                        printf("====================================\n\n");
                        isRunning = false; 
                    } else if (score <= 0) {
                        printf("\n====================================\n");
                        printf(" 💀 [ 게임 오버... ] 점수가 0점이 되었습니다. 💀\n");
                        printf("====================================\n\n");
                        isRunning = false; 
                    }
                }
            }
        }

        // 2. 게임 로직 업데이트
        if (isRunning && noteActive) { 
            noteY += noteSpeed; 
            
            // 바닥에 닿아서 놓친 경우
            if (noteY > WINDOW_HEIGHT) {
                noteActive = false;
                feedbackType = 2;
                feedbackTimer = 20;
                score -= 5;
                printf("[MISS!] 노트를 놓쳤습니다. 점수: %d / 150\n", score); 
                
                // 바닥에 떨어뜨려서 0점이 된 경우도 체크
                if (score <= 0) {
                    printf("\n====================================\n");
                    printf(" 💀 [ 게임 오버... ] 점수가 0점이 되었습니다. 💀\n");
                    printf("====================================\n\n");
                    isRunning = false; 
                }
            }
        } else if (isRunning && !noteActive) {
            // 노트가 없고 피드백 타이머가 끝나면 새로운 노트 생성
            if (feedbackTimer <= 0) {
                noteY = 0;
                noteActive = true;
                noteSpeed += 1.5f; // 맞출 때마다 속도 1.5씩 증가
            }
        }

        if (feedbackTimer > 0) feedbackTimer--;

        // 3. 화면 렌더링
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // 목표 지점(판정선) 그리기
        if (feedbackTimer > 0) {
            if (feedbackType == 1) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);      
            else if (feedbackType == 2) SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); 
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); 
        }
        SDL_Rect targetRect = { 300, TARGET_Y, 200, TARGET_HEIGHT };
        SDL_RenderFillRect(renderer, &targetRect); 

        // 떨어지는 노트 그리기
        if (noteActive) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); 
            SDL_Rect noteRect = { 350, (int)noteY, 100, 50 };
            SDL_RenderFillRect(renderer, &noteRect);
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); 
    }

    // 5. 자원 해제 및 종료
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}