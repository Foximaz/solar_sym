#ifndef LOAD_H
#define LOAD_H

#include "utils.h"
#include "rendering.h"
#include "scene.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <string>

using json = nlohmann::json;

Vector3 vector3FromJson(const json& j);
sf::Color colorFromJson(const json& j);

std::vector<Object> load_obj_config(const std::string& filePath);
Camera load_cam_config(sf::RenderWindow& window, std::vector<Object>& objects, const std::string& filePath);

sf::Font load_ui_config(const std::string& filePath);

#endif