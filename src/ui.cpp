#include "ui.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

void UIElement::setVisible(bool visible) { isVisible = visible; }

bool UIElement::getVisible() const { return isVisible; }

sf::Uint8 clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<sf::Uint8>(value);
}

sf::Color shiftSaturation(const sf::Color& base, const float factor) {
    return sf::Color(
        clamp(base.r + (255 - base.r) * factor),
        clamp(base.g + (255 - base.g) * factor),
        clamp(base.b + (255 - base.b) * factor),
        base.a
    );
}

sf::Color shiftBrightness(const sf::Color& base, const float factor) {
    return sf::Color(
        clamp(base.r * factor),
        clamp(base.g * factor),
        clamp(base.b * factor),
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
, hoverColor(shiftSaturation(normalColor, 0.2f))
, pressedColor(shiftBrightness(normalColor, 0.7f))
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

void Button::update(float dt) {
    if (isPressed) {
        shape.setFillColor(pressedColor);
    } else if (isHovered) {
        shape.setFillColor(hoverColor);
    } else {
        shape.setFillColor(normalColor);
    }
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}

void Button::setPosition(const sf::Vector2f& position) {
    sf::Vector2f roundedPos(std::round(position.x), std::round(position.y));
    
    shape.setPosition(roundedPos);
    
    sf::Vector2f textPos = roundedPos + shape.getSize() / 2.0f;
    textPos.x = std::round(textPos.x);
    textPos.y = std::round(textPos.y);
    
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f,
                   bounds.top + bounds.height / 2.0f);
    text.setPosition(textPos);
}

sf::Vector2f Button::getPosition() const {
    return shape.getPosition();
}


TextBox::TextBox(
    sf::Font& font,
    sf::Vector2f position,
    sf::Vector2f size,
    std::string text,
    unsigned int characterSize,
    sf::Color textColor,
    sf::Color bgColor,
    sf::Color outlineColor,
    float outlineThickness
)
    : font(font)
    , bgColor(bgColor)
    , textColor(textColor)
    , outlineColor(outlineColor)
    , outlineThickness(outlineThickness)
    , drawBackground(bgColor != sf::Color::Transparent)
    , drawOutline(outlineThickness > 0.0f)
{
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(bgColor);
    background.setOutlineThickness(outlineThickness);
    background.setOutlineColor(outlineColor);
    
    this->text.setFont(font);
    this->text.setCharacterSize(characterSize);
    this->text.setFillColor(textColor);
    this->text.setString(text);
    
    setPosition(position);
}

void TextBox::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {}

void TextBox::update(float dt) {}

void TextBox::draw(sf::RenderWindow& window) {
    if (drawBackground) {
        window.draw(background);
    }
    window.draw(text);
}

void TextBox::setPosition(const sf::Vector2f& newPosition) {
    sf::Vector2f roundedPos(std::round(newPosition.x), std::round(newPosition.y));
    
    background.setPosition(roundedPos);
    
    sf::Vector2f textPos = roundedPos + background.getSize() / 2.0f;
    textPos.x = std::round(textPos.x);
    textPos.y = std::round(textPos.y);
    
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f,
                   bounds.top + bounds.height / 2.0f);
    text.setPosition(textPos);
}

sf::Vector2f TextBox::getPosition() const {
    return background.getPosition();
}

void TextBox::setText(const std::string& newText) {
    text.setString(newText);
    
    sf::Vector2f bgPos = background.getPosition();
    sf::Vector2f textPos = bgPos + background.getSize() / 2.0f;
    textPos.x = std::round(textPos.x);
    textPos.y = std::round(textPos.y);
    
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f,
                   bounds.top + bounds.height / 2.0f);
    text.setPosition(textPos);
}


