#include "scene.h"
#include "network.h"

#ifndef CENGINE_LOCK_STEP_SCENE_H
#define CENGINE_LOCK_STEP_SCENE_H

namespace cen {

struct PlayerInputTick {
    cen::player_id_t playerId;
    uint64_t tick;
    cen::PlayerInput input;
};

static std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

struct PlayerInputNetworkMessage {
    cen::player_id_t playerId;
    std::vector<PlayerInputTick> inputs;

    PlayerInputNetworkMessage(cen::player_id_t playerId, std::vector<PlayerInputTick> inputs) {
        this->playerId = playerId;
        this->inputs = inputs;
    }

    std::string Serialize() {
        std::string serializedInputs = "";

        for (int i = 0; i < inputs.size(); i++) {
            auto input = inputs[i];
            serializedInputs += std::to_string(input.tick) + ";" + std::to_string(input.input.down) + ";" + std::to_string(input.input.up) + ";" + std::to_string(input.input.left) + ";" + std::to_string(input.input.right);

            if (i < inputs.size() - 1) {
                serializedInputs += ",";
            }
        }

        return std::to_string(this->playerId) + "|" + std::to_string(inputs.size()) + "|" + serializedInputs;
    }

    static PlayerInputNetworkMessage Deserialize(std::string message) {
        auto parts = split(message, '|');
        cen::player_id_t playerId = std::stoll(parts[0]);
        auto inputsCount = std::stoi(parts[1]);
        auto inputs = split(parts[2], ',');

        std::vector<PlayerInputTick> playerInputTicks;

        for (const auto& input: inputs) {
            auto inputParts = split(input, ';');
            auto tick = std::stoull(inputParts[0]);
            bool down = std::stoi(inputParts[1]);
            bool up = std::stoi(inputParts[2]);
            bool left = std::stoi(inputParts[3]);
            bool right = std::stoi(inputParts[4]);

            playerInputTicks.push_back(
                PlayerInputTick{
                    playerId,
                    tick,
                    cen::PlayerInput{down, up, left, right}
                }
            );
        }

        return PlayerInputNetworkMessage(playerId, playerInputTicks);
    }
};

class LockStepNetworkManager {
    public:
        bool isHost;
        NetworkManager* networkManager;
        UdpTransport* transport;
        cen::player_id_t currentPlayerId;
        std::vector<PlayerInputNetworkMessage> playerInputMessages;
        std::vector<PlayerInputTick> inputsBuffer;

        LockStepNetworkManager(
            NetworkManager* networkManager,
            bool isHost
        ) {
            this->networkManager = networkManager;
            this->isHost = isHost;
            this->transport = nullptr;
        }

        void Init() {
            if (this->isHost) {
                this->currentPlayerId = 1;
                this->transport = this->networkManager->CreateAndInitUdpTransportServer(
                    "lock-step-transport",
                    "127.0.0.1",
                    1234,
                    1000,
                    [this](NetworkMessage message) {
                        std::cout << "Message from client: " << message.data << std::endl;

                        if (message.type != NetworkMessageType::NEW_MESSAGE) {
                            return;
                        }

                        auto playerInputMessage = PlayerInputNetworkMessage::Deserialize(message.data);

                        this->playerInputMessages.push_back(playerInputMessage);

                        // TODO: Change to ring buffer
                        if (this->playerInputMessages.size() > 100) {
                            this->playerInputMessages.erase(this->playerInputMessages.begin() + 20);
                        }
                    }
                );
            } else {
                this->currentPlayerId = 2;
                this->transport = this->networkManager->CreateAndInitUdpTransportClient(
                    "lock-step-transport",
                    "127.0.0.1",
                    1234,
                    1000,
                    [this](NetworkMessage message) {
                        std::cout << "Message from server: " << message.data << std::endl;

                        if (message.type != NetworkMessageType::NEW_MESSAGE) {
                            return;
                        }

                        auto playerInputMessage = PlayerInputNetworkMessage::Deserialize(message.data);

                        this->playerInputMessages.push_back(playerInputMessage);

                        // TODO: Change to ring buffer
                        if (this->playerInputMessages.size() > 100) {
                            this->playerInputMessages.erase(this->playerInputMessages.begin() + 20);
                        }
                    }
                );
            }

            this->transport->Init();
        }

        void SendTickInput() {
            auto message = PlayerInputNetworkMessage(
                this->currentPlayerId,
                this->inputsBuffer
            ).Serialize();
            this->transport->SendMessage(message);
        }

