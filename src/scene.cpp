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
    , G(G)
    , softening(softening)
{
    setupUI();
    updateSpeedDisplay();
}

void Scene::setupUI() {
    uiElements.clear();
    
    auto speedDisplayPtr = std::make_unique<TextBox>(
        font,
        sf::Vector2f(330, 25),
        sf::Vector2f(90, 70),
        "",
        24,
        sf::Color::White,
        sf::Color(50, 50, 50),
        sf::Color(100, 100, 150),
        2.0f
    );
    speedDisplay = speedDisplayPtr.get();
    uiElements.push_back(std::move(speedDisplayPtr));
    
    auto slowerBtn = std::make_unique<Button>(
        font, "<",
        sf::Vector2f(30, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() {
            simulationSpeed *= 0.5;
            updateSpeedDisplay();
        }
    );
    uiElements.push_back(std::move(slowerBtn));
    
    auto pauseBtn = std::make_unique<Button>(
        font, "II",
        sf::Vector2f(130, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() {
            pause = !pause;
            updateSpeedDisplay();
        }
    );
    uiElements.push_back(std::move(pauseBtn));
    
    auto fasterBtn = std::make_unique<Button>(
        font, ">",
        sf::Vector2f(230, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() { 
            simulationSpeed = std::min(128.0, simulationSpeed * 2);
            updateSpeedDisplay();
        }
    );
    uiElements.push_back(std::move(fasterBtn));
    
    auto namesBtn = std::make_unique<Button>(
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
    showNamesButton = namesBtn.get();
    uiElements.push_back(std::move(namesBtn));
    updateShowNamesButtonText();
    
    auto reloadBtn = std::make_unique<Button>(
        font, "Reload",
        sf::Vector2f(window.getSize().x - 230, 25),
        sf::Vector2f(100, 70),
        25,
        [this]() { reload(); }
    );
    uiElements.push_back(std::move(reloadBtn));
    
    auto closeBtn = std::make_unique<Button>(
        font, "x",
        sf::Vector2f(window.getSize().x - 100, 25),
        sf::Vector2f(70, 70),
        40,
        [this]() { window.close(); },
        sf::Color(200, 60, 60)
    );
    uiElements.push_back(std::move(closeBtn));


    auto helpPanel = std::make_unique<CollapsiblePanel>(
        font,
        sf::Vector2f(window.getSize().x - 20, window.getSize().y - 20),
        sf::Vector2f(250, 230),
        "?", "x",
        PanelDirection::LeftUp,
        sf::Color(40, 40, 50),
        sf::Color(150, 150, 200),
        2.0f
    );

    auto line1 = std::make_unique<TextBox>(
        font,
        sf::Vector2f(window.getSize().x - 235, window.getSize().y - 150),
        sf::Vector2f(180, 25),
        "Mouse wheel - zoom",
        17,
        sf::Color(220, 220, 220),
        sf::Color::Transparent,
        sf::Color::Transparent,
        0.0f
    );
    helpPanel->addElement(std::move(line1));
    
    auto line2 = std::make_unique<TextBox>(
        font,
        sf::Vector2f(window.getSize().x - 235, window.getSize().y - 125),
        sf::Vector2f(180, 25),
        "Arrows - rotate",
        17,
        sf::Color(220, 220, 220),
        sf::Color::Transparent,
        sf::Color::Transparent,
        0.0f
    );
    helpPanel->addElement(std::move(line2));
    
    auto line3 = std::make_unique<TextBox>(
        font,
        sf::Vector2f(window.getSize().x - 235, window.getSize().y - 100),
        sf::Vector2f(180, 25),
        "Shift + ___ - fine movement",
        17,
        sf::Color(220, 220, 220),
        sf::Color::Transparent,
        sf::Color::Transparent,
        0.0f
    );
    helpPanel->addElement(std::move(line3));

    auto line4 = std::make_unique<TextBox>(
        font,
        sf::Vector2f(window.getSize().x - 235, window.getSize().y - 75),
        sf::Vector2f(180, 25),
        "Click - select object",
        17,
        sf::Color(220, 220, 220),
        sf::Color::Transparent,
        sf::Color::Transparent,
        0.0f
    );
    helpPanel->addElement(std::move(line4));

    
    helpPanel->setCollapsed(true);
    uiElements.push_back(std::move(helpPanel));
}

void Scene::updateShowNamesButtonText() {
    if (showNamesButton) {
        if (camera->getShowNames()) {
            showNamesButton->setText("  Hide\nnames");
        } else {
            showNamesButton->setText(" Show\nnames");
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
    
    // if (event.type == sf::Event::KeyPressed) {
    //     switch (event.key.code) {
    //         case sf::Keyboard::Num1:
    //             simulationSpeed = 1.0;
    //             updateSpeedDisplay();
    //             break;
    //         case sf::Keyboard::Num2:
    //             simulationSpeed = std::min(128.0, simulationSpeed * 2);
    //             updateSpeedDisplay();
    //             break;
    //         case sf::Keyboard::Num3:
    //             simulationSpeed = simulationSpeed * 0.5;
    //             updateSpeedDisplay();
    //             break;
    //         case sf::Keyboard::Space:
    //             pause = !pause;
    //             updateSpeedDisplay();
    //             break;
    //         default:
    //             break;
    //     }
    // }
    
    for (auto& uiElement : uiElements) {
        uiElement->handleEvent(event, window);
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
    speedDisplay->setText(speedText);
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
    for (auto& uiElement : uiElements) {
        uiElement->update(dt);
    }
}

void Scene::render() {
    window.clear(sf::Color::Black);
    camera->render(objects);
    for (auto& uiElement : uiElements) {
        uiElement->draw(window);
    }
    window.display();
}
