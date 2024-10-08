#ifndef CENGINE_NODE_H_
#define CENGINE_NODE_H_

#include <vector>
#include <iostream>
#include "core.h"

namespace cen {

class Scene;

class NodeIdGenerator {
public:
    NodeIdGenerator(const NodeIdGenerator&) = delete;
    NodeIdGenerator& operator=(const NodeIdGenerator&) = delete;

    static NodeIdGenerator& GetInstance() {
        static NodeIdGenerator instance;
        return instance;
    }

    // Method to get the next ID
    node_id_t GetNextId() {
        std::lock_guard<std::mutex> lock(mutex_);
        return ++counter_;
    }

    node_id_t typeZero() {
        return 0;
    }

private:
    NodeIdGenerator() : counter_(0) {}

    node_id_t counter_;
    std::mutex mutex_;
};

// # Node

class NodeStorage;

class Node: public cen::WithType {
    public:
        bool isInitialized = false;
        NodeStorage* storage;
        Node* parent;
        Scene* scene;
        std::vector<std::unique_ptr<Node>> children;
        node_id_t id;
        bool activated = true;

        static const uint64_t _tid;

        cen::type_id_t TypeId() const override {
            return Node::_tid;
        }

        Node(
            node_id_t id = 0,
            Node* parent = nullptr
        ) {
            this->parent = parent;
        }

        virtual ~Node() {}

        void Deactivate() {
            this->activated = false;
        }

        void Activate() {
            this->activated = true;
        }

        // TODO: refactor this
        bool AnyParentDeactivated() {
            if (this->activated == false) {
                return true;
            }

            if (this->parent != nullptr) {
                return this->parent->AnyParentDeactivated();
            }

            return false;
        }

        virtual void Init() {};
        virtual void FixedUpdate() {};
        virtual void Update() {};

        virtual void InvalidatePrevious() {};

        // # implementations in node_node_storage.h
        template <typename T>
        T* AddNode(std::unique_ptr<T> node);

        // # implementations in node_node_storage.h
        void RemoveChild(Node* node);
        
        // # implementations in node_node_storage.h
        void RemoveChildById(node_id_t id);

        Node* GetById(node_id_t targetId) {
            if (this->id == targetId) {
                return this;
            }

            for (const auto& node: this->children) {
                auto nestedFound = node->GetById(targetId);
                if (nestedFound != nullptr) {
                    return nestedFound;
                }
            }

            return nullptr;
        }

        template <typename T>
        T* GetById(node_id_t targetId) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            if (auto nPtr = dynamic_cast<T*>(this)) {
                if (nPtr->id == targetId) {
                    return nPtr;
                }
            }

            for (const auto& node: this->children) {
                auto nestedFound = node->GetById<T>(targetId);
                if (nestedFound != nullptr) {
                    return nestedFound;
                }
            }

            return nullptr;
        }

        template <typename T>
        void GetChildByType(std::vector<T*>& nodes) {
            for (const auto& node: this->children) {
                if (T* targetType = dynamic_cast<T*>(node.get())) {
                    nodes.push_back(targetType);
                }
            }
        }

        template <typename T>
        void GetChildByTypeDeep(std::vector<T*>& targetNodes) {
            for (const auto& childNode: this->children) {
                if (T* targetType = dynamic_cast<T*>(childNode.get())) {
                    targetNodes.push_back(targetType);
                }
                childNode->GetChildByTypeDeep<T>(targetNodes);
            }
        }

        template <typename T>
        T* GetFirstChildByType() {
            for (const auto& node: this->children) {
                if (auto targetNode = dynamic_cast<T*>(node.get())) {
                    return targetNode;
                }
            }

            return nullptr;
        }

        template <typename T>
        T* GetFirstByType() {
            if (auto targetNode = dynamic_cast<T*>(this)) {
                return targetNode;
            }

            return this->GetFirstChildByType<T>();
        }

        Node* RootNode() {
            if (this->parent != nullptr) {
                return this->parent->RootNode();
            }

            return this;
        };

        void TraverseInit() {
            if (!this->isInitialized) {
                this->Init();
                this->isInitialized = true;
            }

            for (const auto& node: this->children) {
                node->TraverseInit();
            }
        }

        void TraverseUpdate() {
            if (this->activated == false) {
                return;
            }

            this->Update();
            for (const auto& node: this->children) {
                node->TraverseUpdate();
            }
        };

        void TraverseFixedUpdate() {
            if (this->activated == false) {
                return;
            }

            this->FixedUpdate();
            for (const auto& node: this->children) {
                node->TraverseFixedUpdate();
            }
        };

        void TraverseInvalidatePrevious() {
            if (this->activated == false) {
                return;
            }

            this->InvalidatePrevious();
            for (const auto& node: this->children) {
                node->TraverseInvalidatePrevious();
            }
        }
};

} // namespace cen

#endif // CENGINE_NODE_H_