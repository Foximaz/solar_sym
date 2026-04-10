#ifndef UI_H
#define UI_H

#include <functional>
#include <SFML/Graphics.hpp>
#include <memory>

class UIElement {
protected:
    bool isVisible = true;

public:
    virtual ~UIElement() = default;
    
    virtual void handleEvent(const sf::Event& event, const sf::RenderWindow& window) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    
    virtual void setPosition(const sf::Vector2f& position) = 0;
    virtual sf::Vector2f getPosition() const = 0;
    virtual void setVisible(bool visible);
    virtual bool getVisible() const;
};


class Button : public UIElement {
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
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    
    void setPosition(const sf::Vector2f& position) override;
    sf::Vector2f getPosition() const override;
    void setText(const std::string& newText);
    void setNormalColor(const sf::Color& color);
};



class TextBox : public UIElement {
private:
    sf::RectangleShape background;
    sf::Text text;
    sf::Font& font;
    
    sf::Color bgColor;
    sf::Color textColor;
    sf::Color outlineColor;
    float outlineThickness;
    
    bool drawBackground;
    bool drawOutline;

public:
    TextBox(
        sf::Font& font,
        sf::Vector2f position,
        sf::Vector2f size,
        std::string text,
        unsigned int characterSize = 20,
        sf::Color textColor = sf::Color::White,
        sf::Color bgColor = sf::Color::Transparent,
        sf::Color outlineColor = sf::Color::Black,
        float outlineThickness = 0.0f
    );

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void setPosition(const sf::Vector2f& position) override;
    sf::Vector2f getPosition() const override;

    void setText(const std::string& newText);
    std::string getText() const;
    
    void setTextColor(const sf::Color& color);
    void setBackgroundColor(const sf::Color& color);
    void setOutline(const sf::Color& color, float thickness);
    
    void setDrawBackground(bool draw);
    void setDrawOutline(bool draw);
};


class InputField : public UIElement {
private:
    sf::RectangleShape background;
    sf::Text text;
    sf::Font& font;
    float blinkAccumulator;
    bool showCursor;
    bool hasFocus;
    bool numericOnly;
    bool enabled;

    sf::Color bgColor;
    sf::Color textColor;
    sf::Color outlineColor;
    float outlineThickness;

    void updateTextPosition();
    void clampText();

public:
    InputField(
        sf::Font& font,
        sf::Vector2f position,
        sf::Vector2f size,
        unsigned int characterSize = 20,
        bool numericOnly = false,
        sf::Color bgColor = sf::Color(50, 50, 50),
        sf::Color textColor = sf::Color::White,
        sf::Color outlineColor = sf::Color::Black,
        float outlineThickness = 2.0f
    );

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void setPosition(const sf::Vector2f& position) override;
    sf::Vector2f getPosition() const override;
    void setText(const std::string& s);
    std::string getText() const;

    void setNumericOnly(bool numeric);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    bool isFocused() const;
};


enum class PanelDirection {
    RightDown,
    RightUp,
    LeftDown,
    LeftUp
};

class CollapsiblePanel : public UIElement {
private:
    sf::RectangleShape background;
    Button toggleButton;
    std::vector<std::unique_ptr<UIElement>> elements;
    
    bool collapsed;
    PanelDirection direction;
    
    sf::Vector2f anchorPoint;
    sf::Vector2f expandedSize;
    
    sf::Font& font;
    sf::Color bgColor;
    sf::Color outlineColor;
    float outlineThickness;
    
    std::string collapsedText;
    std::string expandedText;

    sf::Vector2f getButtonOffset() const;
    sf::Vector2f getExpandedPosition() const;
    sf::Vector2f getButtonPosition() const;
    void updateLayout();
    void updateButtonPosition();

public:
    CollapsiblePanel(
        sf::Font& font,
        sf::Vector2f anchorPoint,
        sf::Vector2f expandedSize,
        const std::string& collapsedText = "?",
        const std::string& expandedText = "V",
        PanelDirection direction = PanelDirection::RightDown,
        sf::Color bgColor = sf::Color(40, 40, 50),
        sf::Color outlineColor = sf::Color::White,
        float outlineThickness = 2.0f
    );

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void setPosition(const sf::Vector2f& newAnchorPoint) override;
    sf::Vector2f getPosition() const override { return anchorPoint; }

    void addElement(std::unique_ptr<UIElement> element);
    void clearElements();

    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return collapsed; }
    void toggleCollapse();

    void setDirection(PanelDirection direction);
    PanelDirection getDirection() const { return direction; }
    
    void setSize(const sf::Vector2f& newSize);
    sf::Vector2f getSize() const { return expandedSize; }
};

#endif