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
    startPosition = playerSprite.getPosition();
    groundY = playerY; 

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

    float laneTop1 = winSize.y * 0.08f;   
    float laneTop2 = winSize.y * 0.22f;  
    spawnEnemyCar(sf::Vector2f(-100.f, laneTop1), 100.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneTop2), -120.f);

    float laneBottom1 = winSize.y * 0.73f; 
    float laneBottom2 = winSize.y * 0.88f;    
    spawnEnemyCar(sf::Vector2f(-100.f, laneBottom1), 110.f);
    spawnEnemyCar(sf::Vector2f(winSize.x + 100.f, laneBottom2), -130.f);

    float riverLaneUpper = winSize.y * 0.38f; 
    float riverLaneLower = winSize.y * 0.55f; 

    spawnLog(sf::Vector2f(-200.f, riverLaneUpper), 80.f);          
    spawnRiverEnemy(sf::Vector2f(winSize.x + 150.f, riverLaneUpper), 80.f);

    spawnLog(sf::Vector2f(winSize.x + 200.f, riverLaneLower), -90.f); 
    spawnRiverEnemy(sf::Vector2f(-150.f, riverLaneLower), -90.f);      

    sf::Vector2f powerUpPos(winSize.x / 2.f, winSize.y * 0.85f);
    spawnPowerUp(powerUpPos, 0.f);

    safeRiverSpawnDelay = 5.f + static_cast<float>(std::rand() % 6); // 5 to 10 seconds

    m_score = 0;
    m_lives = 3;
    m_maxHeight = 0;
}

void Scene_Frogger::safeRiver()
{
    const float vaultDistance = 60.f;
    sf::Vector2f pos = playerSprite.getPosition();
    pos.y -= vaultDistance; 

    playerSprite.setPosition(pos);
}






