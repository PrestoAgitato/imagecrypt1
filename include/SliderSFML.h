#pragma once
#include <SFML\Graphics.hpp>
#include <iostream>
#include <functional>

class SliderSFML
{
	std::function<void(float)> callbackFunction;
	sf::RectangleShape slider;
	sf::RectangleShape axis;
	sf::Font font;
	sf::Text text;
	int minValue;
	int maxValue;
	int xCord;
	int yCord;
	int axisWidth;
	int axisHeight;
	int sliderWidth;
	int sliderHeight;
	float sliderValue;
public:
	SliderSFML(int x, int y);
	sf::Text returnText(int x, int y, std::string z, int fontSize);
	void create(int min, int max);
	void logic(sf::RenderWindow& window);
	float getSliderValue();
	void setFont(sf::Font& font);
	void setSliderValue(float newValue);
	void setSliderPercentValue(float newPercentValue);
	void setCallback(const std::function<void(float)>& callback);
	void draw(sf::RenderWindow& window);
};
