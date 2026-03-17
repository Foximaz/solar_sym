#include "utils.h"
#include "rendering.h"
#include "object.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <functional>

Camera::Camera(
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
):
    window(window),
    target(target),
    position(target->position),
    yaw(yaw),
    pitch(pitch),
    distance(distance),
    intentYaw(yaw),
    intentPitch(pitch),
    intentDistance(distance),
    fov(fov),
    rotationSpeed(rotationSpeed),
    zoomSpeed(zoomSpeed),
    smoothness(smoothness),
    showNames(showNames),
    font(font),
    characterSize(characterSize),
    minShow(minShow),
    iconSize(iconSize)
{}

Vector3 Camera::getForward() const {
    return Vector3(
        std::cos(pitch) * std::sin(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::cos(yaw)
    ).normalized();
}

Vector3 Camera::getRight() const {
    return Vector3(
        std::cos(yaw),
        0,
        -std::sin(yaw)
    ).normalized();
}

Vector3 Camera::getUp() const {
    return Vector3(
        -std::sin(pitch) * std::sin(yaw),
        std::cos(pitch),
        -std::sin(pitch) * std::cos(yaw)
    ).normalized();
}

void Camera::clampPitch() {
    const float maxPitch = 89.0f * M_PI / 180.0f;
    if (intentPitch > maxPitch) intentPitch = maxPitch;
    if (intentPitch < -maxPitch) intentPitch = -maxPitch;
}

bool Camera::projectToScreen(const Vector3& worldPos, sf::Vector2f& screenPos, float& scale) {
    Vector3 toPoint = worldPos - position;
    
    Vector3 forward = getForward();
    Vector3 right   = getRight();
    Vector3 up      = getUp();
    
    float x = toPoint.dot(right);
    float y = toPoint.dot(up);
    float z = toPoint.dot(forward);
    
    if (z <= 0.0f) return false;
    
    float aspect = static_cast<float>(window.getSize().x) / window.getSize().y;
    float fovRad = fov * M_PI / 180.0f;
    float tanHalfFov = std::tan(fovRad / 2.0f);
    
    float x_ndc = (x / z) / (tanHalfFov * aspect);
    float y_ndc = (y / z) / tanHalfFov;
    
    screenPos.x = (x_ndc + 1.0f) * 0.5f * window.getSize().x;
    screenPos.y = (1.0f - y_ndc) * 0.5f * window.getSize().y;
    
    scale = 200.0f / z;
    return true;
}

void Camera::switchShowNames() { showNames = !showNames; }

void Camera::processInput(float dt, short dYaw, short dPitch, short dDistance, float multiplier) {
    intentYaw += dYaw * rotationSpeed * multiplier;
    intentPitch += dPitch * rotationSpeed * multiplier;
    intentDistance *= (1 + dDistance * zoomSpeed * multiplier);
}

void Camera::setTarget(Object* target) {
    this->target = target;
}

bool Camera::handleMouseClick(sf::Vector2i mousePos, Object*& clickedObject) const {
    clickedObject = nullptr;
    
    for (auto it = renderInfos.rbegin(); it != renderInfos.rend(); ++it) {
        const auto& info = *it;
        
        sf::Vector2f delta = sf::Vector2f(mousePos) - info.screenPos;
        float distSq = delta.x * delta.x + delta.y * delta.y;
        float radiusSq = info.screenRadius * info.screenRadius;
        
        if (distSq <= radiusSq) {
            clickedObject = info.object;
            return true;
        }
    }
    return false;
}

void Camera::update(float dt) {
    yaw = lerp(intentYaw, yaw, smoothness);
    pitch = lerp(intentPitch, pitch, smoothness);
    distance = lerp(intentDistance, distance, smoothness);
    clampPitch();
    position = sphericalToCartesian(target->position, M_PI + yaw, -pitch, distance);
}

void Camera::render(std::vector<Object>& objects) {
    window.clear(sf::Color::Black);
    renderInfos.clear();

    std::vector<std::pair<float, Object*>> sorted;
    for (auto& object : objects) {
        Vector3 toPoint = object.position - position;
        float depth = toPoint.dot(getForward());
        sorted.push_back({depth, &object});
    }
    
    std::sort(sorted.begin(), sorted.end(), 
        [](const auto& a, const auto& b) { return a.first > b.first; });
    
    for (auto& [depth, object] : sorted) {
        sf::Vector2f screenPos;
        float scale;
        if (projectToScreen(object->position, screenPos, scale)) {
            bool show = sqrt(object->mass) * scale > minShow;
            if (show) {
                bool useIcon = object->size * scale < iconSize + 1.5;
                
                float radius;
                if (useIcon) {
                    radius = iconSize;
                    sf::CircleShape icon(iconSize);
                    icon.setFillColor(sf::Color::Transparent);
                    icon.setOutlineColor(object->color);
                    icon.setOutlineThickness(3);
                    icon.setOrigin(iconSize, iconSize);
                    icon.setPosition(screenPos);
                    window.draw(icon);
                } else {
                    radius = object->size * scale;
                    sf::CircleShape circle(object->size * scale);
                    circle.setFillColor(object->color);
                    circle.setOrigin(circle.getRadius(), circle.getRadius());
                    circle.setPosition(screenPos);
                    window.draw(circle);
                }
                renderInfos.push_back({object, screenPos, radius});
            
                if (showNames) {
                    sf::Text nameText;
                    nameText.setFont(font);
                    nameText.setString(object->name);
                    nameText.setCharacterSize(characterSize);
                    nameText.setFillColor(sf::Color::White);
                    
                    sf::FloatRect bounds = nameText.getLocalBounds();
                    nameText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                    
                    nameText.setPosition(screenPos.x, screenPos.y - 15.0f - std::max(iconSize, object->size * scale));
                    
                    window.draw(nameText);
                }
            }
        }
    }
    window.display();
}

Button::Button(
    sf::Font& font,
    const std::string& buttonText,
    sf::Vector2f position,
    sf::Vector2f size,
    std::function<void()> callback
)
: font(font)
, onClick(callback)
, normalColor(sf::Color(100, 100, 250))
, hoverColor(sf::Color(150, 150, 250))
, pressedColor(sf::Color(50, 50, 200))
, isHovered(false)
, isPressed(false)
{
    shape.setPosition(position);
    shape.setSize(size);
    shape.setFillColor(normalColor);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::Black);

    text.setFont(font);
    text.setString(buttonText);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);

    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width/2.0f,
        textBounds.top  + textBounds.height/2.0f);
    text.setPosition(position + size/2.0f);
}

void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

    bool contains = shape.getGlobalBounds().contains(worldPos);

    if (event.type == sf::Event::MouseMoved) {
        isHovered = contains;
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left && contains) {
            isPressed = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isPressed && contains && onClick) {
                onClick();
            }
            isPressed = false;
        }
    }
}

void Button::update() {
    if (isPressed) {
        shape.setFillColor(pressedColor);
    } else if (isHovered) {
        shape.setFillColor(hoverColor);
    } else {
        shape.setFillColor(normalColor);
    }
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}
