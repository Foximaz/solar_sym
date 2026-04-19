#include "scene.h"
#include "load.h"
#include "ui.h"
#include <string>
#include <filesystem>
#include <algorithm>
#include <cmath>

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
    , selectedObject(&objects[0])
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
            setInputFieldsEnabled(pause);
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
            camera->setShowNames(!camera->getShowNames());
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

    setupHelpPanel();
    setupEditPanel();    
}

void Scene::setupHelpPanel() {
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

void Scene::setupEditPanel() {
    auto panel = std::make_unique<CollapsiblePanel>(
        font,
        sf::Vector2f(20, window.getSize().y - 20),
        sf::Vector2f(300, 340),
        "Exp", "V",
        PanelDirection::RightUp,
        sf::Color(50, 50, 70),
        sf::Color(200, 200, 250),
        2.0f
    );

    float baseX = 30;
    float baseY = window.getSize().y - 260;
    float labelX = baseX;
    float fieldX = baseX + 70;
    float fieldW = 200;
    float fieldH = 25;
    float rowHeight = 30;
    
    float currentY = baseY;
    
    // Name
    auto nameLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Name", 14, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(nameLabel));
    
    auto nameInput = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(fieldW - 10, fieldH),
        16, false, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    nameField = nameInput.get();
    panel->addElement(std::move(nameInput));
    currentY += rowHeight;
    
    // Position
    auto posLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Position", 16, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(posLabel));
    
    auto posXin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    posX = posXin.get();
    panel->addElement(std::move(posXin));
    
    auto posYin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 65, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    posY = posYin.get();
    panel->addElement(std::move(posYin));
    
    auto posZin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 130, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    posZ = posZin.get();
    panel->addElement(std::move(posZin));
    currentY += rowHeight;
    
    // Velocity
    auto velLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Velocity", 16, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(velLabel));
    
    auto velXin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    velX = velXin.get();
    panel->addElement(std::move(velXin));
    
    auto velYin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 65, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    velY = velYin.get();
    panel->addElement(std::move(velYin));
    
    auto velZin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 130, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    velZ = velZin.get();
    panel->addElement(std::move(velZin));
    currentY += rowHeight;
    
    // Mass
    auto massLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Mass", 16, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(massLabel));
    
    auto massInput = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(fieldW - 10, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    massField = massInput.get();
    panel->addElement(std::move(massInput));
    currentY += rowHeight;
    
    // Size
    auto sizeLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Size", 16, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(sizeLabel));
    
    auto sizeInput = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(fieldW - 10, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    sizeField = sizeInput.get();
    panel->addElement(std::move(sizeInput));
    currentY += rowHeight;
    
    // Color
    auto colorLabel = std::make_unique<TextBox>(
        font, sf::Vector2f(labelX, currentY), sf::Vector2f(70, fieldH),
        "Color", 16, sf::Color::White, sf::Color::Transparent,
        sf::Color::Transparent, 0
    );
    panel->addElement(std::move(colorLabel));
    
    auto colorRin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    colorR = colorRin.get();
    panel->addElement(std::move(colorRin));
    
    auto colorGin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 65, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    colorG = colorGin.get();
    panel->addElement(std::move(colorGin));
    
    auto colorBin = std::make_unique<InputField>(
        font, sf::Vector2f(fieldX + 130, currentY), sf::Vector2f(60, fieldH),
        14, true, sf::Color(60, 60, 80), sf::Color::White, sf::Color::Black, 2
    );
    colorB = colorBin.get();
    panel->addElement(std::move(colorBin));
    currentY += rowHeight + 10;
    
    auto createBtn = std::make_unique<Button>(
        font, "New", sf::Vector2f(labelX, currentY), sf::Vector2f(80, 35), 18,
        [this]() { createObjectFromFields(); }
    );
    panel->addElement(std::move(createBtn));
    
    auto editBtn = std::make_unique<Button>(
        font, "Save", sf::Vector2f(labelX + 90, currentY), sf::Vector2f(80, 35), 18,
        [this]() { applyEditToObject(); }
    );
    panel->addElement(std::move(editBtn));
    
    auto removeBtn = std::make_unique<Button>(
        font, "Remove", sf::Vector2f(labelX + 180, currentY), sf::Vector2f(90, 35), 18,
        [this]() { removeSelectedObject(); }
    );
    panel->addElement(std::move(removeBtn));
    
    uiElements.push_back(std::move(panel));
    
    setInputFieldsEnabled(false);
}

void Scene::updateEditPanelFromObject() {
    if (!selectedObject) {
        nameField->setText("");
        posX->setText(""); posY->setText(""); posZ->setText("");
        velX->setText(""); velY->setText(""); velZ->setText("");
        massField->setText("");
        sizeField->setText("");
        colorR->setText(""); colorG->setText(""); colorB->setText("");
        return;
    }
    nameField->setText(selectedObject->name);
    posX->setText(std::to_string(selectedObject->position.x));
    posY->setText(std::to_string(selectedObject->position.y));
    posZ->setText(std::to_string(selectedObject->position.z));
    velX->setText(std::to_string(selectedObject->velocity.x));
    velY->setText(std::to_string(selectedObject->velocity.y));
    velZ->setText(std::to_string(selectedObject->velocity.z));
    massField->setText(std::to_string(selectedObject->mass));
    sizeField->setText(std::to_string(selectedObject->size));
    colorR->setText(std::to_string(selectedObject->color.r));
    colorG->setText(std::to_string(selectedObject->color.g));
    colorB->setText(std::to_string(selectedObject->color.b));
}

void Scene::applyEditToObject() {
    if (!selectedObject || !pause) return;
    try {
        selectedObject->name = nameField->getText();
        selectedObject->position.x = std::stod(posX->getText());
        selectedObject->position.y = std::stod(posY->getText());
        selectedObject->position.z = std::stod(posZ->getText());
        selectedObject->velocity.x = std::stod(velX->getText());
        selectedObject->velocity.y = std::stod(velY->getText());
        selectedObject->velocity.z = std::stod(velZ->getText());
        selectedObject->mass = std::stod(massField->getText());
        selectedObject->size = std::stod(sizeField->getText());
        selectedObject->color.r = std::stoi(colorR->getText());
        selectedObject->color.g = std::stoi(colorG->getText());
        selectedObject->color.b = std::stoi(colorB->getText());
    } catch (...) {}
}

void Scene::createObjectFromFields() {
    if (!pause) return;
    
    std::string name = nameField->getText();
    if (name.empty()) name = "New";
    
    Vector3 position;
    Vector3 velocity;
    double mass = 1.0;
    double size = 0.1;
    sf::Color color = sf::Color::White;
    
    try {
        position = Vector3(
            std::stod(posX->getText()),
            std::stod(posY->getText()),
            std::stod(posZ->getText())
        );
    } catch (...) {
        position = Vector3(0, 0, 0);
    }
    
    try {
        velocity = Vector3(
            std::stod(velX->getText()),
            std::stod(velY->getText()),
            std::stod(velZ->getText())
        );
    } catch (...) {
        velocity = Vector3(0, 0, 0);
    }
    
    try {
        mass = std::stod(massField->getText());
    } catch (...) {
        mass = 1.0;
    }
    
    try {
        size = std::stod(sizeField->getText());
    } catch (...) {
        size = 0.1;
    }
    
    try {
        color = sf::Color(
            std::stoi(colorR->getText()),
            std::stoi(colorG->getText()),
            std::stoi(colorB->getText())
        );
    } catch (...) {
        color = sf::Color::White;
    }
    
    Object newObj(position, velocity, mass, size, color, name);
    objects.push_back(newObj);
    
    selectedObject = &objects.back();
    camera->setTarget(selectedObject);
    updateEditPanelFromObject();
}

void Scene::removeSelectedObject() {
    if (!selectedObject || !pause) return;
    auto it = std::find_if(objects.begin(), objects.end(),
        [this](const Object& obj) { return &obj == selectedObject; });
    if (it != objects.end()) {
        objects.erase(it);
        if (objects.empty()) {
            selectedObject = nullptr;
        } else {
            selectedObject = &objects[0];
            camera->setTarget(selectedObject);
        }
        updateEditPanelFromObject();
    }
}

void Scene::setInputFieldsEnabled(bool enabled) {
    if (nameField) nameField->setEnabled(enabled);
    if (posX) posX->setEnabled(enabled);
    if (posY) posY->setEnabled(enabled);
    if (posZ) posZ->setEnabled(enabled);
    if (velX) velX->setEnabled(enabled);
    if (velY) velY->setEnabled(enabled);
    if (velZ) velZ->setEnabled(enabled);
    if (massField) massField->setEnabled(enabled);
    if (sizeField) sizeField->setEnabled(enabled);
    if (colorR) colorR->setEnabled(enabled);
    if (colorG) colorG->setEnabled(enabled);
    if (colorB) colorB->setEnabled(enabled);
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
    selectedObject = &objects[0];
    camera = std::make_unique<Camera>(load_cam_config(window, objects, (std::filesystem::path(dictPath) / "cam_config.json").string()));
    updateSpeedDisplay();
}

void Scene::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::Closed) {
        window.close();
    }
    
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            if (std::abs(event.mouseWheelScroll.delta) < MAX_ZOOM_SPEED)
                pendingZoom += event.mouseWheelScroll.delta;
            else
                pendingZoom += std::copysign(MAX_ZOOM_SPEED, event.mouseWheelScroll.delta);
        }
    }
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Object* clicked = nullptr;
        if (camera->handleMouseClick(mousePos, clicked)) {
            camera->setTarget(clicked);
            selectedObject = clicked;
            updateEditPanelFromObject();
        }
    }
    
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
    
    double multiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? ACCURACY_MULTIPLIER : 1.0;
    
    camera->processInput(dt, dYaw, dPitch, pendingZoom, multiplier);
    pendingZoom = 0.0f;
}

