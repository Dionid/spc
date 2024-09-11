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

void gameLoopPipeline(
    cen::GameContext* ctx,
    SpcAudio* gameAudio
) {
    // # MatchEndMenu
    auto matchEndMenu = ctx->scene->node_storage->AddNode(std::make_unique<MatchEndMenu>(
        [&](cen::GameContext* ctx) {
            ctx->scene->eventBus.emit(RestartEvent());
        }
    ));

    matchEndMenu->Deactivate();

    // # Match
    MatchManager* matchManager = ctx->scene->node_storage->AddNode(std::make_unique<MatchManager>(
        gameAudio,
        [&](cen::GameContext* ctx) {
            matchManager->Deactivate();
            matchEndMenu->SetPlayerWon(ctx, matchManager->playerScore > matchManager->enemyScore);
            matchEndMenu->Activate();
            EnableCursor();
        }
    ));

    matchManager->Deactivate();

    // # MainMenu
    MainMenu* mainMenu = ctx->scene->node_storage->AddNode(std::make_unique<MainMenu>(
        [&](cen::GameContext* ctx) {
            ctx->scene->eventBus.emit(StartEvent());
        }
    ));

    auto ose = cen::EventListener(
        [&](cen::GameContext* ctx, const cen::Event& event) {
            PlaySound(gameAudio->start);
            mainMenu->Deactivate();
            matchEndMenu->Deactivate();
            matchManager->Reset(ctx);
            matchManager->Activate();
            DisableCursor();
        }
    );

    ctx->scene->eventBus.on(
        StartEvent{},
        &ose
    );

    ctx->scene->eventBus.on(
        RestartEvent{},
        &ose
    );

    // # Init
    // # While nodes are initing more of them can be added
    for (const auto& node: ctx->scene->node_storage->nodes) {
        node->TraverseInit(ctx);
    }

    // # Node Storage
    ctx->scene->node_storage->Init();

    const int targetFPS = 60;
    const std::chrono::milliseconds targetFrameTime(1000 / targetFPS);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // ## Update
        //----------------------------------------------------------------------------------
        ctx->scene->node_storage->InitNewNodes(ctx);

        // ## Initial
        for (const auto& node: ctx->scene->node_storage->nodes) {
            node->TraverseUpdate(ctx);
        }

        // ## Collision
        ctx->collisionEngine->NarrowCollisionCheckNaive(ctx);

        // // ## Map Game state to Renderer
        ctx->scene->renderingEngine->MapNodesToCanvasItems(
            ctx->scene->node_storage.get()
        );

        // ## Flush events
        for (const auto& topic: ctx->scene->topics) {
            topic->flush();
        }

        ctx->scene->eventBus.flush(ctx);

        auto frameEnd = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> frameDuration = frameEnd - frameStart;

        if (frameDuration < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - frameDuration);
        }
    }
};

int main() {
    // # Init
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "super pong");
    EnableCursor();
    SetTargetFPS(FPS);

    cen::Debugger debugger;

    // # Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

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

    // # Scene
    cen::Scene scene = cen::Scene();

    cen::CollisionEngine collisionEngine;

    // # Game Context
    cen::GameContext ctx = {
        &scene,
        &collisionEngine,
        screenWidth,
        screenHeight
    };

    // # Game Loop Thread
    std::thread gameLoopThread(gameLoopPipeline, &ctx, &gameAudio);

    // # Render Loop Thread
    int counter = 0;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
            ClearBackground(BLACK);
            scene.renderingEngine->Render();
            // TODO: Refactore debug
            // debugger.Render(&ctx);
        EndDrawing();
    }

    gameLoopThread.join();

    CloseAudioDevice();

    // ## De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}