InputField::InputField(
    sf::Font& font,
    sf::Vector2f position,
    sf::Vector2f size,
    unsigned int characterSize,
    bool numericOnly,
    sf::Color bgColor,
    sf::Color textColor,
    sf::Color outlineColor,
    float outlineThickness
)
    : font(font), bgColor(bgColor), textColor(textColor),
      outlineColor(outlineColor), outlineThickness(outlineThickness),
      numericOnly(numericOnly), enabled(true), hasFocus(false), showCursor(false)
{
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(bgColor);
    background.setOutlineThickness(outlineThickness);
    background.setOutlineColor(outlineColor);

    text.setFont(font);
    text.setCharacterSize(characterSize);
    text.setFillColor(textColor);
    text.setString("");
    updateTextPosition();
}

void InputField::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!enabled) return;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        bool contains = background.getGlobalBounds().contains(mousePos);
        hasFocus = contains;
        showCursor = false;
    }

    if (hasFocus && event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8) {
            std::string str = text.getString();
            if (!str.empty()) str.pop_back();
            text.setString(str);
        } else if (event.text.unicode == 13) {
            hasFocus = false;
        } else if (event.text.unicode >= 32 && event.text.unicode < 128) {
            char c = static_cast<char>(event.text.unicode);
            if (numericOnly) {
                if (c == '.' || c == '-' || (c >= '0' && c <= '9')) {
                    text.setString(text.getString() + c);
                }
            } else {
                text.setString(text.getString() + c);
            }
        }
        clampText();
    }
}

void InputField::update(float dt) {
    if (hasFocus && enabled) {
        blinkAccumulator += dt;
        if (blinkAccumulator >= 0.5f) {
            showCursor = !showCursor;
            blinkAccumulator = 0.0f;
        }
    } else {
        showCursor = false;
        blinkAccumulator = 0.0f;
    }
}

void InputField::draw(sf::RenderWindow& window) {
    window.draw(background);

    sf::Text displayText = text;
    if (hasFocus && showCursor) {
        std::string str = text.getString();
        str += '|';
        displayText.setString(str);
    }
    window.draw(displayText);
}

void InputField::setPosition(const sf::Vector2f& position) {
    sf::Vector2f roundedPos(std::round(position.x), std::round(position.y));
    
    background.setPosition(roundedPos);
    updateTextPosition();
}

sf::Vector2f InputField::getPosition() const {
    return background.getPosition();
}

void InputField::setText(const std::string& s) {
    text.setString(s);
    clampText();
    updateTextPosition();
}

std::string InputField::getText() const {
    return text.getString();
}

void InputField::setEnabled(bool enabled) {
    enabled = enabled;
    if (!enabled) hasFocus = false;
    if (enabled) {
        background.setFillColor(bgColor);
        text.setFillColor(textColor);
    } else {
        background.setFillColor(sf::Color(80, 80, 80));
        text.setFillColor(sf::Color(150, 150, 150));
    }
}

void InputField::updateTextPosition() {
    sf::Vector2f bgPos = background.getPosition();
    
    sf::FloatRect textBounds = text.getLocalBounds();
    
    float textX = bgPos.x + 5;
    float textY = bgPos.y + background.getSize().y / 2.0f - textBounds.height;
    
    textX = std::round(textX);
    textY = std::round(textY);
    
    text.setPosition(textX, textY);
}

void InputField::clampText() {
    sf::FloatRect textBounds = text.getLocalBounds();
    float maxWidth = background.getSize().x - 10;
    while (textBounds.width > maxWidth && text.getString().getSize() > 0) {
        std::string str = text.getString();
        str.pop_back();
        text.setString(str);
        textBounds = text.getLocalBounds();
    }
}

bool InputField::isEnabled() const { return enabled; }


CollapsiblePanel::CollapsiblePanel(
    sf::Font& font,
    sf::Vector2f anchorPoint,
    sf::Vector2f expandedSize,
    const std::string& collapsedText,
    const std::string& expandedText,
    PanelDirection direction,
    sf::Color bgColor,
    sf::Color outlineColor,
    float outlineThickness
)
    : font(font)
    , anchorPoint(anchorPoint)
    , expandedSize(expandedSize)
    , collapsedText(collapsedText)
    , expandedText(expandedText)
    , direction(direction)
    , bgColor(bgColor)
    , outlineColor(outlineColor)
    , outlineThickness(outlineThickness)
    , collapsed(false)
    , toggleButton(
        font,
        expandedText,
        sf::Vector2f(0, 0),
        sf::Vector2f(70, 70),
        30,
        [this]() { toggleCollapse(); }
    )
{
    background.setPosition(getExpandedPosition());
    background.setSize(expandedSize);
    background.setFillColor(bgColor);
    background.setOutlineThickness(outlineThickness);
    background.setOutlineColor(outlineColor);
    
    updateButtonPosition();
}

