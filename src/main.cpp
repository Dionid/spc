#include <ctime>
#include <thread>
#include <chrono>
#include "cengine/cengine.h"
#include "cengine/loader.h"
#include "utils.h"
#include "audio.h"
#include "entity.h"
#include "match.h"
#include "menus.h"
#include "game_tick.h"
#include "scene.h"

void simulationPipeline(
    MainScene* scene
) {
    // # Init scene
    scene->Init();

    // ## Init Nodes
    for (const auto& node: scene->nodeStorage->rootNodes) {
        node->TraverseInit();
    }

    // ## Node Storage
    scene->nodeStorage->Init();

    // ## Tick Manager
    SpcGameTickManager tickManager = SpcGameTickManager(scene->nodeStorage.get());

    // ## Main Loop
    const int targetFPS = 60;
    const int fixedUpdateRate = 40;

    const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);
    const std::chrono::milliseconds targetFixedUpdateTime(1000 / fixedUpdateRate);

    auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds accumulatedFixedTime(0);

    int fixedUpdateCyclesLimit = 10;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // # Tick
        scene->frameTick++;

        // # Start
        auto frameStart = std::chrono::high_resolution_clock::now();

        // # Input
        auto currentPlayerInput = cen::PlayerInput{
            IsKeyDown(KEY_W),
            IsKeyDown(KEY_S),
            IsKeyDown(KEY_A),
            IsKeyDown(KEY_D)
        };

        // # Update
        scene->nodeStorage->InitNewNodes();

        // # Fixed update
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
        lastFixedFrameTime = now;
        accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

        int fixedUpdateCycles = 0;
        while (accumulatedFixedTime >= targetFixedUpdateTime && fixedUpdateCycles < fixedUpdateCyclesLimit) {
            tickManager.currentTick++;

            // # Reconcile GameStateTick
            // ## Take arrived GameStateTick and check if they are correct
            const auto& compareResult = tickManager.CompareArrivedAndPending();

            if (
                compareResult.invalidPendingGameStateTickId == -1
            ) {
                // ## Merge correct GameStateTick
                tickManager.RemoveValidated(compareResult);
            } else {
                // ## Rollback and Apply
                tickManager.Rollback(compareResult);

                // ## Simulate new GameTicks using PlayerInputTicks
                for (const auto& playerInputTick: tickManager.playerInputTicks) {
                    scene->playerInput = playerInputTick.input;
                    scene->SimulationTick();
                    tickManager.SaveGameTick(scene->playerInput);
                }
            }

            // # Simulation current Tick
            scene->playerInput = currentPlayerInput;
            scene->SimulationTick();
            tickManager.SaveGameTick(scene->playerInput);

            // ## Correct time and cycles
            accumulatedFixedTime -= targetFixedUpdateTime;
            fixedUpdateCycles++;
        }

        // # Initial
        for (const auto& node: scene->nodeStorage->rootNodes) {
            node->TraverseUpdate();
        }

        // # Flush events
        for (const auto& topic: scene->topics) {
            topic->flush();
        }

        scene->eventBus->flush();

        // # Sync GameState and RendererState
        auto alpha = static_cast<double>(accumulatedFixedTime.count()) / targetFixedUpdateTime.count();

        scene->renderingEngine->SyncRenderBuffer(
            scene->nodeStorage.get(),
            alpha
        );

        // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
        // # End (busy wait)
        while (std::chrono::high_resolution_clock::now() - frameStart <= targetFrameTime) {}
    }
};

void renderingPipeline(cen::Scene* scene) {
    scene->renderingEngine->runPipeline(&scene->debugger);
}

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    EnableCursor();
    SetTargetFPS(FPS);

    // # Audio
    InitAudioDevice();

    SpcAudio gameAudio = {
        Sound(
            LoadSound(cen::GetResourcePath("audio/start.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/hit.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/score.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/lost.wav").c_str())
        ),
        Sound(
            LoadSound(cen::GetResourcePath("audio/win.wav").c_str())
        )
    };

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // # Scene
    MainScene scene = MainScene(
        &gameAudio,
        cen::ScreenResolution{screenWidth, screenHeight},
        &camera
    );

    cen::NetworkManager networkManager;

    // # Game Loop Thread
    std::vector<std::thread> threads;

    threads.push_back(std::thread(simulationPipeline, &scene));

    // # Render Loop Thread
    renderingPipeline(&scene);

    // # Exit
    // ## Join threads after stop signal
    for (auto& thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // ## Audio
    CloseAudioDevice();

    // ## Close window
    CloseWindow();

    return 0;
}