#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

typedef void (*ValueChangedCallback)(float newValue);

class Slider {
public:
    Slider()
        : handleVisible(false), dragging(false), isSliderPressed(false) {}

    Slider(std::string t, sf::Vector2f size, int charSize, float maxVal)
        : maxVal(maxVal), currentVal(0), handleVisible(false), dragging(false), isSliderPressed(false) {
        text.setString(t);
        text.setFillColor(sf::Color::Blue);
        text.setCharacterSize(charSize);


        updateValueText();

        handle.setSize({ size.x / 10, size.y * 2 });
        handle.setFillColor(sf::Color::Red);

        track.setSize(size);
        track.setFillColor(sf::Color::Green);
        updateHandlePosition();
    }

    void setFont(sf::Font& font) {
        text.setFont(font);
        value.setFont(font);
    }

    void setPosition(sf::Vector2f pos) {
        track.setPosition(pos);

        float xPosV = pos.x + track.getGlobalBounds().width + 10; // Расположение текста значения справа от трека
        float yPosV = pos.y + (track.getSize().y - value.getGlobalBounds().height) / 2;
        value.setPosition({ xPosV, yPosV });

        float xPos = pos.x - text.getGlobalBounds().width - 10; // Расположение текста слева от трека
        float yPos = pos.y + (track.getSize().y - text.getGlobalBounds().height) / 2;
        text.setPosition({ xPos, yPos });

        updateHandlePosition();
    }

    void drawTo(sf::RenderWindow& window) {
        window.draw(track);
        window.draw(text);
        window.draw(value);
        if (handleVisible) {
            window.draw(handle);
        }
    }

    void setHandleVisible(bool isVisible) {
        handleVisible = isVisible;
    }

    void handleMousePressed(sf::RenderWindow& window) {
        if (isMouseOver(window) && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            dragging = true;
            handleVisible = true; // Make sure this is set to true
            isSliderPressed = true;
            handleMouseMoved(window); // Consider updating the handle position immediately
        }
    }

    void handleMouseReleased() {
        std::cout << "Mouse released on slider" << std::endl; // Добавьте этот вывод для отладки
        dragging = false;
        isSliderPressed = false;
    }

    void handleMouseMoved(sf::RenderWindow& window) {
        if (dragging && isSliderPressed) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);  // Преобразование координат мыши

            float clampedX = std::max(track.getPosition().x, std::min(worldPos.x, track.getPosition().x + track.getSize().x - handle.getSize().x));
            float percentage = (clampedX - track.getPosition().x) / (track.getSize().x - handle.getSize().x);
            currentVal = percentage * maxVal; // Допустим, что maxVal уже установлено в максимальное значение

             if (onValueChanged) onValueChanged(currentVal);

            // Отладочный вывод для проверки вычисленных значений
            std::cout << "Mouse X: " << worldPos.x << ", Clamped X: " << clampedX << ", Percentage: " << percentage << ", Val: " << currentVal << std::endl;

            updateValueText();
            updateHandlePosition();
        }
    }
    void updateHandlePosition() {
        float handleXPos = track.getPosition().x + (currentVal / maxVal) * (track.getSize().x - handle.getSize().x);
        handle.setPosition(handleXPos, track.getPosition().y + (track.getSize().y - handle.getSize().y) / 2);

        // Отладочный вывод для проверки положения ползунка
        std::cout << "Handle Position X: " << handleXPos << std::endl;
    }




    bool getSliderPressed() { return isSliderPressed; }

    bool isMouseOver(const sf::RenderWindow& window) const {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect sliderBounds = track.getGlobalBounds();
        return sliderBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    float getValue() const {
        return currentVal;
    }


private:
    sf::RectangleShape track;
    sf::RectangleShape handle;
    sf::Text text;
    sf::Text value;
    float maxVal;
    float currentVal;

    bool handleVisible;
    bool dragging;
    bool isSliderPressed;

    void updateValueText() {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << currentVal;
        value.setString(stream.str());
    }

    ValueChangedCallback onValueChanged;
    
};
