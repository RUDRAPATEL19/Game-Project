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
    //  Player Sprite Initialization
    playerSprite.setTexture(Assets::getInstance().getTexture("Entities"));
    playerSprite.setTextureRect(sf::IntRect(217, 203, 39, 40));
    sf::FloatRect playerBounds = playerSprite.getLocalBounds();
    playerSprite.setOrigin(playerBounds.width / 2.f, playerBounds.height / 2.f);
    const sf::Vector2u winSize = _game->window().getSize();
    float playerX = winSize.x / 2.f;
    float playerY = winSize.y - playerBounds.height / 2.f;
    playerSprite.setPosition(playerX, playerY);

    //  Background Initialization 
    backgroundSprite.setTexture(Assets::getInstance().getTexture("Background"));
    backgroundSprite.setPosition(0.f, 0.f);
    {
        sf::FloatRect bgBounds = backgroundSprite.getLocalBounds();
        backgroundSprite.setScale(
            static_cast<float>(winSize.x) / bgBounds.width,
            static_cast<float>(winSize.y) / bgBounds.height
        );
    }

    // --- Spawn Enemy Cars Outside the River (Upper & Lower Lanes) ---
    // Example: Two lanes outside the river
    float laneTop1 = winSize.y * 0.08f;   // Upper lane (near top)
    float laneTop2 = winSize.y * 0.22f;  // Second upper lane
    spawnEnemyCar(sf::Vector2f(-100.f, laneTop1), 100.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneTop2), -120.f);

    float laneBottom1 = winSize.y * 0.73f;  // First lower lane
    float laneBottom2 = winSize.y * 0.88f;     // Second lower lane
    spawnEnemyCar(sf::Vector2f(-100.f, laneBottom1), 110.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneBottom2), -130.f);

    // --- Spawn Logs and River Enemies in the River Region ---
    // Define the river region. Previously you used 30%-70%; now we shift them upward
    // and reduce the separation between the two lanes.
    // For example, we can choose two lanes at 35% and 45% of the window height.
    float riverLaneUpper = winSize.y * 0.38f; // Upper river lane (shifted upward)
    float riverLaneLower = winSize.y * 0.55f; // Lower river lane (closer to the upper lane)

    // In the upper river lane, have both logs and river enemies move to the right.
    spawnLog(sf::Vector2f(-200.f, riverLaneUpper), 80.f);            // Log starting off-screen left.
    spawnRiverEnemy(sf::Vector2f(winSize.x + 150.f, riverLaneUpper), 80.f); // River enemy starting off-screen right.

    // In the lower river lane, have both logs and river enemies move to the left.
    spawnLog(sf::Vector2f(winSize.x + 200.f, riverLaneLower), -90.f);   // Log starting off-screen right.
    spawnRiverEnemy(sf::Vector2f(-150.f, riverLaneLower), -90.f);        // River enemy starting off-screen left.

    // --- Other Initialization ---
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
    // --- Player Movement (as before) ---
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

    // (Add player boundary checking code as before.)

    const sf::Vector2u winSize = _game->window().getSize();

    // --- Update Enemy Cars ---
    for (auto& car : enemyCars)
    {
        car.sprite.move(car.speed * dt.asSeconds(), 0.f);
        sf::FloatRect carBounds = car.sprite.getGlobalBounds();
        if (car.speed > 0 && carBounds.left > winSize.x) {
            car.sprite.setPosition(-carBounds.width, car.sprite.getPosition().y);
        }
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0) {
            car.sprite.setPosition(winSize.x, car.sprite.getPosition().y);
        }
    }

    // --- Update Logs ---
    for (auto& log : logs)
    {
        log.sprite.move(log.speed * dt.asSeconds(), 0.f);
        sf::FloatRect logBounds = log.sprite.getGlobalBounds();
        if (log.speed > 0 && logBounds.left > winSize.x) {
            log.sprite.setPosition(-logBounds.width, log.sprite.getPosition().y);
        }
        else if (log.speed < 0 && (logBounds.left + logBounds.width) < 0) {
            log.sprite.setPosition(winSize.x, log.sprite.getPosition().y);
        }
    }

    // --- Update River Enemies ---
    for (auto& enemy : riverEnemies)
    {
        enemy.sprite.move(enemy.speed * dt.asSeconds(), 0.f);
        sf::FloatRect enemyBounds = enemy.sprite.getGlobalBounds();
        if (enemy.speed > 0 && enemyBounds.left > winSize.x) {
            enemy.sprite.setPosition(-enemyBounds.width, enemy.sprite.getPosition().y);
        }
        else if (enemy.speed < 0 && (enemyBounds.left + enemyBounds.width) < 0) {
            enemy.sprite.setPosition(winSize.x, enemy.sprite.getPosition().y);
        }
    }

    // Update additional helper functions as needed.
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

    // Draw enemy cars.
    for (const auto& car : enemyCars) {
        _game->window().draw(car.sprite);
    }

    // Draw logs.
    for (const auto& log : logs) {
        _game->window().draw(log.sprite);
    }

    // Draw river enemies.
    for (const auto& enemy : riverEnemies) {
        _game->window().draw(enemy.sprite);
    }

    // Draw the player.
    _game->window().draw(playerSprite);

    // Optionally draw HUD elements.
    sf::Text scoreText("Score: 100", Assets::getInstance().getFont("main"), 20);
    scoreText.setPosition(10.f, 10.f);
    _game->window().draw(scoreText);
}


void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;
    car.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    // Use the "car" animation sub-rectangle (adjust these values as needed)
    car.sprite.setTextureRect(sf::IntRect(220, 59, 35, 29));
    car.sprite.setPosition(position);
    car.speed = speed;
    enemyCars.push_back(car);
}


void Scene_Frogger::spawnLog(const sf::Vector2f& position, float speed)
{
    Log log;
    log.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    // For example, assume the log image is in the "tree2.png" area:
    // Adjust the sub-rectangle to the correct coordinates for your log image.
    log.sprite.setTextureRect(sf::IntRect(30, 29, 195, 28));
    log.sprite.setPosition(position);
    log.speed = speed;
    logs.push_back(log);
}

// Spawn a river enemy (e.g., a crocodile).
void Scene_Frogger::spawnRiverEnemy(const sf::Vector2f& position, float speed)
{
    RiverEnemy enemy;
    enemy.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    // Use the "croc" animation sub-rectangle.
    // For example, choose between "croc (1)" and "croc (2)". Here we use "croc (1)".
    enemy.sprite.setTextureRect(sf::IntRect(1, 203, 102, 40));
    enemy.sprite.setPosition(position);
    enemy.speed = speed;
    riverEnemies.push_back(enemy);
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
