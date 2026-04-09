#include "scene.h"
#include "load.h"
#include "ui.h"
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream> //DEBUG

Scene::Scene(
    sf::RenderWindow& window,
    const std::string& dictPath,
    double G,
    double softening
)
    : window(window)
    , dictPath(dictPath)
    , font(load_ui_config((std::filesystem::path(dictPath) / "ui_config.json").string()))
    , objects(load_obj_config((std::filesystem::path(dictPath) / "obj_config.json").string()))
    , camera(std::make_unique<Camera>(load_cam_config(window, objects, (std::filesystem::path(dictPath) / "cam_config.json").string())))
    , speedDisplay(
        font,
        sf::Vector2f(330, 25),
        sf::Vector2f(90, 70),
        24,
        sf::Color(30, 30, 40),
        sf::Color::White,
        sf::Color(100, 100, 150),
        2.0f
    )
    , G(G)
    , softening(softening)
{
    initButtons();
}

void Scene::initButtons() {
    buttons.clear();
    buttons.reserve(6);
    
    buttons.emplace_back(
        font, "<",
        sf::Vector2f(30, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() {
            simulationSpeed *= 0.5;
            updateSpeedDisplay();
        }
    );
    
    buttons.emplace_back(
        font, "II",
        sf::Vector2f(130, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() {
            pause = !pause;
            updateSpeedDisplay();
        }
    );
    
    buttons.emplace_back(
        font, ">",
        sf::Vector2f(230, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() { 
            simulationSpeed = std::min(128.0, simulationSpeed * 2);
            updateSpeedDisplay();
        }
    );
    
    buttons.emplace_back(
        font,
        " Hide\nnames",
        sf::Vector2f(window.getSize().x / 2, 25),
        sf::Vector2f(100, 70),
        20,
        [this]() {
            camera->switchShowNames();
            updateShowNamesButtonText();
        }
    );
    showNamesButton = &buttons.back();
    updateShowNamesButtonText();

    buttons.emplace_back(
        font, "Reload",
        sf::Vector2f(window.getSize().x - 230, 25),
        sf::Vector2f(100, 70),
        25,
        [this]() { reload(); }
    );
    
    buttons.emplace_back(
        font, "x",
        sf::Vector2f(window.getSize().x - 100, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() { window.close(); },
        sf::Color(200, 60, 60)
    );
}

void Scene::updateShowNamesButtonText() {
    if (showNamesButton) {
        if (camera->getShowNames()) {
            showNamesButton->setText("  Hide\nnames");
        } else {
            showNamesButton->setText("  Show\nnames");
        }
    }
}

void Scene::reload() {
    objects = load_obj_config((std::filesystem::path(dictPath) / "obj_config.json").string());
    camera = std::make_unique<Camera>(load_cam_config(window, objects, (std::filesystem::path(dictPath) / "cam_config.json").string()));
    updateSpeedDisplay();
}

void Scene::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Closed) {
        window.close();
    }
    
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            pendingZoom += event.mouseWheelScroll.delta;
        }
    }
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Object* clicked = nullptr;
        if (camera->handleMouseClick(mousePos, clicked)) {
            camera->setTarget(clicked);
        }
    }
    
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Num1:
                simulationSpeed = 1.0;
                updateSpeedDisplay();
                break;
            case sf::Keyboard::Num2:
                simulationSpeed = std::min(128.0, simulationSpeed * 2);
                updateSpeedDisplay();
                break;
            case sf::Keyboard::Num3:
                simulationSpeed = simulationSpeed * 0.5;
                updateSpeedDisplay();
                break;
            case sf::Keyboard::Space:
                pause = !pause;
                updateSpeedDisplay();
                break;
            default:
                break;
        }
    }
    
    for (auto& button : buttons) {
        button.handleEvent(event, window);
    }
}

void Scene::processInput(float dt) {
    short dYaw = 0;
    short dPitch = 0;
    short dDistance = 0;
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) dYaw += 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dYaw -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) dPitch -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) dPitch += 1;
    
    double multiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.1 : 1.0;
    
    camera->processInput(dt, dYaw, dPitch, pendingZoom, multiplier);
    pendingZoom = 0.0f;
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

void Scene::updateSpeedDisplay() {
    std::string speedText;
    if (pause) {
        speedText = "x0";
    } else {
        char buffer[32];
        if (simulationSpeed == 1.0) {
            speedText = "x1";
        } else if (simulationSpeed < 1.0) {
            snprintf(buffer, sizeof(buffer), "x1/%.0f", 1.0 / simulationSpeed);
            speedText = buffer;
        } else if (simulationSpeed == static_cast<int>(simulationSpeed)) {
            snprintf(buffer, sizeof(buffer), "x%d", static_cast<int>(simulationSpeed));
            speedText = buffer;
        } else {
            snprintf(buffer, sizeof(buffer), "x%.2f", simulationSpeed);
            speedText = buffer;
        }
    }
    speedDisplay.setText(speedText);
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

    camera->update(dt);
    for (auto& button : buttons) {
        button.update();
    }
}

void Scene::render() {
    window.clear(sf::Color::Black);
    camera->render(objects);
    for (auto& button : buttons) {
        button.render(window);
    }
    speedDisplay.draw(window);
    window.display();
}
