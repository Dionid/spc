#include <raylib.h>
#include <raymath.h>
#include "engine.h"

#ifndef CENGINE_COLLISION_H
#define CENGINE_COLLISION_H

struct CollisionHit {
    float penetration;
    Vector2 normal;
};

CollisionHit CircleRectangleCollision(
    Vector2 circlePosition,
    float circleRadius,
    Vector2 rectPosition,
    Size rectSize
) {
    Vector2 closest = {
        std::clamp(circlePosition.x, rectPosition.x - rectSize.width/2, rectPosition.x + rectSize.width/2),
        std::clamp(circlePosition.y, rectPosition.y - rectSize.height/2, rectPosition.y + rectSize.height/2)
    };

    Vector2 distance = Vector2Subtract(circlePosition, closest);
    float distanceLength = Vector2Length(distance);

    if (distanceLength < circleRadius) {
        return {
            circleRadius - distanceLength,
            Vector2Normalize(distance)
        };
    }

    return {
        0,
        Vector2Zero()
    };
}

#endif //CENGINE_COLLISION_H