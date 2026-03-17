#ifndef RENDERING_H
#define RENDERING_H

#include "utils.h"
#include "object.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

struct RenderInfo {
    Object* object;
    sf::Vector2f screenPos;
    float screenRadius;
};

class Camera {
private:
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

    void switchShowNames();
    void setTarget(Object* target);
    void processInput(float dt, short dYaw, short dPitch, short dDistance, float multiplier);
    bool handleMouseClick(sf::Vector2i mousePos, Object*& clickedObject) const;
    void update(float dt);
    void render(std::vector<Object>& objects);
};

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Font& font;
    std::function<void()> onClick;
    
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;
    
    bool isHovered;
    bool isPressed;
    
public:
    Button(
        sf::Font& font,
        const std::string& buttonText,
        sf::Vector2f position,
        sf::Vector2f size,
        std::function<void()> callback
    );
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update();
    void draw(sf::RenderWindow& window);
    
    void setPosition(sf::Vector2f pos);
    void setText(const std::string& newText);
};

#endif
