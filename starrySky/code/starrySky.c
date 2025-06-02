#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>

#define NUM_STARS 1000

/*
 ** Description
 * Creates a window to be displayed on the screen
 ** Parameters
 * windowTitle: the title that is displayed on the menu bar of the window
 * windowPosX: the x position on the screen to display the window
 * windowPosY: the y position on the screen to display the window
 * windowSizeX: the x size of the displayed window
 * windowSizeY: the y size of the displayed window
 * flags: additional flags for things like look of window, functionality, etc...
 ** Return Value
 * window: returns a pointer to the window that has been displayed or null if error
 */
SDL_Window *createWindow(const char *windowTitle, int windowPosX, int windowPosY, int windowSizeX, int windowSizeY, int flags) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Failed to initialize the SDL2 library\n");
        return NULL;
    }

    SDL_Window *window = SDL_CreateWindow(windowTitle, windowPosX, windowPosY, windowSizeX, windowSizeY, flags);

    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return NULL;
    }

    return window;
}

// function to draw an ellipse
void fillEllipse(SDL_Renderer* renderer, int cx, int cy, int a, int b) {
    for (int y = cy - b; y <= cy + b; y++) {
        int dy = y - cy;
        double dx = a * sqrt(1.0 - (dy * dy) / (double)(b * b));
        int x1 = (int)(cx - dx);
        int x2 = (int)(cx + dx);
        SDL_RenderDrawLine(renderer, x1, y, x2, y);
    }
}


// Struct for each star generated
typedef struct {
    int x, y;       // current position (for drawing)
    float radius;   // distance from center point
    float angle;    // current angle from center point in radians
    uint8_t brightness;
} Star;


int main() {
    int isGameRunning = 0;

    int SCREEN_WIDTH = 640; // 1280 // 640
    int SCREEN_HEIGHT = 480; // 720 // 480
    int starCX = SCREEN_WIDTH/2;
    int starCY = SCREEN_HEIGHT;

    // create a window
    SDL_Window *window = createWindow("Starry Sky", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

    // Create renderer and set bg to black
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    isGameRunning = 1;


    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
 
    SDL_LoadWAV("../assets/toTheGateway_MarioGalaxy.wav", &wavSpec, &wavBuffer, &wavLength);

    // open audio device (speakers, headphones, etc)
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    // clear audio queue
    SDL_ClearQueuedAudio(deviceId);

    // play audio
    SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);

    // init random number stuffs
    srand(time(NULL));


    // Initialize stars in polar coordinates relative to center
    Star starArray[NUM_STARS] = {0};
    float maxRadius = SCREEN_HEIGHT+(SCREEN_HEIGHT/4);
    for (int i = 0; i < NUM_STARS; i++) {
        float angle = ((float)rand() / RAND_MAX) * 2 * M_PI; // random angle [0, 2π]
        float radius = sqrtf((float)rand() / RAND_MAX) * maxRadius; // uniform distribution

        starArray[i].x = starCX + radius * cosf(angle);
        starArray[i].y = starCY + radius * sinf(angle);

        starArray[i].angle = angle;
        starArray[i].radius = radius;

        // random brightness between 100-255
        starArray[i].brightness = 100 + rand() % (255-100+1);
    }


    const float rotationSpeed = 0.0002f; // radians per frame

    while (isGameRunning == 1) {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0) {
            if (e.type == SDL_QUIT) {
                isGameRunning = 0;
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

                // regenerate the stars
                SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
                starCX = SCREEN_WIDTH/2;
                starCY = SCREEN_HEIGHT;

                maxRadius = SCREEN_HEIGHT+(SCREEN_HEIGHT/4);
                for (int i = 0; i < NUM_STARS; i++) {
                    float angle = ((float)rand() / RAND_MAX) * 2 * M_PI; // random angle [0, 2π]
                    float radius = sqrtf((float)rand() / RAND_MAX) * maxRadius; // uniform distribution
            
                    starArray[i].x = starCX + radius * cosf(angle);
                    starArray[i].y = starCY + radius * sinf(angle);
            
                    starArray[i].angle = angle;
                    starArray[i].radius = radius;
            
                    // random brightness between 100-255
                    starArray[i].brightness = 100 + rand() % (255-100+1);
                }
            }
        }

        // Check if audio finished playing (no queued audio)
        if (SDL_GetQueuedAudioSize(deviceId) == 0) {
            // Re-queue audio to loop
            SDL_QueueAudio(deviceId, wavBuffer, wavLength);
        }


        // clear screen to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
        SDL_RenderClear(renderer);

        // rotate stars along starCX and starCY axis
        for (int i = 0; i < NUM_STARS; i++) {
            starArray[i].angle += rotationSpeed;

            starArray[i].x = starCX + starArray[i].radius * cosf(starArray[i].angle);
            starArray[i].y = starCY + starArray[i].radius * sinf(starArray[i].angle);


            // draw point (if within bounds)
            if ((int)starArray[i].x >= 0 && (int)starArray[i].x <= SCREEN_WIDTH && (int)starArray[i].y >= 0 && (int)starArray[i].y <= SCREEN_HEIGHT) {
                SDL_SetRenderDrawColor(renderer, starArray[i].brightness, starArray[i].brightness, starArray[i].brightness, 255);
                SDL_RenderDrawPoint(renderer, (int)starArray[i].x, (int)starArray[i].y);
            }
        }


        // draw landscape
        SDL_SetRenderDrawColor(renderer, 22, 29, 21, 255);
        fillEllipse(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT, SCREEN_WIDTH/2+50, SCREEN_HEIGHT/10);

        // display updated frame
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }


    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}