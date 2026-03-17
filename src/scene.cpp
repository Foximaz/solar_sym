#include "scene.h"
#include "load.h"
#include <string>
#include <filesystem>
#include <iostream> //DEBUG

Scene::Scene(
    sf::RenderWindow& window,
    const std::string& dictPath,
    double G,
    double softening
)
    : window(window)
    , objects(load_obj_config((std::filesystem::path(dictPath) / "obj_config.json").string()))
    , camera(load_cam_config(window, objects, (std::filesystem::path(dictPath) / "cam_config.json").string()))
    , G(G), softening(softening)
{}

void Scene::processInput(float dt) {
    short dDistance = 0;
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                dDistance += event.mouseWheelScroll.delta;
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Num1:
                    simulationSpeed = 1.0;
                    std::cout << "Speed: 1x" << std::endl;
                    break;
                case sf::Keyboard::Num2:
                    simulationSpeed *= 4;
                    std::cout << "Speed: " << simulationSpeed << "x" << std::endl;
                    break;
                case sf::Keyboard::Num3:
                    simulationSpeed *= 0.25;
                    std::cout << "Speed: " << simulationSpeed << "x" << std::endl;
                    break;
                case sf::Keyboard::Space:
                    pause = !pause;
                default:
                    break;
            }
        }
    }

    short dYaw = 0;
    short dPitch = 0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) dYaw += 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dYaw -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) dPitch -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) dPitch += 1;

    double multiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.1 : 1;
    camera.processInput(dt, dYaw, dPitch, dDistance, multiplier);

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Object* clicked = nullptr;
        if (camera.handleMouseClick(mousePos, clicked)) {
            camera.setTarget(clicked);
        }
    }
}

std::vector<Vector3> Scene::computeAccelerations(const std::vector<Object>& objects) const {
    std::vector<Vector3> acc(objects.size(), Vector3(0, 0, 0));
    
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            Vector3 r = objects[j].position - objects[i].position;
            
            double distSq = r.x * r.x + r.y * r.y + r.z * r.z + softening * softening;
            double dist = std::sqrt(distSq);
            
            double factor = G / (distSq * dist);
            
            Vector3 acc_i = r * (factor * objects[j].mass);
            Vector3 acc_j = r * (-factor * objects[i].mass);
            
            acc[i] += acc_i;
            acc[j] += acc_j;
        }
    }
    return acc;
}

void Scene::update(float dt) {
    if (!pause) {
        double step;
        if (simulationSpeed > 1.0) {
            accumulator += dt * simulationSpeed;
            step = fixedTimestep;
        } else {
            accumulator += dt;
            step = fixedTimestep * simulationSpeed;
        }

        while (accumulator >= fixedTimestep) {
            std::vector<Vector3> acc0 = computeAccelerations(objects);
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i].velocity += acc0[i] * (step / 2.0);
            }

            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i].position += objects[i].velocity * step;
            }

            std::vector<Vector3> acc1 = computeAccelerations(objects);
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i].velocity += acc1[i] * (step / 2.0);
            }
            accumulator -= fixedTimestep;
        }
    }

    camera.update(dt);
}

void Scene::render() {
    camera.render(objects);
}