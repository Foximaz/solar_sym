#ifndef SCENE_H
#define SCENE_H

#include "object.h"
#include "rendering.h"
#include "ui.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Scene {
private:
    float MAX_ZOOM_SPEED = 0.7f;
    float ACCURACY_MULTIPLIER = 0.1f;
    sf::Vector2f HELP_PANEL_LINE_SIZE = sf::Vector2f(180, 25);
    sf::Vector2f HELP_PANEL_POS_SHIFT = sf::Vector2f(-235, -150);
    sf::Color HELP_PANEL_FONT_COLOR = sf::Color(220, 220, 220);

    const std::string& dictPath;
    sf::RenderWindow& window;
    sf::Font font;
    std::vector<Object> objects;
    std::unique_ptr<Camera> camera;
    Object* selectedObject;

    std::vector<std::unique_ptr<UIElement>> uiElements;
    TextBox* speedDisplay;
    Button* showNamesButton;
    InputField* nameField;
    InputField* posX, *posY, *posZ;
    InputField* velX, *velY, *velZ;
    InputField* massField;
    InputField* sizeField;
    InputField* colorR, *colorG, *colorB;

    double G;
    double softening;

    double simulationSpeed = 1.0;
    double fixedTimestep = 1.0 / 60.0;
    double accumulator = 0.0;
    bool pause = false;

    float pendingZoom = 0.0f;

    void setupHelpPanel();
    void setupEditPanel();
    void updateEditPanelFromObject();
    void applyEditToObject();
    void createObjectFromFields();
    void removeSelectedObject();
    void setInputFieldsEnabled(bool enabled);
    void setupUI();
    void updateSpeedDisplay();
    void updateShowNamesButtonText();
    std::vector<Vector3> computeAccelerations(const std::vector<Object>& objects) const;
    void reload();

public:
    Scene(
        sf::RenderWindow& window,
        const std::string& dictPath,
        double G,
        double softening
    );

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void processInput(float dt);
    void update(float dt);
    void draw();
};

#endif