void Scene_Frogger::resetPlayer()
{
    playerSprite.setPosition(startPosition);
    isJumping = false;
    verticalVelocity = 0.f;
    onLog = false;
    currentLogIndex = -1;
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
    const sf::Vector2u winSize = _game->window().getSize();

    // --- Power-Up Spawn Check (Safe River) ---
    {
        bool safeRiverActive = false;
        for (const auto& pu : powerUps) {
            if (pu.active) {
                safeRiverActive = true;
                break;
            }
        }

        // If none is active, update the spawn timer.
        if (!safeRiverActive)
        {
            safeRiverSpawnTimer += dt.asSeconds();
            if (safeRiverSpawnTimer >= safeRiverSpawnDelay)
            {
                float spawnY = winSize.y * 0.90f;
                float spawnX = (winSize.x * 0.1f) +
                    static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (winSize.x * 0.8f)));
                sf::Vector2f powerUpPos(spawnX, spawnY);
                spawnSafeRiver(powerUpPos, 0.f);
                safeRiverSpawnTimer = 0.f;
                safeRiverSpawnDelay = 5.f + static_cast<float>(std::rand() % 6);
            }
        }
    }

    // --- Power-Up Collection Check ---
    for (auto& pu : powerUps)
    {
        if (pu.active && playerSprite.getGlobalBounds().intersects(pu.sprite.getGlobalBounds()))
        {
            std::cout << "Safe River Power-Up collected!" << std::endl;
            hasSafeRiver = true;
            pu.active = false;
            pu.sprite.setColor(sf::Color::Transparent);
            safeRiverSpawnTimer = 0.f;
            break;
        }
    }

    // --- Update Safe River Power-Up Timer ---
    if (hasSafeRiver)
    {
        safeRiverTimer += dt.asSeconds();
        if (safeRiverTimer >= safeRiverDuration)
        {
            std::cout << "Safe River power-up expired." << std::endl;
            hasSafeRiver = false;
            safeRiverTimer = 0.f;
        }
    }

    // ============================================================
    // 4. Horizontal Movement (Always applied)
    // ============================================================
    const float moveSpeed = 150.f;
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        movement.y -= moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        movement.y += moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        movement.x -= moveSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        movement.x += moveSpeed * dt.asSeconds();
    playerSprite.move(movement);


    // 5. Jump Mechanic (Vertical jump only)

    if (!isJumping && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        isJumping = true;
        jumpTimer = 0.f;
        jumpStartPosition = playerSprite.getPosition();
    }
    if (isJumping)
    {
        jumpTimer += dt.asSeconds();
        float fraction = jumpTimer / jumpDuration;
        if (fraction > 1.f) fraction = 1.f;

        float verticalArc = jumpHeight * sin(3.14159f * fraction);

        float forwardOffset = jumpForward * fraction;

        sf::Vector2f newPos = jumpStartPosition;
        newPos.y = jumpStartPosition.y - forwardOffset - verticalArc;
        playerSprite.setPosition(newPos);

        if (jumpTimer >= jumpDuration)
        {
            isJumping = false;
            newPos.y = jumpStartPosition.y - jumpForward;
            playerSprite.setPosition(newPos);
        }
    }

    // LOG RIDING (Only when NOT jumping)

    if (!isJumping)
    {
        bool foundLog = false;
        const float margin = 5.f;
        for (int i = 0; i < logs.size(); ++i)
        {
            sf::FloatRect logBounds = logs[i].sprite.getGlobalBounds();
            // Expand the log bounds
            logBounds.left -= margin;
            logBounds.top -= margin;
            logBounds.width += 2 * margin;
            logBounds.height += 2 * margin;

            if (playerSprite.getGlobalBounds().intersects(logBounds))
            {
                foundLog = true;
                currentLogIndex = i;
                // Snap player's vertical position onto the log.
                sf::FloatRect actualLogBounds = logs[i].sprite.getGlobalBounds();
                sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
                float newY = actualLogBounds.top - playerBounds.height / 2.f;
                sf::Vector2f pos = playerSprite.getPosition();
                pos.y = newY;
                playerSprite.setPosition(pos);
                // Move player along with the log.
                playerSprite.move(logs[i].speed * dt.asSeconds(), 0.f);
                break;
            }
        }
        onLog = foundLog;
    }


    // 7. Safe Landing on Road Check
    float roadThreshold = winSize.y * 0.70f;
    if (onLog && playerSprite.getPosition().y >= roadThreshold)
    {
        std::cout << "Safe landing on road from log jump!" << std::endl;
        sf::Vector2f pos = playerSprite.getPosition();
        pos.y = groundY;
        playerSprite.setPosition(pos);
        isJumping = false;
        verticalVelocity = 0.f;
        onLog = false;
    }


    // --- River Collision Check (if not on log and not safe due to power-up) ---
    if (!isJumping) {
        float riverTop = winSize.y * 0.30f;
        float riverBottom = winSize.y * 0.65f;
        float playerCenterY = playerSprite.getGlobalBounds().top + playerSprite.getGlobalBounds().height / 2.f;

        if (playerCenterY >= riverTop && playerCenterY <= riverBottom && !onLog)
        {
            if (hasSafeRiver && sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            {
                std::cout << "Safe River power-up activated by Z!" << std::endl;
                safeRiver();
                hasSafeRiver = false;
                safeRiverTimer = 0.f;
            }
            else if (!hasSafeRiver)
            {
                std::cout << "Player in river without log and no power-up! Dying." << std::endl;
                m_lives--;
                resetPlayer();
                if (m_lives <= 0)
                    _game->quitLevel();
            }
        }
    }

    // 8. Log Lost Check: If riding a log that goes off screen, the player dies.

    if (onLog && currentLogIndex >= 0)
    {
        sf::FloatRect logBounds = logs[currentLogIndex].sprite.getGlobalBounds();
        if ((logBounds.left + logBounds.width < 0) || (logBounds.left > winSize.x))
        {
            std::cout << "Log lost! Player falls." << std::endl;
            m_lives--;
            resetPlayer();
            if (m_lives <= 0)
                _game->quitLevel();
        }
    }

    // --- Update Other Game Objects ---
    for (auto& car : enemyCars)
    {
        car.sprite.move(car.speed * dt.asSeconds(), 0.f);
        sf::FloatRect carBounds = car.sprite.getGlobalBounds();
        if (car.speed > 0 && carBounds.left > winSize.x)
            car.sprite.setPosition(-carBounds.width, car.sprite.getPosition().y);
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0)
            car.sprite.setPosition(winSize.x, car.sprite.getPosition().y);
    }
    for (auto& log : logs)
    {
        log.sprite.move(log.speed * dt.asSeconds(), 0.f);
        sf::FloatRect logBounds = log.sprite.getGlobalBounds();
        if (log.speed > 0 && logBounds.left > winSize.x)
            log.sprite.setPosition(-logBounds.width, log.sprite.getPosition().y);
        else if (log.speed < 0 && (logBounds.left + logBounds.width) < 0)
            log.sprite.setPosition(winSize.x, log.sprite.getPosition().y);
    }
    for (auto& enemy : riverEnemies)
    {
        enemy.sprite.move(enemy.speed * dt.asSeconds(), 0.f);
        sf::FloatRect enemyBounds = enemy.sprite.getGlobalBounds();
        if (enemy.speed > 0 && enemyBounds.left > winSize.x)
            enemy.sprite.setPosition(-enemyBounds.width, enemy.sprite.getPosition().y);
        else if (enemy.speed < 0 && (enemyBounds.left + enemyBounds.width) < 0)
            enemy.sprite.setPosition(winSize.x, enemy.sprite.getPosition().y);
    }

    sCollisions(dt);
    sMovement(dt);
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

    // Draw power-ups.
    for (const auto& pu : powerUps) {
        if (pu.active) { 
            _game->window().draw(pu.sprite);
        }
    }

    // Draw the score.
    sf::Text scoreText("Score: 100", Assets::getInstance().getFont("main"), 20);
    scoreText.setPosition(10.f, 10.f);
    _game->window().draw(scoreText);

    // --- Draw Power-Up Status ---
    sf::Text powerUpText;
    powerUpText.setFont(Assets::getInstance().getFont("main"));
    powerUpText.setCharacterSize(20);
    powerUpText.setFillColor(sf::Color::White);

    if (hasSafeRiver)
    {
        int secondsRemaining = static_cast<int>(safeRiverDuration - safeRiverTimer);
        powerUpText.setString("Power-Up: Safe River (" + std::to_string(secondsRemaining) + "s)");
    }
    else
    {
        powerUpText.setString("");
    }
    // Position below the score.
    powerUpText.setPosition(10.f, 40.f);
    _game->window().draw(powerUpText);
}


