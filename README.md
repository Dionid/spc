# Raylib Pong

# Roadmap

1. Menu
    1. Start, end and reset match
1. Fix ball collision
1. Audio
1. Release (macOS, Windows, Linux, web)
1. CollisionEngine pair.id
1. Node.id on BitMask
    1. NodeStorage index by class
1. Add units (meters, seconds, etc.)
1. Remove raylib
1. Name convention (https://google.github.io/styleguide/cppguide.html#General_Naming_Rules)
1. SAT
1. Z index
1. Remove CharacterNode2D.size
1. NodeGenerator id graveyard
1. Defer Node Add & Remove
1. Multithreading
    1. Physics and Render pipelines
1. Maybe
    1. ? Arena allocator

# Features

1. CharacterBody2D (move and slide, move and collide)
1. Collisions
1. Custom RTTI

# Useful Links

1. [VSCode CLang](https://code.visualstudio.com/docs/cpp/config-clang-mac)

# Caution

1. Use global position to know position of the Node in the world (i.e. combining all parent positions).
1. Put Collider directly into ColliderBody2D.
1. Initial nested Nodes must be added in Init method.
1. ...