std::vector<Vector3> Scene::computeAccelerations(const std::vector<Object>& objects) const {
    // Newton's law of universal gravitation: F = G * M * m / r^2
    // Acceleration of object i: a_i = F / m_i = G * sum_{j!=i} (m_j / r_ij^3) * r_ji
    // To avoid division by zero when objects get too close, we add a softening parameter
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
    // Velocity Verlet integration scheme (second order accurate, symplectic)
    // Good energy conservation for gravitational systems
    // Two acceleration calculations per time step
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
            // 1. Compute acceleration at beginning of the step
            std::vector<Vector3> acc0 = computeAccelerations(objects);

            // 2. Half-step velocity update (v_{n+1/2} = v_n + a_n * Δt/2)
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i].velocity += acc0[i] * (step / 2.0);
            }

            // 3. Full-step position update (x_{n+1} = x_n + v_{n+1/2} * Δt
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i].position += objects[i].velocity * step;
            }

            // 4. Compute acceleration at the new position
            std::vector<Vector3> acc1 = computeAccelerations(objects);

            // 5. Second half-step velocity update (v_{n+1} = v_{n+1/2} + a_{n+1} * Δt/2)
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

    if (!pause && selectedObject) {
        updateEditPanelFromObject();
    }
}

void Scene::draw() {
    window.clear(sf::Color::Black);
    camera->render(objects);
    for (auto& uiElement : uiElements) {
        uiElement->draw(window);
    }
    window.display();
}
