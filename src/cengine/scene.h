#ifndef CENGINE_SCENE_H
#define CENGINE_SCENE_H

#include <vector>
#include "rendering.h"
#include "node_storage.h"
#include "collision.h"
#include "event.h"

namespace cen {
    class Scene {
        public:
            Camera2D* camera;
            cen::CollisionEngine* collisionEngine;
            cen::EventBus eventBus;
            cen::ScreenResolution screen;
            cen::PlayerInput playerInput;
            std::unique_ptr<cen::RenderingEngine2D> renderingEngine;
            std::unique_ptr<cen::NodeStorage> nodeStorage;
            std::vector<std::unique_ptr<cen::TopicBase>> topics;

            // TODO: refactor init
            Scene(
                Camera2D* camera,
                cen::CollisionEngine* collisionEngine,
                cen::ScreenResolution screen,
                std::unique_ptr<NodeStorage> nodeStorage = std::make_unique<NodeStorage>(),
                std::unique_ptr<RenderingEngine2D> renderingEngine = std::make_unique<RenderingEngine2D>(),
                std::vector<std::unique_ptr<cen::TopicBase>> topics = {},
                EventBus eventBus = EventBus(),
                cen::PlayerInput playerInput = cen::PlayerInput{}
            ) {
                this->camera = camera;
                this->collisionEngine = collisionEngine;
                this->screen = screen;
                this->nodeStorage = std::move(nodeStorage);
                this->nodeStorage->scene = this;
                this->renderingEngine = std::move(renderingEngine);
                this->topics = std::move(topics);
                this->eventBus = eventBus;
            }

            template <typename T>
            cen::Topic<T>* AddTopic(std::unique_ptr<cen::Topic<T>> topic) {
                static_assert(std::is_base_of<cen::TopicBase, cen::Topic<T>>::value, "T must inherit from TopicBase");

                auto topicPtr = topic.get();

                this->topics.push_back(
                    std::move(topic)
                );

                return topicPtr;
            }
    };
}

#endif // CENGINE_SCENE_H