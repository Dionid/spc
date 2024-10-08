#ifndef CSP_MATCH_H_
#define CSP_MATCH_H_

#include <functional>
#include "cengine/cengine.h"
#include "entity.h"
#include "audio.h"

class LaunchBallTimer: public cen::Timer {
    public:
        cen::node_id_t ballId;

        LaunchBallTimer(
            cen::node_id_t ballId,
            int duration
        );

        void OnTimerEnd() override;
};

class MatchManager: public cen::Node2D {
    public:
        bool mirror;
        SpcAudio* gameAudio;
        cen::node_id_t ballId;
        cen::node_id_t playerId;
        cen::node_id_t enemyId;
        int playerScore;
        int enemyScore;
        int winScore;
        LaunchBallTimer* launchBallTimer;
        cen::TextView* playerScoreText;
        cen::TextView* enemyScoreText;

        MatchManager(
            SpcAudio* gameAudio,
            bool mirror = false,
            int winScore = 100,
            int playerScore = 0,
            int enemyScore = 0
        );

        void Reset();

        void PlayerScored();
        void EnemyScored();
        void ResetEntities();

        void Init() override;
        void FixedUpdate() override;
};

#endif // CSP_MATCH_H_