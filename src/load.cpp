#include "utils.h"
#include "load.h"
#include "scene.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

Vector3 vector3FromJson(const json& j) {
    return Vector3(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
}

sf::Color colorFromJson(const json& j) {
    return sf::Color(j[0].get<int>(), j[1].get<int>(), j[2].get<int>());
}

sf::Font fontFromJson(const json& j) {
    std::string path = j.get<std::string>();
    sf::Font font;
    if (!font.loadFromFile(path))
        throw std::runtime_error("Cannot load font: " + path);
    return font;
}

sf::Font load_ui_config(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    json data;
    file >> data;

    return fontFromJson(data["fontPath"]);
}

std::vector<Object> load_obj_config(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    json data;
    file >> data;

    std::vector<Object> objects;
    for (const auto& item : data) {
        objects.emplace_back(
            vector3FromJson(item["position"]),
            vector3FromJson(item["velocity"]),
            item["mass"].get<double>(),
            item["size"].get<double>(),
            colorFromJson(item["color"]),
            item["name"].get<std::string>()
        );
    }
    return objects;
}

Camera load_cam_config(sf::RenderWindow& window, std::vector<Object>& objects, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    json data;
    file >> data;

    return Camera(
        window,
        &objects[0],
        data["yaw"].get<float>() * M_PI / 180.0,
        - data["pitch"].get<float>() * M_PI / 180.0,
        data["distance"].get<float>(),
        data["fov"].get<float>(),
        data["rotationSpeed"].get<float>(),
        data["zoomSpeed"].get<float>(),
        data["smoothness"].get<float>(),
        data["showNames"].get<bool>(),
        fontFromJson(data["fontPath"]),
        data["characterSize"].get<unsigned int>(),
        data["minShow"].get<float>(),
        data["iconSize"].get<float>()
    );
}
