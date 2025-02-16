#include "Scene_Frogger.h"
#include "Scene_Menu.h"  // Ensure this is present in Scene_Frogger.cpp
#include <cstdlib> // For rand()
#include <iostream>

Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
{
    init(levelPath);
}



void Scene_Frogger::init(const std::string& path)
{
    // --- Player Sprite Initialization (as before) ---
    playerSprite.setTexture(Assets::getInstance().getTexture("Entities"));
    playerSprite.setTextureRect(sf::IntRect(217, 203, 39, 40));
    sf::FloatRect playerBounds = playerSprite.getLocalBounds();
    playerSprite.setOrigin(playerBounds.width / 2.f, playerBounds.height / 2.f);
    const sf::Vector2u winSize = _game->window().getSize();
    float playerX = winSize.x / 2.f;
    float playerY = winSize.y - playerBounds.height / 2.f;
    playerSprite.setPosition(playerX, playerY);

    // --- Background Initialization ---
    backgroundSprite.setTexture(Assets::getInstance().getTexture("Background"));
    backgroundSprite.setPosition(0.f, 0.f);
    {
        sf::FloatRect bgBounds = backgroundSprite.getLocalBounds();
        backgroundSprite.setScale(
            static_cast<float>(winSize.x) / bgBounds.width,
            static_cast<float>(winSize.y) / bgBounds.height
        );
    }

    // --- Spawn Enemy Cars ---
    // For example, spawn one car moving left-to-right and another moving right-to-left.
    spawnEnemyCar(sf::Vector2f(-100.f, winSize.y * 0.4f), 100.f);   // moves right
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, winSize.y * 0.6f), -120.f); // moves left

    // Other initialization...
    m_score = 0;
    m_lives = 3;
    m_maxHeight = 0;
}


void Scene_Frogger::loadBackground()
{
    // Attempt to load the background image
    if (!backgroundTexture.loadFromFile("../assets/Textures/background01.png")) {
        std::cerr << "Error: Could not load background image!\n";
    }
    backgroundSprite.setTexture(backgroundTexture);

    // Scale the background to fit the window
    backgroundSprite.setScale(
        _game->window().getSize().x / backgroundSprite.getLocalBounds().width,
        _game->window().getSize().y / backgroundSprite.getLocalBounds().height
    );
}

void Scene_Frogger::loadLevel(const std::string& path)
{
    // Placeholder: Read level file (to be implemented)
    std::cout << "Loading level from: " << path << std::endl;

    // Here, you would parse `path` to create lanes, obstacles, and player positions.
}

void Scene_Frogger::onEnd()
{
    std::cout << "Exiting Scene_Frogger..." << std::endl;
}
void Scene_Frogger::update(sf::Time dt)
{
    // --- Player Movement and Boundary Checking (as before) ---
    const float moveSpeed = 150.f;
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        movement.y -= moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        movement.y += moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        movement.x -= moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        movement.x += moveSpeed * dt.asSeconds();
    playerSprite.move(movement);

    // (Include your clamping logic for the player here.)

    // --- Update Enemy Cars ---
    const sf::Vector2u winSize = _game->window().getSize();
    for (auto& car : enemyCars)
    {
        // Update the car animation.
        car.animation->update(dt);

        // Move the car horizontally.
        car.animation->getSprite().move(car.speed * dt.asSeconds(), 0.f);

        // Wrap around: if a car leaves the window, reposition it on the opposite side.
        sf::FloatRect carBounds = car.animation->getSprite().getGlobalBounds();
        if (car.speed > 0 && carBounds.left > winSize.x) {
            car.animation->getSprite().setPosition(-carBounds.width, car.animation->getSprite().getPosition().y);
        }
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0) {
            car.animation->getSprite().setPosition(winSize.x, car.animation->getSprite().getPosition().y);
        }
    }

    // Call other update helper functions if necessary.
    sMovement(dt);
    sCollisions(dt);
    //sUpdate(dt);
    sAnimation(dt);
}

void Scene_Frogger::sDoAction(const Command& action)
{
    if (action.type() == "START") {
        if (action.name() == "MENU") {
            std::cout << "Returning to Main Menu..." << std::endl;
            _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game));
        }
    }
}


void Scene_Frogger::sRender() {
    _game->window().clear();

    // Draw the background.
    _game->window().draw(backgroundSprite);

    // Draw all enemy cars using their animation sprite.
    for (const auto& car : enemyCars) {
        _game->window().draw(car.animation->getSprite());
    }

    // Draw the player.
    _game->window().draw(playerSprite);

    // Optionally, draw HUD elements.
    sf::Text scoreText("Score: 100", Assets::getInstance().getFont("main"), 20);
    scoreText.setPosition(10.f, 10.f);
    _game->window().draw(scoreText);
}

void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;

    // Randomly choose an animation name for the car.
    int carType = rand() % 5;  // five possible car types: car, raceCarL, raceCarR, truck, tractor.
    std::string carAnimName;
    switch (carType) {
    case 0: carAnimName = "car"; break;
    case 1: carAnimName = "raceCarL"; break;
    case 2: carAnimName = "raceCarR"; break;
    case 3: carAnimName = "truck"; break;
    case 4: carAnimName = "tractor"; break;
    default: carAnimName = "car"; break;
    }
    car.type = carAnimName;

    // Retrieve the animation from the Assets singleton.
    // (getAnimation returns a const reference; use const_cast if you plan to update the animation.)
    car.animation = const_cast<Animation*>(&Assets::getInstance().getAnimation(carAnimName));

    // Set the car's initial position.
    car.animation->getSprite().setPosition(position);

    // Set the horizontal movement speed.
    car.speed = speed;

    enemyCars.push_back(car);
}

void Scene_Frogger::sMovement(sf::Time dt)
{

}

void Scene_Frogger::sCollisions(sf::Time dt)
{

}

//void Scene_Frogger::update(sf::Time dt)
//{
//    
//}


void Scene_Frogger::sAnimation(sf::Time dt)
{

}