sf::Vector2f CollapsiblePanel::getButtonOffset() const {
    if (collapsed) {
        switch (direction) {
            case PanelDirection::RightDown:
                return sf::Vector2f(0, 0);
            case PanelDirection::RightUp:
                return sf::Vector2f(0, -70);
            case PanelDirection::LeftDown:
                return sf::Vector2f(-70, 0);
            case PanelDirection::LeftUp:
                return sf::Vector2f(-70, -70);
        }
        return sf::Vector2f(-70, -70);
    }
    
    switch (direction) {
        case PanelDirection::RightDown:
            return sf::Vector2f(expandedSize.x - 80, expandedSize.y - 80);
        case PanelDirection::RightUp:
            return sf::Vector2f(expandedSize.x - 80, 10);
        case PanelDirection::LeftDown:
            return sf::Vector2f(10, expandedSize.y - 80);
        case PanelDirection::LeftUp:
            return sf::Vector2f(10, 10);
    }
    return sf::Vector2f(expandedSize.x - 80, 10);
}

sf::Vector2f CollapsiblePanel::getExpandedPosition() const {
    switch (direction) {
        case PanelDirection::RightDown:
            return anchorPoint;
        case PanelDirection::RightUp:
            return sf::Vector2f(anchorPoint.x, anchorPoint.y - expandedSize.y);
        case PanelDirection::LeftDown:
            return sf::Vector2f(anchorPoint.x - expandedSize.x, anchorPoint.y);
        case PanelDirection::LeftUp:
            return sf::Vector2f(anchorPoint.x - expandedSize.x, anchorPoint.y - expandedSize.y);
    }
    return anchorPoint;
}

sf::Vector2f CollapsiblePanel::getButtonPosition() const {
    if (collapsed) {
        return anchorPoint + getButtonOffset();
    } else {
        return getExpandedPosition() + getButtonOffset();
    }
}

void CollapsiblePanel::updateButtonPosition() {
    toggleButton.setPosition(getButtonPosition());
}

void CollapsiblePanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    toggleButton.handleEvent(event, window);
    
    if (!collapsed) {
        for (auto& elem : elements) {
            elem->handleEvent(event, window);
        }
    }
}

void CollapsiblePanel::update(float dt) {
    toggleButton.update(dt);
    
    if (!collapsed) {
        for (auto& elem : elements) {
            elem->update(dt);
        }
    }
}

void CollapsiblePanel::draw(sf::RenderWindow& window) {
    if (!collapsed) {
        window.draw(background);

        for (auto& elem : elements) {
            elem->draw(window);
        }
    }
    
    toggleButton.draw(window);
}

void CollapsiblePanel::setPosition(const sf::Vector2f& newAnchorPoint) {
    sf::Vector2f delta = newAnchorPoint - anchorPoint;
    anchorPoint = newAnchorPoint;
    
    for (auto& element : elements) {
        sf::Vector2f oldPos = element->getPosition();
        element->setPosition(oldPos + delta);
    }
    
    if (!collapsed) {
        background.setPosition(getExpandedPosition());
    }
    
    updateButtonPosition();
}

void CollapsiblePanel::addElement(std::unique_ptr<UIElement> element) {
    elements.push_back(std::move(element));
}

void CollapsiblePanel::setCollapsed(bool newState) {
    collapsed = newState;
    toggleButton.setText(collapsed ? collapsedText : expandedText);
    
    if (collapsed) {
        updateButtonPosition();
    } else {
        background.setPosition(getExpandedPosition());
        updateButtonPosition();
    }
}

void CollapsiblePanel::toggleCollapse() {
    setCollapsed(!collapsed);
}
