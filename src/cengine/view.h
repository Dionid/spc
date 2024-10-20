#ifndef CENGINE_VIEW_H_
#define CENGINE_VIEW_H_

#include "texture.h"
#include "node_2d.h"
#include "collision.h"

// # Views

namespace cen {

class TextView: public cen::Node2D {
    public:
        std::string  text;
        int fontSize;
        Color color;

        TextView(
            Vector2 position,
            std::string text,
            int fontSize,
            Color color
        ): cen::Node2D(position) {
            this->text = text;
            this->fontSize = fontSize;
            this->color = color;
        }
};

class LineView: public cen::Node2D {
    public:
        float length;
        float alpha;
        Color color;

        LineView(Vector2 position, float length, Color color = WHITE, float alpha = 1.0f,  Vector2 globalPosition = Vector2{}, bool ySort = false, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->length = length;
            this->alpha = alpha;
            this->color = color;
        }
};

class CircleView: public cen::Node2D {
    public:
        float radius;
        float alpha;
        Color color;
        bool fill;

        CircleView(float radius, Vector2 position = Vector2{}, Color color = WHITE, float alpha = 1.0f, bool fill = true, bool ySort = false, int zOrder = 0,  Vector2 globalPosition = Vector2{}, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->radius = radius;
            this->alpha = alpha;
            this->color = color;
            this->fill = fill;
        }
};

class RectangleView: public cen::Node2D {
    public:
        cen::Size size;
        Color color;
        float alpha;

        RectangleView(cen::Size size, Color color = WHITE, float alpha = 1.0f, Vector2 position = Vector2{}, bool ySort = false, int zOrder = 0, uint16_t id = 0, Node* parent = nullptr): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->size = size;
            this->color = color;
            this->alpha = alpha;
        }
};

class TextureView: public cen::Node2D {
    public:
        Texture texture;
        Rectangle texturePosition;
        Size size;

        TextureView(
            Texture texture,
            Rectangle texturePosition,
            Vector2 position,
            Size size,
            bool ySort = false,
            int zOrder = 0,
            uint16_t id = 0,
            Node* parent = nullptr
        ): cen::Node2D(position, ySort, zOrder, id, parent) {
            this->size = size;
            this->texturePosition = texturePosition;
            this->texture = texture;
        }
};

} // namespace cen

#endif // CENGINE_VIEW_H_