void Scene_Frogger::spawnPowerUp(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    pu.sprite.setTextureRect(sf::IntRect(11, 1, 18, 21)); 
    pu.sprite.setPosition(position);
    pu.speed = speed;
    pu.active = true;
    powerUps.push_back(pu);
}

void Scene_Frogger::spawnSafeRiver(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    pu.sprite.setTextureRect(sf::IntRect(50, 1, 18, 21));
    pu.sprite.setPosition(position);
    pu.speed = speed;
    pu.active = true;

    powerUps.push_back(pu);
}



void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;
    car.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    car.sprite.setTextureRect(sf::IntRect(220, 59, 35, 29));
    car.sprite.setPosition(position);
    car.speed = speed;
    enemyCars.push_back(car);
}


void Scene_Frogger::spawnLog(const sf::Vector2f& position, float speed)
{
    Log log;
    log.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    log.sprite.setTextureRect(sf::IntRect(30, 29, 195, 28));
    log.sprite.setPosition(position);
    log.speed = speed;
    logs.push_back(log);
}

void Scene_Frogger::spawnRiverEnemy(const sf::Vector2f& position, float speed)
{
    RiverEnemy enemy;
    enemy.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
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
    bool collisionDetected = false;

    // Check collision with enemy cars.
    for (const auto& car : enemyCars)
    {
        if (playerSprite.getGlobalBounds().intersects(car.sprite.getGlobalBounds()))
        {
            std::cout << "Player collided with enemy car!" << std::endl;
            collisionDetected = true;
            break;
        }
    }

    // Check collision with river enemies.
    if (!collisionDetected) {
        for (const auto& enemy : riverEnemies)
        {
            if (playerSprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds()))
            {
                std::cout << "Player collided with river enemy!" << std::endl;
                collisionDetected = true;
                break;
            }
        }
    }

    if (collisionDetected)
    {
        // Decrease lives.
        m_lives--;
        std::cout << "Lives remaining: " << m_lives << std::endl;
        resetPlayer();

        isJumping = false;
        verticalVelocity = 0.f;

        // If no lives remain, exit to menu.
        if (m_lives <= 0)
        {
            std::cout << "No lives left. Exiting to menu." << std::endl;
            _game->quitLevel();
            return;
        }
    }
}


//void Scene_Frogger::update(sf::Time dt)
//{
//    
//}


void Scene_Frogger::sAnimation(sf::Time dt)
{

}
