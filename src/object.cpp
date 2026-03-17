#include "object.h"
#include "utils.h"
#include <string>

Object::Object(Vector3 pos, Vector3 vel, float mass, float size, sf::Color color, std::string name)
    : position(pos), velocity(vel), mass(mass), size(size), color(color), name(name) {}