        PlayerInputTick SendAndWaitForPlayersInputs(
            uint64_t tick,
            PlayerInput currentPlayerInput
        ) {
            this->inputsBuffer.push_back(
                PlayerInputTick{
                    this->currentPlayerId,
                    tick,
                    currentPlayerInput
                }
            );

            // TODO: Change to ring buffer
            if (this->inputsBuffer.size() > 30) {
                this->inputsBuffer.erase(this->inputsBuffer.begin() + 20);
            }

            this->SendTickInput();

            int counter = 0;

            while (counter < 100) {
                for (const auto& playerInputMessage: this->playerInputMessages) {
                    for (const auto& input: playerInputMessage.inputs) {
                        if (input.tick == tick) {
                            return input;
                        }
                    }
                }
                // counter++;
                std::this_thread::yield();
            }

            // auto previousPlayerInput = this->playerInputMessages.end();

            // if (previousPlayerInput != this->playerInputMessages.end()) {
            //     return *previousPlayerInput;
            // }

            return PlayerInputTick{0, 0, PlayerInput()};
        }
};

class LockStepScene: public Scene {
    public:
        bool isHost;
        NetworkManager* networkManager;

        LockStepScene(
            bool isHost,
            NetworkManager* networkManager,
            scene_name name,
            cen::ScreenResolution screen,
            Camera2D* camera,
            RenderingEngine2D* renderingEngine,
            cen::EventBus* eventBus,
            int simulationFrameRate = 60,
            int simulationFixedFrameRate = 40,
            int simulationFixedFrameCyclesLimit = 10,
            cen::PlayerInputManager playerInputManager = cen::PlayerInputManager{},
            std::unique_ptr<cen::CollisionEngine> collisionEngine = std::make_unique<cen::CollisionEngine>(),
            std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>()
        ): Scene(
            name,
            screen,
            camera,
            renderingEngine,
            eventBus,
            simulationFrameRate,
            simulationFixedFrameRate,
            simulationFixedFrameCyclesLimit,
            playerInputManager,
            std::move(collisionEngine),
            std::move(nodeStorage)
        ) {
            this->isHost = isHost;
            this->networkManager = networkManager;
        }

        void RunSimulation() {
            if (!this->isInitialized) {
                // # Init scene
                this->FullInit();
            }

            LockStepNetworkManager lockStepNetworkManager(this->networkManager, this->isHost);
            lockStepNetworkManager.Init();

            std::chrono::milliseconds accumulatedFixedTime(0);
            auto lastFixedFrameTime = std::chrono::high_resolution_clock::now();

            while (this->isAlive.load(std::memory_order_acquire))    // Detect window close button or ESC key
            {
                // # Frame Tick
                this->frameTick++;

                // # Input
                auto currentPlayerInput = cen::PlayerInput{
                    IsKeyDown(KEY_W),
                    IsKeyDown(KEY_S),
                    IsKeyDown(KEY_A),
                    IsKeyDown(KEY_D)
                };

                // # Get other player input
                auto otherPlayerInput = lockStepNetworkManager.SendAndWaitForPlayersInputs(
                    this->frameTick,
                    currentPlayerInput
                );

                this->playerInputManager.playerInputs[otherPlayerInput.playerId] = otherPlayerInput.input;
                this->playerInputManager.currentPlayerInput = currentPlayerInput;

                // # Start
                auto frameStart = std::chrono::high_resolution_clock::now();

                // # Init new nodes
                this->nodeStorage->InitNewNodes();

                // # Fixed update
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> fixedFrameDuration = now - lastFixedFrameTime;
                lastFixedFrameTime = now;
                accumulatedFixedTime += std::chrono::duration_cast<std::chrono::milliseconds>(fixedFrameDuration);

                int fixedUpdateCycles = 0;
                while (accumulatedFixedTime >= simulationFixedFrameRate && fixedUpdateCycles < simulationFixedFrameCyclesLimit) {
                    this->fixedSimulationTick++;

                    // # Simulation current Tick
                    this->FixedSimulationTick();

                    // ## Correct time and cycles
                    accumulatedFixedTime -= simulationFixedFrameRate;
                    fixedUpdateCycles++;
                }

                // # Initial
                for (const auto& node: this->nodeStorage->rootNodes) {
                    node->TraverseUpdate();
                }

                // # Flush events
                this->eventBus.Flush();

                // # Sync GameState and RendererState
                auto alpha = static_cast<double>(accumulatedFixedTime.count()) / simulationFixedFrameRate.count();

                this->renderingEngine->SyncRenderBuffer(
                    this->nodeStorage.get(),
                    alpha
                );

                // QUESTION: maybe sleep better? But it overshoots (nearly 3ms)
                // # End (busy wait)
                while (std::chrono::high_resolution_clock::now() - frameStart <= simulationFrameRate) {}
            }
        }
};

}

#endif