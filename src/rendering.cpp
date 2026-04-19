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

void Camera::clampDistance() {
    if (intentDistance < 1e-7) intentDistance = 1e-7;
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

void Camera::setShowNames(bool show) { showNames = show; }
bool Camera::getShowNames() const { return showNames; }

void Camera::processInput(float dt, short dYaw, short dPitch, float dDistance, float multiplier) {
    intentYaw += dYaw * rotationSpeed * multiplier * dt;
    intentPitch += dPitch * rotationSpeed * multiplier * dt;
    
    intentDistance *= (1.0f + dDistance * zoomSpeed * multiplier * dt);

    clampPitch();
    clampDistance();
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
    position = sphericalToCartesian(target->position, M_PI + yaw, -pitch, distance);
}

void Camera::render(std::vector<Object>& objects) {
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
            bool show = (object == target) || sqrt(object->mass) * scale > minShow;
            if (show) {
                float currentIconSize = iconSize * (1 + log(0.01 + object->mass) / 8);
                bool useIcon = object->size * scale < currentIconSize + 1.5;
                
                float radius;
                if (useIcon) {
                    radius = currentIconSize;
                    sf::CircleShape icon(currentIconSize);
                    icon.setFillColor(sf::Color::Transparent);
                    icon.setOutlineColor(object->color);
                    icon.setOutlineThickness(3);
                    icon.setOrigin(currentIconSize, currentIconSize);
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
                    
                    nameText.setPosition(screenPos.x, screenPos.y - 15.0f - std::max(currentIconSize, object->size * scale));
                    
                    window.draw(nameText);
                }
            }
        }
    }
}
