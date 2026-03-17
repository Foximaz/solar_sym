#ifndef SCENE_H
#define SCENE_H

#include "object.h"
#include "rendering.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Scene {
private:
    sf::RenderWindow& window;
    std::vector<Object> objects;
    Camera camera;

    double G;
    double softening;

    double simulationSpeed = 1.0;
    double fixedTimestep = 1.0 / 60.0;
    double accumulator = 0.0;
    bool pause = false;

    std::vector<Vector3> computeAccelerations(const std::vector<Object>& objects) const;

public:
    Scene(
        sf::RenderWindow& window,
        const std::string& dictPath,
        double G,
        double softening
    );

    void processInput(float dt);
    void update(float dt);
    void render();
};

#endif
