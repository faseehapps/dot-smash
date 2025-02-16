#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>
#include <random>
#include <cmath>
#include <chrono>
#include <vector>
#include <numeric>

int generateRandomNumber(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

class Dot {
public:
    sf::CircleShape dot;
    int radius = 50;
    sf::Vector2i position;

    Dot(sf::Vector2i resolution) : dot(50) {
        dot.setFillColor(sf::Color::Green);
        changePosition(resolution);
    }
    
    void changePosition(sf::Vector2i resolution) {
        position.x = generateRandomNumber(0, resolution.x - radius * 2);
        position.y = generateRandomNumber(0, resolution.y - radius * 2);
        dot.setPosition(position.x, position.y);
	}
	
	// Returns true if 'point' is on top of the circle.
	bool onTop(sf::Vector2i point) {
        int dx = position.x + radius - point.x;
        int dy = position.y + radius - point.y;
        int distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= radius * radius;
	}
};

class ScoreManager {
public:
    int totalNumberOfDots = 10;
    int dotsRemaining = totalNumberOfDots;
    bool gameOver = false;
    std::vector<float> responseTimes; // To calculate the average response time.
    std::chrono::high_resolution_clock::time_point start; // To calculate response time.

    bool checkForGameOver() {
        if (dotsRemaining == 0) {
            gameOver = true;
        }
        return gameOver;
    }

    float calculateResponseTime() {
        float responseTime;
        if (dotsRemaining == totalNumberOfDots) {
            start = std::chrono::high_resolution_clock::now();
            responseTime = 0.0f;
        }
        else {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = end - start;
            start = end;
            responseTime = elapsed.count(); // In seconds as float.
            responseTimes.push_back(responseTime);
        }
        if (dotsRemaining > 0) {
            dotsRemaining -= 1;
        }
        return responseTime;
    }

    float calculateAverageResponseTime() {
        float sum = std::accumulate(responseTimes.begin() + 1, responseTimes.end(), 0.0f);
        return sum / responseTimes.size();
    }
};

// To change the text
void changeAverageResponseTime(std::string newString, sf::Text& object) {
    object.setString(newString);
    // Update the origin
    object.setOrigin(object.getLocalBounds().width / 2, object.getLocalBounds().height / 2);
}

int main() {
    // Create the window.
    sf::Vector2i resolution; // Declare the window resolution.
    resolution.x = 1280;
    resolution.y = 720;
    sf::RenderWindow window(
        sf::VideoMode(resolution.x, resolution.y),
        "Dot Smash",
        sf::Style::Close
    );

    Dot theDot(resolution); // The dot that has to be smashed.
    ScoreManager scoreManager;

    // Load PlayfulTime-BLBB8.ttf.
    sf::Font playfulTime;
    if (!playfulTime.loadFromFile("assets/PlayfulTime-BLBB8.ttf")) {
        std::cout << "Failed to fetch the font file." << std::endl;
        return 1;
    }

    // Create a text object for response time.
    sf::Text responseTime("Click the dot to start.", playfulTime, 30);
    responseTime.setFillColor(sf::Color::White);
    responseTime.setPosition(10, 10);

    // Create a text object for remaining dots.
    sf::Text remaining(std::to_string(scoreManager.dotsRemaining), playfulTime, 60);
    remaining.setFillColor(sf::Color::White);
    remaining.setPosition(10, 48);

    // Game Over Text
    sf::Text gameOver("Game Over", playfulTime, 80);
    gameOver.setFillColor(sf::Color::White);
    // Make it centered
    gameOver.setOrigin(gameOver.getLocalBounds().width / 2, gameOver.getLocalBounds().height / 2);
    gameOver.setPosition(resolution.x / 2, resolution.y / 3);

    // Average Response Time Text
    sf::Text averageResponseTime("Average Response Time: ", playfulTime, 50);
    averageResponseTime.setFillColor(sf::Color::White);
    // Make it centered
    averageResponseTime.setOrigin(averageResponseTime.getLocalBounds().width / 2, averageResponseTime.getLocalBounds().height / 2);
    averageResponseTime.setPosition(resolution.x / 2, 330);

    // To determine whether we need to calculate the average response time.
    bool calculateAverageResponseTime = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (!scoreManager.checkForGameOver() && theDot.onTop(sf::Mouse::getPosition(window))) {
                    theDot.changePosition(resolution);
                    responseTime.setString(
                        "Response Time: " +
                        std::to_string(scoreManager.calculateResponseTime())
                    );
                    remaining.setString(std::to_string(scoreManager.dotsRemaining));
                    if (scoreManager.checkForGameOver()) {
                        calculateAverageResponseTime = true;
                    }
                }
            }
        }

        window.clear();

        if (!scoreManager.gameOver) {
            window.draw(theDot.dot);
            window.draw(responseTime);
            window.draw(remaining);
        }
        else {
            window.draw(gameOver);
            if (calculateAverageResponseTime) {
                changeAverageResponseTime(
                    std::to_string(
                        scoreManager.calculateAverageResponseTime()
                    ),
                    averageResponseTime
                );
                calculateAverageResponseTime = false;
            }
            window.draw(averageResponseTime);
        }

        window.display();
    }

    return 0;
}
