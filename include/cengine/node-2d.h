#ifndef CENGINE_NODES_H
#define CENGINE_NODES_H

#include <raymath.h>
#include "node.h"

class Node2D: public Node {
    public:
        Vector2 position;

        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Node2D::_tid;
        }

        Node2D(Vector2 position, uint16_t id = 0, Node* parent = nullptr): Node(id, parent) {
            this->position = position;
        }

        Node2D* ClosestNode2DParent() {
            auto currentParent = this->parent;
            if (currentParent == nullptr) {
                return nullptr;
            }

            auto parent = dynamic_cast<Node2D*>(currentParent);

            if (parent != nullptr) {
                return parent;
            }

            return parent->ClosestNode2DParent();
        }

        Vector2 GlobalPosition() {
            auto parent = this->ClosestNode2DParent();

            if (parent == nullptr) {
                return this->position;
            }

            return Vector2Add(parent->GlobalPosition(), this->position);
        }

        Node2D* RootNode2D() {
            auto p = this->ClosestNode2DParent();

            if (p == nullptr) {
                return this;
            }

            return p->RootNode2D();
        }
};

#endif // CENGINE_NODES_H