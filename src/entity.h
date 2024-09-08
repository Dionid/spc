#ifndef CSP_ENTITY_H_
#define CSP_ENTITY_H_

#include "cengine/cengine.h"
#include "audio.h"

// # Paddle
class Paddle: public CharacterBody2D {
    public:
        float speed;
        float maxVelocity;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Paddle::_tid;
        }

        Paddle(
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void ApplyFriction();
        void ApplyWorldBoundaries(float worldWidth, float worldHeight);
};

// # Player
class Player: public Paddle {
    public:
        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Player::_tid;
        }

        Player(
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Update(cen::GameContext* ctx) override;
        void Init(cen::GameContext* ctx) override;
};

// # Ball
class Ball: public CharacterBody2D {
    public:
        float radius;
        float maxVelocity;
        SpcAudio* gameAudio;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Ball::_tid;
        }

        Ball(
            SpcAudio* gameAudio,
            float radius,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float maxVelocity
        );

        void Init(cen::GameContext* ctx) override;
        void Update(cen::GameContext* ctx) override;
        void OnCollision(Collision c) override;
        void OnCollisionStarted(Collision c) override;
};

// # Enemy

class Enemy: public Paddle {
    public:
        node_id_t ballId;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Enemy::_tid;
        }

        Enemy(
            node_id_t ballId,
            Vector2 position,
            cen::Size size,
            Vector2 velocity,
            float speed,
            float maxVelocity
        );

        void Init(cen::GameContext* ctx) override;
        void Update(cen::GameContext* ctx) override;
};

// # Goal

class Goal: public CollisionObject2D {
    public:
        static const uint64_t _tid;
        bool isLeft;
        cen::Size size;
        Vector2 position;

        cen::type_id_t TypeId() const override {
            return Goal::_tid;
        }

        Goal(
            bool isLeft,
            Vector2 position,
            cen::Size size
        );

        void Init(cen::GameContext* ctx) override;
};

#endif // CSP_ENTITY_H_