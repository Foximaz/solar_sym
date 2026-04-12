#include "scene.h"
#include <SFML/Graphics.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode(1500, 900), "Solar simulation");
    window.setFramerateLimit(60);
    
    std::string dictPath = "./configs/";
    
    if (argc >= 2)
        dictPath = argv[1];
    std::cout << "Loading from: " << dictPath << std::endl;
    
    const double G_real = 6.6743e-11;

    const double L = 1.496e11; // 1 a.u. in meters
    const double M = 5.972e24; // 1 Earth mass in kg
    const double T = 24 * 3600; // 1 day in seconds

    double G_sim = G_real * (M * T * T) / (L * L * L);

    Scene scene(window, dictPath, G_sim, 0.00000001);

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        sf::Event event;
        while (window.pollEvent(event)) { scene.handleEvent(event, window); }
        scene.processInput(dt);
        scene.update(dt);
        scene.draw();
    }
    return 0;
}
