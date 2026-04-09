#ifndef UI_H
#define UI_H

#include <functional>
#include <SFML/Graphics.hpp>

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Font& font;
    unsigned int characterSize;
    std::function<void()> onClick;
    
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;
    sf::Color textColor;
    sf::Color outlineColor;
    
    bool isHovered;
    bool isPressed;

    sf::Color computeHoverColor(const sf::Color& base) const;
    sf::Color computePressedColor(const sf::Color& base) const;
    
public:
    Button(
        sf::Font& font,
        const std::string& buttonText,
        sf::Vector2f position,
        sf::Vector2f size,
        unsigned int characterSize,
        std::function<void()> callback,
        const sf::Color& normalColor = sf::Color(120, 120, 240),
        const sf::Color& textColor = sf::Color::White,
        const sf::Color& outlineColor = sf::Color::Black
    );
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update();
    void render(sf::RenderWindow& window);
    
    void setPosition(sf::Vector2f pos);
    void setText(const std::string& newText);
    void setNormalColor(const sf::Color& color);
};


class TextBox {
private:
    sf::RectangleShape background;
    sf::Text text;
    sf::Font& font;
    
    sf::Color backgroundColor;
    sf::Color textColor;
    sf::Color outlineColor;
    
    float outlineThickness;
    
public:
    TextBox(
        sf::Font& font,
        sf::Vector2f position,
        sf::Vector2f size,
        unsigned int characterSize = 20,
        sf::Color backgroundColor = sf::Color(50, 50, 50),
        sf::Color textColor = sf::Color::White,
        sf::Color outlineColor = sf::Color::Black,
        float outlineThickness = 2.0f
    );
    
    void setText(const std::string& str);
    void setText(const sf::String& str);
    
    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    
    void setBackgroundColor(const sf::Color& color);
    void setTextColor(const sf::Color& color);
    
    void draw(sf::RenderWindow& window);
};

#endif