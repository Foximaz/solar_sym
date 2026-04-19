#ifndef RENDERING_H
#define RENDERING_H

#include "utils.h"
#include "object.h"
#include <SFML/Graphics.hpp>
#include <string>

struct RenderInfo {
    Object* object;
    sf::Vector2f screenPos;
    float screenRadius;
};

class Camera {
private:
    float MAX_PITCH_DEGREES = 89.0f;
    float MIN_DISTANCE = 1e-7;
    float FOCAL_LENGTH = 200.0f;
    float ICON_OUTLINE_THICKNESS = 3.0f;

    Vector3 position;
    Object* target;
    float yaw;
    float pitch;
    float distance;
    float intentYaw;
    float intentPitch;
    float intentDistance;
    float fov;
    float rotationSpeed;
    float zoomSpeed;
    float smoothness;
    bool showNames;
    sf::Font font;
    unsigned int characterSize;
    float minShow;
    float iconSize;
    std::vector<RenderInfo> renderInfos;
    sf::RenderWindow& window;

    Vector3 getForward() const;
    Vector3 getRight() const;
    Vector3 getUp() const;
    void clampPitch();
    void clampDistance();
    bool projectToScreen(const Vector3& worldPos, sf::Vector2f& screenPos, float& scale);

public:    
    Camera(
        sf::RenderWindow& window,
        Object* target,
        float yaw,
        float pitch,
        float distance,
        float fov,
        float rotationSpeed,
        float zoomSpeed,
        float smoothness,
        bool showNames,
        sf::Font font,
        unsigned int characterSize,
        float minShow,
        float iconSize
    );

    void setShowNames(bool show);
    bool getShowNames() const;

    void setTarget(Object* target);
    
    void processInput(float dt, short dYaw, short dPitch, float dDistance, float multiplier);
    bool handleMouseClick(sf::Vector2i mousePos, Object*& clickedObject) const;
    void update(float dt);
    void render(std::vector<Object>& objects);
};

#endif
