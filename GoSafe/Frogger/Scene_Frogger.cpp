#include "Scene_Frogger.h"
#include "Scene_Menu.h"  
#include <cstdlib> 
#include <iostream>

Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
{
    init(levelPath);
}



void Scene_Frogger::init(const std::string& path)
{
    playerSprite.setTexture(Assets::getInstance().getTexture("Entities"));
    playerSprite.setTextureRect(sf::IntRect(217, 203, 39, 40));
    sf::FloatRect playerBounds = playerSprite.getLocalBounds();
    playerSprite.setOrigin(playerBounds.width / 2.f, playerBounds.height / 2.f);
    const sf::Vector2u winSize = _game->window().getSize();
    float playerX = winSize.x / 2.f;
    float playerY = winSize.y - playerBounds.height / 2.f;
    playerSprite.setPosition(playerX, playerY);

    backgroundSprite.setTexture(Assets::getInstance().getTexture("Background"));
    backgroundSprite.setPosition(0.f, 0.f);
    {
        sf::FloatRect bgBounds = backgroundSprite.getLocalBounds();
        backgroundSprite.setScale(
            static_cast<float>(winSize.x) / bgBounds.width,
            static_cast<float>(winSize.y) / bgBounds.height
        );
    }

    float laneTop1 = winSize.y * 0.1f;    
    float laneTop2 = winSize.y * 0.25f;  
    float laneBottom1 = winSize.y * 0.75f;  
    float laneBottom2 = winSize.y * 0.9f;   

    spawnEnemyCar(sf::Vector2f(-100.f, laneTop1), 100.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneTop2), -120.f);

    spawnEnemyCar(sf::Vector2f(-100.f, laneBottom1), 110.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneBottom2), -130.f);

    m_score = 0;
    m_lives = 3;
    m_maxHeight = 0;
}


void Scene_Frogger::loadBackground()
{
    if (!backgroundTexture.loadFromFile("../assets/Textures/background01.png")) {
        std::cerr << "Error: Could not load background image!\n";
    }
    backgroundSprite.setTexture(backgroundTexture);

    backgroundSprite.setScale(
        _game->window().getSize().x / backgroundSprite.getLocalBounds().width,
        _game->window().getSize().y / backgroundSprite.getLocalBounds().height
    );
}

void Scene_Frogger::loadLevel(const std::string& path)
{
    std::cout << "Loading level from: " << path << std::endl;

}

void Scene_Frogger::onEnd()
{
    std::cout << "Exiting Scene_Frogger..." << std::endl;
}
void Scene_Frogger::update(sf::Time dt)
{
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


    const sf::Vector2u winSize = _game->window().getSize();
    for (auto& car : enemyCars)
    {
        car.animation->update(dt);

        car.animation->getSprite().move(car.speed * dt.asSeconds(), 0.f);

        sf::FloatRect carBounds = car.animation->getSprite().getGlobalBounds();
        if (car.speed > 0 && carBounds.left > winSize.x) {
            car.animation->getSprite().setPosition(-carBounds.width, car.animation->getSprite().getPosition().y);
        }
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0) {
            car.animation->getSprite().setPosition(winSize.x, car.animation->getSprite().getPosition().y);
        }
    }

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

    _game->window().draw(backgroundSprite);
    
    for (const auto& car : enemyCars) {
        _game->window().draw(car.animation->getSprite());
    }

    _game->window().draw(playerSprite);

    sf::Text scoreText("Score: 100", Assets::getInstance().getFont("main"), 20);
    scoreText.setPosition(10.f, 10.f);
    _game->window().draw(scoreText);
}

void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;

    int carType = rand() % 5; 
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

    car.animation = const_cast<Animation*>(&Assets::getInstance().getAnimation(carAnimName));

    car.animation->getSprite().setPosition(position);

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
