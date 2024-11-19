#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GROUND_LEVEL = 500.0f;

struct Cloud {
    float x, y;
    float speed;
};

class Background {
public:
    Background(SDL_Renderer* renderer) : renderer(renderer) {
        castleRect = {100, 200, 600, 300};

        clouds = {
            {50, 100, 30},
            {300, 150, 20},
            {600, 120, 25}
        };
    }

    void renderBackground() {
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_Rect groundRect = {0, WINDOW_HEIGHT - 100, WINDOW_WIDTH, 100};
        SDL_RenderFillRect(renderer, &groundRect);

        SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);
        SDL_RenderFillRect(renderer, &castleRect);
    }

    void renderClouds(float deltaTime) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (auto& cloud : clouds) {
            cloud.x += cloud.speed * deltaTime;
            if (cloud.x > WINDOW_WIDTH) {
                cloud.x = -100;
            }
            SDL_Rect cloudRect = {static_cast<int>(cloud.x), static_cast<int>(cloud.y), 100, 50};
            SDL_RenderFillRect(renderer, &cloudRect);
        }
    }

private:
    SDL_Renderer* renderer;
    SDL_Rect castleRect;
    std::vector<Cloud> clouds;
};

class Character {
public:
    Character(SDL_Renderer* renderer, float x, float y, Mix_Chunk* jumpSound)
        : renderer(renderer), x(x), y(y), originalY(y), velocity(0.0f), isJumping(false), animationFrame(0), jumpSound(jumpSound) {}

    void startJump() {
        if (!isJumping) {
            isJumping = true;
            velocity = initialJumpVelocity;

            // Reproduce el sonido del salto
            Mix_PlayChannel(-1, jumpSound, 0);
        }
    }

    void moveLeft(float deltaTime) {
        x -= horizontalSpeed * deltaTime;
        if (x < 0) {
            x = 0;
        }
    }

    void moveRight(float deltaTime) {
        x += horizontalSpeed * deltaTime;
        if (x > WINDOW_WIDTH - characterWidth) {
            x = WINDOW_WIDTH - characterWidth;
        }
    }

    void update(float deltaTime) {
        if (isJumping) {
            velocity += gravity * deltaTime;
            y += velocity * deltaTime;

            animationFrame = (animationFrame + 1) % 20;

            if (y >= originalY) {
                y = originalY;
                isJumping = false;
                velocity = 0.0f;
                animationFrame = 0;
            }
        }
    }

    void renderCharacter() {
        int renderX = static_cast<int>(x);
        int renderY = static_cast<int>(y);

        int armOffset = 0;
        int legOffset = 0;

        if (isJumping) {
            armOffset = (animationFrame % 10) - 5;
            legOffset = ((animationFrame / 2) % 5) - 2;
        }

        SDL_SetRenderDrawColor(renderer, 255, 223, 0, 255);
        SDL_Rect crownRect = {renderX + 8, renderY - 8, 16, 8};
        SDL_RenderFillRect(renderer, &crownRect);

        SDL_SetRenderDrawColor(renderer, 255, 182, 193, 255);
        SDL_Rect headRect = {renderX + 8, renderY, 16, 16};
        SDL_RenderFillRect(renderer, &headRect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect bodyRect = {renderX + 8, renderY + 16, 16, 24};
        SDL_RenderFillRect(renderer, &bodyRect);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect leftArmRect = {renderX + armOffset, renderY + 16, 8, 16};
        SDL_Rect rightArmRect = {renderX + 24 - armOffset, renderY + 16, 8, 16};
        SDL_RenderFillRect(renderer, &leftArmRect);
        SDL_RenderFillRect(renderer, &rightArmRect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect leftLegRect = {renderX + 8 + legOffset, renderY + 40, 8, 16};
        SDL_Rect rightLegRect = {renderX + 16 - legOffset, renderY + 40, 8, 16};
        SDL_RenderFillRect(renderer, &leftLegRect);
        SDL_RenderFillRect(renderer, &rightLegRect);
    }

    float getX() const { return x; }
    float getY() const { return y; }

private:
    SDL_Renderer* renderer;
    float x, y;
    float originalY;
    float velocity;
    bool isJumping;
    int animationFrame;
    const float gravity = 2000.0f;
    const float initialJumpVelocity = -900.0f;
    const float horizontalSpeed = 800.0f;
    const int characterWidth = 32;

    Mix_Chunk* jumpSound;
};

class Dog {
public:
    Dog(SDL_Renderer* renderer, float x, float y, Mix_Chunk* dieSound)
        : renderer(renderer), x(x), y(y), movingRight(true), dieSound(dieSound) {}

    void update(float deltaTime, float playerX, float playerY) {
        if (movingRight) {
            x += speed * deltaTime;
            if (x > WINDOW_WIDTH - dogWidth) {
                x = WINDOW_WIDTH - dogWidth;
                movingRight = false;
            }
        } else {
            x -= speed * deltaTime;
            if (x < 0) {
                x = 0;
                movingRight = true;
            }
        }

        if (abs(x - playerX) < dogWidth && abs(y - playerY) < 32) {
            Mix_PlayChannel(-1, dieSound, 0);
        }
    }

    void renderDog() {
        int renderX = static_cast<int>(x);
        int renderY = static_cast<int>(y);

        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
        SDL_Rect headRect = {renderX, renderY, 16, 16};
        SDL_RenderFillRect(renderer, &headRect);

        SDL_SetRenderDrawColor(renderer, 160, 82, 45, 255);
        SDL_Rect bodyRect = {renderX - 8, renderY + 16, 32, 16};
        SDL_RenderFillRect(renderer, &bodyRect);

        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); 
        SDL_Rect leftLeg = {renderX - 6, renderY + 32, 8, 8};
        SDL_Rect rightLeg = {renderX + 14, renderY + 32, 8, 8};
        SDL_RenderFillRect(renderer, &leftLeg);
        SDL_RenderFillRect(renderer, &rightLeg);

        SDL_SetRenderDrawColor(renderer, 160, 82, 45, 255);
        SDL_Rect tailRect = {renderX - 10, renderY + 20, 8, 4};
        SDL_RenderFillRect(renderer, &tailRect);
    }

private:
    SDL_Renderer* renderer;
    float x, y;
    bool movingRight;
    const float speed = 200.0f;  
    const int dogWidth = 32;

    Mix_Chunk* dieSound;
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error inicializando SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Error inicializando SDL_mixer: " << Mix_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Castle Background with Clouds and Jumping Character", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Mix_Music* backgroundMusic = Mix_LoadMUS("fondo.wav");
    Mix_Chunk* jumpSound = Mix_LoadWAV("salto.wav");
    Mix_Chunk* dieSound = Mix_LoadWAV("die.wav");

    if (!backgroundMusic || !jumpSound || !dieSound) {
        std::cerr << "Error cargando sonidos: " << Mix_GetError() << std::endl;
        return -1;
    }

    Mix_PlayMusic(backgroundMusic, -1);

    Background background(renderer);
    Character character(renderer, 350.0f, GROUND_LEVEL, jumpSound);
    Dog dog(renderer, 100.0f, GROUND_LEVEL, dieSound);

    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    SDL_Event event;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP) {
                    character.startJump();
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    character.moveRight(deltaTime);
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    character.moveLeft(deltaTime);
                }
            }
        }

        character.update(deltaTime);
        dog.update(deltaTime, character.getX(), character.getY());

        background.renderBackground();
        background.renderClouds(deltaTime);
        character.renderCharacter();
        dog.renderDog();

        SDL_RenderPresent(renderer);
    }

    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(dieSound);
    Mix_CloseAudio();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
