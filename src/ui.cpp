#include "ui.h"
#include <SFML/Graphics.hpp>
#include <string>

sf::Uint8 clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<sf::Uint8>(value);
}

sf::Color Button::computeHoverColor(const sf::Color& base) const {
    return sf::Color(
        clamp(base.r + (255 - base.r) * 0.2),
        clamp(base.g + (255 - base.g) * 0.2),
        clamp(base.b + (255 - base.b) * 0.2),
        base.a
    );
}

sf::Color Button::computePressedColor(const sf::Color& base) const {
    return sf::Color(
        clamp(base.r * 0.7),
        clamp(base.g * 0.7),
        clamp(base.b * 0.7),
        base.a
    );
}

Button::Button(
    sf::Font& font,
    const std::string& buttonText,
    sf::Vector2f position,
    sf::Vector2f size,
    unsigned int characterSize,
    std::function<void()> callback,
    const sf::Color& normalColor,
    const sf::Color& textColor,
    const sf::Color& outlineColor
)
: font(font)
, onClick(callback)
, normalColor(normalColor)
, hoverColor(computeHoverColor(normalColor))
, pressedColor(computePressedColor(normalColor))
, textColor(textColor)
, outlineColor(outlineColor)
, isHovered(false)
, isPressed(false)
, characterSize(characterSize)
{
    shape.setPosition(position);
    shape.setSize(size);
    shape.setFillColor(normalColor);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(outlineColor);

    text.setFont(font);
    text.setString(buttonText);
    text.setCharacterSize(characterSize);
    text.setFillColor(textColor);

    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width/2.0f,
        textBounds.top  + textBounds.height/2.0f);
    text.setPosition(position + size/2.0f);
}

void Button::setNormalColor(const sf::Color& color) {
    normalColor = color;
    hoverColor = computeHoverColor(color);
    pressedColor = computePressedColor(color);

    if (!isPressed && !isHovered) {
        shape.setFillColor(normalColor);
    }
}

void Button::setText(const std::string& newText) {
    text.setString(newText);
    
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f,
                   bounds.top + bounds.height / 2.0f);
    
    text.setPosition(shape.getPosition() + shape.getSize() / 2.0f);
}

void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

    bool contains = shape.getGlobalBounds().contains(worldPos);

    if (event.type == sf::Event::MouseMoved) {
        isHovered = contains;
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left && contains) {
            isPressed = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isPressed && contains && onClick) {
                onClick();
            }
            isPressed = false;
        }
    }
}

void Button::update() {
    if (isPressed) {
        shape.setFillColor(pressedColor);
    } else if (isHovered) {
        shape.setFillColor(hoverColor);
    } else {
        shape.setFillColor(normalColor);
    }
}

void Button::render(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}


TextBox::TextBox(
    sf::Font& font,
    sf::Vector2f position,
    sf::Vector2f size,
    unsigned int characterSize,
    sf::Color backgroundColor,
    sf::Color textColor,
    sf::Color outlineColor,
    float outlineThickness
) : font(font)
  , backgroundColor(backgroundColor)
  , textColor(textColor)
  , outlineColor(outlineColor)
  , outlineThickness(outlineThickness)
{
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(backgroundColor);
    background.setOutlineThickness(outlineThickness);
    background.setOutlineColor(outlineColor);
    
    text.setFont(font);
    text.setCharacterSize(characterSize);
    text.setFillColor(textColor);
    text.setString("x1");
    
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(
        textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f
    );
    text.setPosition(position + size / 2.0f);
}

void TextBox::setText(const std::string& str) {
    text.setString(str);
    
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(
        textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f
    );
    text.setPosition(background.getPosition() + background.getSize() / 2.0f);
}

void TextBox::setText(const sf::String& str) {
    text.setString(str);
    
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(
        textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f
    );
    text.setPosition(background.getPosition() + background.getSize() / 2.0f);
}

void TextBox::setPosition(const sf::Vector2f& position) {
    background.setPosition(position);
    text.setPosition(position + background.getSize() / 2.0f);
}

void TextBox::setSize(const sf::Vector2f& size) {
    background.setSize(size);
    text.setPosition(background.getPosition() + size / 2.0f);
}

void TextBox::setBackgroundColor(const sf::Color& color) {
    backgroundColor = color;
    background.setFillColor(color);
}

void TextBox::setTextColor(const sf::Color& color) {
    textColor = color;
    text.setFillColor(color);
}

void TextBox::draw(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(text);
}
