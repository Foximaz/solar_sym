#ifndef OBJECT_H
#define OBJECT_H

#include "utils.h"
#include <SFML/Graphics.hpp>

class Object {
public:
    Vector3 position;
    Vector3 velocity;
    float mass;
    float size;
    sf::Color color;
    std::string name;

    Object(Vector3 pos, Vector3 vel, float mass, float size, sf::Color color, std::string name);
};

#endif