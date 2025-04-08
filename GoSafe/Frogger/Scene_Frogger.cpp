#include "Scene_Frogger.h"
#include "Scene_Menu.h"  
#include <cstdlib> 
#include <cmath> 
#include <iostream>

Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
{
    init(levelPath);
}


const float safeJumpOffset = 50.f;


void Scene_Frogger::init(const std::string& path)
{
    unsigned int designWidth = 2560;
    unsigned int designHeight = 1600;
    float computedScaleX = static_cast<float>(designWidth) / 480.f;
    float computedScaleY = static_cast<float>(designHeight) / 600.f;
    float overallMultiplier = 0.5f;
    scaleFactorX = computedScaleX * overallMultiplier;
    scaleFactorY = computedScaleY * overallMultiplier;

    spawnDrone(sf::Vector2f(1280.f, 800.f), 100.f);
    initTrafficSignals();

    // Initialize Player Sprite.
    playerSprite.setTexture(Assets::getInstance().getTexture("Entities"));
    playerSprite.setTextureRect(sf::IntRect(223, 201, 27, 41));
    sf::FloatRect playerBounds = playerSprite.getLocalBounds();
    playerSprite.setOrigin(playerBounds.width / 2.f, playerBounds.height / 2.f);
    float playerX = designWidth / 2.f;
    float playerY = designHeight - playerBounds.height / 2.f;
    playerSprite.setPosition(playerX, playerY);
    playerSprite.setScale(scaleFactorX, scaleFactorY);
    startPosition = playerSprite.getPosition();
    originalStartPosition = startPosition;
    groundY = playerY;

    // Initialize Background.
    backgroundSprite.setTexture(Assets::getInstance().getTexture("Background"));
    backgroundSprite.setPosition(0.f, 0.f);
    {
        sf::FloatRect bgBounds = backgroundSprite.getLocalBounds();
        backgroundSprite.setScale(designWidth / bgBounds.width, designHeight / bgBounds.height);
    }

    // Spawn enemy cars.
    float laneTop1 = designHeight * 0.08f;
    float laneTop2 = designHeight * 0.22f;
    spawnEnemyCar(sf::Vector2f(-100.f, laneTop1), 100.f * scaleFactorX);
    spawnEnemyCar(sf::Vector2f(designWidth + 100.f, laneTop2), -120.f * scaleFactorX);

    float laneBottom1 = designHeight * 0.73f;
    float laneBottom2 = designHeight * 0.88f;
    spawnEnemyCar(sf::Vector2f(-100.f, laneBottom1), 110.f * scaleFactorX);
    spawnEnemyCar(sf::Vector2f(designWidth + 100.f, laneBottom2), -130.f * scaleFactorX);

    // Define river lane positions.
    float riverLaneUpper = designHeight * 0.38f;
    float riverLaneLower = designHeight * 0.55f;
    float logEnemyOffset = 20.f;
    float adjustedUpperLane = riverLaneUpper + logEnemyOffset;

    // Spawn logs and river enemies.
    spawnLog(sf::Vector2f(-200.f, adjustedUpperLane + safeJumpOffset), 80.f * scaleFactorX);
    spawnRiverEnemy(sf::Vector2f(1000.f, adjustedUpperLane + safeJumpOffset), 80.f * scaleFactorX);


    spawnLog(sf::Vector2f(1200.f, riverLaneLower), -90.f * scaleFactorX);
    spawnRiverEnemy(sf::Vector2f(-150.f, riverLaneLower), -90.f * scaleFactorX);

    // Spawn safe river power-up.
    sf::Vector2f powerUpPos(designWidth / 2.f, designHeight * 0.85f);
    spawnPowerUp(powerUpPos, 0.f);

    safeRiverSpawnDelay = 5.f + static_cast<float>(std::rand() % 6);
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
    const float upwardOffset = 220.f;   // amount to move up on each death


    playerSprite.setPosition(originalStartPosition.x, originalStartPosition.y - upwardOffset);

    isJumping = false;
    verticalVelocity = 0.f;
    onLog = false;
    currentLogIndex = -1;

    std::cout << "New spawn position: (" << startPosition.x
        << ", " << startPosition.y << ")" << std::endl;
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



void cycleState(TrafficSignal& signal) {
    if (signal.state == SignalState::Green)
        signal.state = SignalState::Yellow;
    else if (signal.state == SignalState::Yellow)
        signal.state = SignalState::Red;
    else
        signal.state = SignalState::Green;

    // Set the sprite's color based on the new state:
    if (signal.state == SignalState::Green)
        signal.sprite.setColor(sf::Color::Green);
    else if (signal.state == SignalState::Yellow)
        signal.sprite.setColor(sf::Color::Yellow);
    else
        signal.sprite.setColor(sf::Color::Red);

    // Reset any state timer if needed.
    signal.stateTimer = 0.f;
}


void Scene_Frogger::initTrafficSignals() {
    TrafficSignal signal1;
    signal1.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    signal1.sprite.setTextureRect(sf::IntRect(16, 10, 37, 42));

    sf::FloatRect bounds = signal1.sprite.getLocalBounds();
    signal1.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    signal1.sprite.setColor(sf::Color::Magenta);

    const sf::Vector2u winSize = _game->window().getSize();
    float posX = 50.f;
    float posY = winSize.y * 0.805f;
    signal1.sprite.setPosition(posX, posY);

    signal1.state = SignalState::Green;
    signal1.sequenceOrder = 1;
    signal1.activated = false;

    trafficSignals.push_back(signal1);

}

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Scene_Frogger::handleDroneHit() {
    triggerDeath();
}



void updateDrone(Drone& drone, sf::Time dt, const sf::Vector2u& winSize, const sf::Sprite& playerSprite, Scene_Frogger* scene) {
    sf::Vector2f dronePos = drone.sprite.getPosition();
    sf::Vector2f playerPos = playerSprite.getPosition();

    float dx = playerPos.x - dronePos.x;
    float dy = playerPos.y - dronePos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    const float alignmentThreshold = 30.f;
    const float chargeDuration = 2.f;
    const float fireDuration = 2.f;
    const float cooldownDuration = 1.f;

    switch (drone.state)
    {
    case DroneState::Following:
    {
        if (std::abs(dx) <= alignmentThreshold && dronePos.y < playerPos.y) {
            drone.state = DroneState::Charging;
            drone.stateTimer = 0.f;
        }
        else {
            if (dist != 0.f) {
                sf::Vector2f direction = (playerPos - dronePos) / dist;
                drone.sprite.move(direction * (drone.speed * 2.f) * dt.asSeconds());
            }
        }
        break;
    }
    case DroneState::Charging:
    {
        drone.stateTimer += dt.asSeconds();
        if (drone.stateTimer >= chargeDuration) {
            drone.state = DroneState::Firing;
            drone.stateTimer = 0.f;
            // Initialize laser hitbox:
            drone.laserHitbox.setSize(sf::Vector2f(60.f, 200.f));
            drone.laserHitbox.setFillColor(sf::Color(255, 0, 0, 200));
            drone.laserHitbox.setOrigin(drone.laserHitbox.getSize().x / 2.f, 0.f);
            drone.laserHitbox.setPosition(drone.sprite.getPosition());
        }
        break;
    }
    case DroneState::Firing:
    {
        drone.stateTimer += dt.asSeconds();
        float laserOffsetY = 20.f;
        drone.laserHitbox.setPosition(drone.sprite.getPosition().x,
            drone.sprite.getPosition().y + laserOffsetY);
        if (playerSprite.getGlobalBounds().intersects(drone.laserHitbox.getGlobalBounds())) {
            scene->handleDroneHit();
        }
        if (drone.stateTimer >= fireDuration) {
            drone.state = DroneState::Cooldown;
            drone.stateTimer = 0.f;
            drone.laserHitbox.setFillColor(sf::Color::Transparent);
        }
        break;
    }


    case DroneState::Cooldown:
    {
        drone.stateTimer += dt.asSeconds();
        if (drone.stateTimer >= cooldownDuration) {
            drone.state = DroneState::Following;
            drone.stateTimer = 0.f;
        }
        break;
    }
    }
}



void Scene_Frogger::update(sf::Time dt)
{
    const sf::Vector2u winSize = _game->window().getSize();

    // --- Win Condition Check (Game Finished) ---
    if (!gameFinished && playerSprite.getPosition().y <= 5.f)
    {
        gameFinished = true;
        finishOption = 0;
        std::cout << "Game Finished: You Won!" << std::endl;
    }

    // If win condition is met, process finish menu input and exit update().
    if (gameFinished)
    {
        // Process finish menu input for win scenario.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            finishOption = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            finishOption = 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            if (finishOption == 0)
            {
                std::cout << "Play Again selected!" << std::endl;
                _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, "../assets/level1.txt"), true);
            }
            else if (finishOption == 1)
            {
                std::cout << "Back to Menu selected!" << std::endl;
                _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
            }
        }
        return;  // Skip all further processing when gameFinished is true.
    }


    // --- Process Game Over Menu (if gameOver is true) ---
    if (gameOver)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            gameOverOption = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            gameOverOption = 1;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            if (gameOverOption == 0)
            {
                std::cout << "Play Again selected!" << std::endl;
                _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, "../assets/level1.txt"), true);
            }
            else if (gameOverOption == 1)
            {
                std::cout << "Back to Menu selected!" << std::endl;
                _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
            }
        }
        return;
    }

    // --- Power-Up Spawn Check (Safe River) ---
    {
        bool safeRiverActive = false;
        for (const auto& pu : powerUps)
        {
            if (pu.active)
            {
                safeRiverActive = true;
                break;
            }
        }
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

    // --- Horizontal Movement ---
    const float moveSpeed = 150.f * scaleFactorX;
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

    // --- Traffic Signal Interaction ---
    for (auto& signal : trafficSignals)
    {
        if (playerSprite.getGlobalBounds().intersects(signal.sprite.getGlobalBounds()))
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            {
                if (!signal.activated)
                {
                    cycleState(signal);
                    signal.activated = true;
                    puzzleCheckTriggered = true;
                    std::cout << "Signal " << signal.sequenceOrder << " toggled." << std::endl;
                }
            }
            else
            {
                signal.activated = false;
            }
        }
    }

    // --- Check Puzzle Sequence Only When Player Submits ---
    if (puzzleCheckTriggered && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
    {
        std::cout << "X pressed: Checking signal state..." << std::endl;
        bool puzzleSolved = true;
        for (const auto& signal : trafficSignals)
        {
            if (signal.state != SignalState::Red)
            {
                puzzleSolved = false;
                break;
            }
        }
        if (puzzleSolved)
        {
            std::cout << "Correct sequence! Safe passage activated." << std::endl;
            safePassageActivated = true;
            safePassageTimer = 0.f;
        }
        else
        {
            std::cout << "Incorrect sequence. Try again." << std::endl;
        }
        puzzleCheckTriggered = false;
    }


    // --- Jump Mechanic ---
    if (!isJumping && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        isJumping = true;
        jumpTimer = 0.f;
        jumpStartPosition = playerSprite.getPosition();

        m_playerAnimState = PlayerAnimState::Jumping;
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

        if (isJumping && jumpTimer >= jumpDuration)
        {
            isJumping = false;
            newPos.y = jumpStartPosition.y - jumpForward;
            playerSprite.setPosition(newPos);
            m_playerAnimState = PlayerAnimState::Walking;
        }
    }

    //if (m_playerIsHit)
    //{
    //    m_playerAnimState = PlayerAnimState::Dying;
    //    m_animTimer = 0.f;      // Reset the timer
    //    m_dyingFrameIndex = 0;  // Start from the first dying frame
    //}



    // --- Log Riding ---
    if (!isJumping)
    {
        bool foundLog = false;
        const float margin = 5.f;
        for (int i = 0; i < logs.size(); ++i)
        {
            sf::FloatRect logBounds = logs[i].sprite.getGlobalBounds();
            logBounds.left -= margin;
            logBounds.top -= margin;
            logBounds.width += 2 * margin;
            logBounds.height += 2 * margin;
            if (playerSprite.getGlobalBounds().intersects(logBounds))
            {
                foundLog = true;
                currentLogIndex = i;
                sf::FloatRect actualLogBounds = logs[i].sprite.getGlobalBounds();
                sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
                float newY = actualLogBounds.top - playerBounds.height / 2.f;
                sf::Vector2f pos = playerSprite.getPosition();
                pos.y = newY;
                playerSprite.setPosition(pos);
                playerSprite.move(logs[i].speed * dt.asSeconds(), 0.f);
                break;
            }
        }
        onLog = foundLog;
    }

    // --- Safe Landing on Road Check ---
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

    // --- River Collision Check ---
    if (!isJumping && !gameFinished && m_playerAnimState != PlayerAnimState::Dying)
    {
        float riverTop = winSize.y * 0.35f;
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
                triggerDeath();
            }
        }
    }




    const float designWidth = 2560.f;
    const float designHeight = 1600.f;

    // Lane positions from your init code:
    float laneTop1 = designHeight * 0.08f;
    float laneTop2 = designHeight * 0.22f;
    float laneBottom1 = designHeight * 0.73f;
    float laneBottom2 = designHeight * 0.88f;
    float riverLaneUpper = designHeight * 0.38f;
    float riverLaneLower = designHeight * 0.55f;
    float logEnemyOffset = 20.f; // Vertical offset for logs in upper lane
    float adjustedUpperLane = riverLaneUpper + logEnemyOffset;

    const float clearance = 100.f;

    // Increase spawn probability values as desired.
    float vehicleSpawnChance = 0.05f; 
    float logSpawnChance = 0.02f; 
    float enemySpawnChance = 0.05f;


    // --- Spawn Enemy Cars ---
    if (static_cast<float>(std::rand()) / RAND_MAX < vehicleSpawnChance * dt.asSeconds())
    {
        int laneChoice = std::rand() % 4;
        float laneY = 0.f;
        float speed = 0.f;
        float startX = 0.f;
        switch (laneChoice)
        {
        case 0:
            laneY = laneTop1;
            startX = -100.f;
            speed = 100.f * scaleFactorX;
            break;
        case 1:
            laneY = laneTop2;
            startX = designWidth + 100.f;
            speed = -120.f * scaleFactorX;
            break;
        case 2:
            laneY = laneBottom1;
            startX = -100.f;
            speed = 110.f * scaleFactorX;
            break;
        case 3:
            laneY = laneBottom2;
            startX = designWidth + 100.f;
            speed = -130.f * scaleFactorX;
            break;
        }
        if (isLaneClearForEnemyCar(laneY, startX, clearance))
        {
            spawnEnemyCar(sf::Vector2f(startX, laneY), speed);
        }
    }

    // Define an extra offset for upper lane logs/river enemies for safe jumping.
      // Increase to 50 pixels for visibility

    // --- Spawn Logs ---
    if (static_cast<float>(std::rand()) / RAND_MAX < logSpawnChance * dt.asSeconds())
    {
        int lane = std::rand() % 2;  // 0 for upper lane, 1 for lower lane
        float logY;
        if (lane == 0) {
            // Upper lane: add extra safe jump offset
            logY = adjustedUpperLane + safeJumpOffset;
            std::cout << "Spawning log in upper lane at Y: " << logY << std::endl;
        }
        else {
            logY = riverLaneLower;
        }
        float startX, speed;
        if (lane == 0)
        {
            // Upper lane: move left-to-right.
            startX = -200.f;
            speed = 80.f * scaleFactorX;
        }
        else
        {
            // Lower lane: move right-to-left.
            startX = designWidth + 200.f;
            speed = -90.f * scaleFactorX;
        }
        if (isLaneClearForLog(logY, startX, clearance))
        {
            spawnLog(sf::Vector2f(startX, logY), speed);
        }
    }

    // --- Spawn River Enemies ---
    if (static_cast<float>(std::rand()) / RAND_MAX < enemySpawnChance * dt.asSeconds())
    {
        int lane = std::rand() % 2;  // 0 for upper lane, 1 for lower lane
        float enemyY;
        if (lane == 0) {
            enemyY = adjustedUpperLane + safeJumpOffset;  // Add extra offset for upper lane
            std::cout << "Spawning river enemy in upper lane at Y: " << enemyY << std::endl;
        }
        else {
            enemyY = riverLaneLower;
        }
        float startX, speed;
        if (lane == 0)
        {
            // Upper lane: move left-to-right.
            startX = -150.f;
            speed = 80.f * scaleFactorX;
        }
        else
        {
            // Lower lane: move right-to-left.
            startX = designWidth + 150.f;
            speed = -90.f * scaleFactorX;
        }
        if (isLaneClearForRiverEnemy(enemyY, startX, clearance))
        {
            spawnRiverEnemy(sf::Vector2f(startX, enemyY), speed);
        }
    }

    // --- Safe Passage Effect on Enemy Cars ---
    if (safePassageActivated)
    {
        safePassageTimer += dt.asSeconds();
        if (safePassageTimer >= safePassageDuration)
        {
            safePassageActivated = false;
            safePassageTimer = 0.f;
            std::cout << "Safe passage ended." << std::endl;
        }
    }
    for (auto& car : enemyCars)
    {
        // When safe passage is active, stop the cars (multiplier = 0), otherwise move normally (multiplier = 1)
        float speedMultiplier = safePassageActivated ? 0.0f : 1.0f;
        car.sprite.move(car.speed * speedMultiplier * dt.asSeconds(), 0.f);

        sf::FloatRect carBounds = car.sprite.getGlobalBounds();
        if (car.speed > 0 && carBounds.left > winSize.x)
            car.sprite.setPosition(-carBounds.width, car.sprite.getPosition().y);
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0)
            car.sprite.setPosition(winSize.x, car.sprite.getPosition().y);
    }

        
    // --- Update Logs ---
    for (auto& log : logs)
    {
        log.sprite.move(log.speed * dt.asSeconds(), 0.f);
        sf::FloatRect logBounds = log.sprite.getGlobalBounds();
        if (log.speed > 0 && logBounds.left > winSize.x)
            log.sprite.setPosition(-logBounds.width - 50.f, log.sprite.getPosition().y);
        else if (log.speed < 0 && (logBounds.left + logBounds.width) < 0)
            log.sprite.setPosition(winSize.x + 50.f, log.sprite.getPosition().y);
    }


    // --- Update River Enemies (Unaffected by safe passage) ---
    for (auto& enemy : riverEnemies)
    {
        float speedMultiplier = 1.0f;
        enemy.sprite.move(enemy.speed * speedMultiplier * dt.asSeconds(), 0.f);
        if (enemy.speed < 0)
            enemy.sprite.setScale(-RIVER_ENEMY_SCALE.x, RIVER_ENEMY_SCALE.y);
        else
            enemy.sprite.setScale(RIVER_ENEMY_SCALE.x, RIVER_ENEMY_SCALE.y);

        sf::FloatRect enemyBounds = enemy.sprite.getGlobalBounds();
        if (enemy.speed > 0 && enemyBounds.left > winSize.x)
            enemy.sprite.setPosition(-enemyBounds.width - 100.f, enemy.sprite.getPosition().y);
        else if (enemy.speed < 0 && (enemyBounds.left + enemyBounds.width) < 0)
            enemy.sprite.setPosition(winSize.x + 100.f, enemy.sprite.getPosition().y);
    }


    // --- Update Drones ---
    for (auto& drone : drones)
    {
        updateDrone(drone, dt, winSize, playerSprite, this);
    }


    sCollisions(dt);
    sMovement(dt);
    sAnimation(dt);
}

bool Scene_Frogger::isLaneClearForEnemyCar(float laneY, float spawnX, float clearance)
{
    for (const auto& car : enemyCars)
    {
        if (std::abs(car.sprite.getPosition().y - laneY) < 10.f)
        {
            if (std::abs(car.sprite.getPosition().x - spawnX) < clearance)
                return false;
        }
    }
    return true;
}

bool Scene_Frogger::isLaneClearForLog(float laneY, float spawnX, float clearance)
{
    for (const auto& log : logs)
    {
        if (std::abs(log.sprite.getPosition().y - laneY) < 10.f)
        {
            if (std::abs(log.sprite.getPosition().x - spawnX) < clearance)
                return false;
        }
    }
    return true;
}

bool Scene_Frogger::isLaneClearForRiverEnemy(float laneY, float spawnX, float clearance)
{
    for (const auto& enemy : riverEnemies)
    {
        if (std::abs(enemy.sprite.getPosition().y - laneY) < 10.f)
        {
            if (std::abs(enemy.sprite.getPosition().x - spawnX) < clearance)
                return false;
        }
    }
    return true;
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

    // Draw traffic signals.
    for (const auto& signal : trafficSignals) {
        _game->window().draw(signal.sprite);
    }

	// Draw drones.
    for (const auto& drone : drones)
    {
        _game->window().draw(drone.sprite);
        if (drone.state == DroneState::Firing)
        {
            _game->window().draw(drone.laserHitbox);
        }
    }

    // After drawing all game elements, get hover message:
    std::string hoverMessage = getHoverMessage();
    if (!hoverMessage.empty()) {
        sf::Text hoverText;
        hoverText.setFont(Assets::getInstance().getFont("main"));
        hoverText.setCharacterSize(24);
        hoverText.setFillColor(sf::Color::Cyan);
        hoverText.setOutlineColor(sf::Color::Black);
        hoverText.setOutlineThickness(1.f);
        hoverText.setString(hoverMessage);

        hoverText.setPosition(20.f, _game->window().getSize().y - 60.f);

        _game->window().draw(hoverText);
    }




    // Draw the score.
    sf::Text scoreText("Score: 100", Assets::getInstance().getFont("main"), 20);
    scoreText.setPosition(10.f, 10.f);
    _game->window().draw(scoreText);

    // Draw Power-Up Status.
    sf::Text powerUpText;
    powerUpText.setFont(Assets::getInstance().getFont("main"));
    powerUpText.setCharacterSize(20);
    powerUpText.setFillColor(sf::Color::White);
    if (hasSafeRiver) {
        int secondsRemaining = static_cast<int>(safeRiverDuration - safeRiverTimer);
        powerUpText.setString("Power-Up: Safe River (" + std::to_string(secondsRemaining) + "s)");
    }
    else {
        powerUpText.setString("");
    }
    powerUpText.setPosition(10.f, 40.f);
    _game->window().draw(powerUpText);

    if (gameOver || gameFinished)
    {
        // Draw a semi-transparent overlay.
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(_game->window().getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        _game->window().draw(overlay);

        // Draw Game Over text.
        sf::Text finalText;
        finalText.setFont(Assets::getInstance().getFont("main"));
        finalText.setCharacterSize(64);
        finalText.setString("Game Over");
        finalText.setFillColor(sf::Color::Red);
        sf::FloatRect textBounds = finalText.getLocalBounds();
        finalText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        finalText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y * 0.3f);
        _game->window().draw(finalText);

        // Draw options.
        std::string options;
        if (gameOver) {
            if (gameOverOption == 0)
                options = "> Play Again <\n  Back to Menu";
            else
                options = "  Play Again\n> Back to Menu <";
        }
        else {
            // For game finished, similar option handling if needed.
            if (finishOption == 0)
                options = "> Play Again <\n  Back to Menu";
            else
                options = "  Play Again\n> Back to Menu <";
        }
        sf::Text optionText(options, Assets::getInstance().getFont("main"), 48);
        optionText.setFillColor(sf::Color::White);
        sf::FloatRect optBounds = optionText.getLocalBounds();
        optionText.setOrigin(optBounds.left + optBounds.width / 2.f, optBounds.top + optBounds.height / 2.f);
        optionText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y * 0.5f);
        _game->window().draw(optionText);

        // Draw instructions.
        sf::Text instructText("Use W/S to choose, D to select", Assets::getInstance().getFont("main"), 20);
        instructText.setFillColor(sf::Color::White);
        sf::FloatRect instrBounds = instructText.getLocalBounds();
        instructText.setOrigin(instrBounds.left + instrBounds.width / 2.f, instrBounds.top + instrBounds.height / 2.f);
        instructText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y * 0.8f);
        _game->window().draw(instructText);
    }

    //_game->window().display();
}


void Scene_Frogger::triggerDeath()
{
    if (gameFinished)
        return;

    m_playerIsHit = true;
    m_playerAnimState = PlayerAnimState::Dying;
    m_animTimer = 0.f;
    m_dyingFrameIndex = 0;
    m_dyingTotalTime = 0.f;

    isJumping = false;
    verticalVelocity = 0.f;
}






void Scene_Frogger::spawnPowerUp(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    pu.sprite.setTextureRect(sf::IntRect(18, 193, 35, 35));
    pu.sprite.setPosition(position);
    pu.speed = speed;
    pu.active = true;
    powerUps.push_back(pu);
}

void Scene_Frogger::spawnSafeRiver(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    pu.sprite.setTextureRect(sf::IntRect(19, 193, 35, 35));  
    pu.sprite.setPosition(position);
    pu.speed = speed;
    pu.active = true;

    pu.sprite.setScale(SAFE_RIVER_SCALE);

    std::cout << "Safe river power-up new scale: "
        << pu.sprite.getScale().x << ", "
        << pu.sprite.getScale().y << std::endl;

    powerUps.push_back(pu);
}










void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;
    car.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    
    sf::IntRect carRects[5] = {
        sf::IntRect(130, 96, 36, 21),
        sf::IntRect(171, 96, 37, 22),
        sf::IntRect(214, 96, 47, 25),
    };
    int carType = std::rand() % 3;
    car.sprite.setTextureRect(carRects[carType]);
    
    // Apply scaling to match the design resolution.
    car.sprite.setScale(scaleFactorX, scaleFactorY);
    
    // Center the sprite.
    sf::FloatRect bounds = car.sprite.getLocalBounds();
    car.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    
    car.sprite.setPosition(position);
    car.speed = speed;  
    enemyCars.push_back(car);
}




void Scene_Frogger::spawnLog(const sf::Vector2f& position, float speed)
{
    sf::IntRect logRect(9, 94, 68, 29);
    Log log;
    log.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    log.sprite.setTextureRect(logRect);

    log.sprite.setScale(scaleFactorX, scaleFactorY);

    // Center the log's origin.
    sf::FloatRect bounds = log.sprite.getLocalBounds();
    log.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    log.sprite.setPosition(position);
    log.speed = speed; 
    logs.push_back(log);
}

const sf::Vector2f Scene_Frogger::SAFE_RIVER_SCALE(1.0f, 1.0f);
const sf::Vector2f Scene_Frogger::RIVER_ENEMY_SCALE(1.5f, 1.5f);



void Scene_Frogger::spawnRiverEnemy(const sf::Vector2f& position, float speed)
{
    RiverEnemy enemy;
    enemy.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    enemy.sprite.setTextureRect(sf::IntRect(10, 231, 83, 42));  // Region for river enemy
    sf::FloatRect bounds = enemy.sprite.getLocalBounds();
    enemy.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    enemy.sprite.setPosition(position);
    enemy.speed = speed;

    // Set the scale explicitly using our constant.
    enemy.sprite.setScale(RIVER_ENEMY_SCALE);

    std::cout << "River enemy new scale: "
              << enemy.sprite.getScale().x << ", "
              << enemy.sprite.getScale().y << std::endl;

    riverEnemies.push_back(enemy);
}



void Scene_Frogger::spawnDrone(const sf::Vector2f& position, float speed) {
    Drone drone;
    drone.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    // Set a texture rectangle that represents your drone.
    drone.sprite.setTextureRect(sf::IntRect(60, 193, 65, 35));
    // Center the drone’s origin.
    sf::FloatRect bounds = drone.sprite.getLocalBounds();
    drone.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    drone.sprite.setPosition(position);
    drone.speed = speed;
    // Optionally set a scale (using player scale factors, or define your own)
    drone.sprite.setScale(scaleFactorX, scaleFactorY);
    // Set an initial random target position (for testing)
    const sf::Vector2u winSize = _game->window().getSize();
    drone.targetPos.x = static_cast<float>(std::rand() % winSize.x);
    drone.targetPos.y = static_cast<float>(std::rand() % winSize.y);

    std::cout << "Spawning drone at: " << position.x << ", " << position.y << std::endl;
    drones.push_back(drone);
}


std::string Scene_Frogger::getHoverMessage() {
    // Get the current mouse position relative to the window.
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(_game->window());
    // Convert it to world coordinates.
    sf::Vector2f mousePos = _game->window().mapPixelToCoords(mousePixelPos);

    // Check if the mouse is over a traffic signal.
    for (const auto& signal : trafficSignals) {
        if (signal.sprite.getGlobalBounds().contains(mousePos)) {
            return "Traffic Signal: Press E to toggle. When red, press X for safe passage.";
        }
    }

    // Check if the mouse is over an enemy car.
    for (const auto& car : enemyCars) {
        if (car.sprite.getGlobalBounds().contains(mousePos)) {
            return "Enemy Car: Avoid these moving vehicles!";
        }
    }

    // Check if the mouse is over a log.
    for (const auto& log : logs) {
        if (log.sprite.getGlobalBounds().contains(mousePos)) {
            return "Log: Ride these to cross the river safely.";
        }
    }

    // Check if the mouse is over a river enemy.
    for (const auto& enemy : riverEnemies) {
        if (enemy.sprite.getGlobalBounds().contains(mousePos)) {
            return "River Enemy: Beware of these obstacles in the water.";
        }
    }

    // Check if the mouse is over a drone (if you have drones).
    for (const auto& drone : drones) {
        if (drone.sprite.getGlobalBounds().contains(mousePos)) {
            return "Drone: Stay clear of its laser attack!";
        }
    }

    // Check if the mouse is over a power-up.
    for (const auto& pu : powerUps) {
        if (pu.active && pu.sprite.getGlobalBounds().contains(mousePos)) {
            return "Power-Up: Collect this for a safe river crossing.";
        }
    }

    // No interactive element is being hovered over.
    return "";
}




void Scene_Frogger::sMovement(sf::Time dt)
{

}

void Scene_Frogger::sCollisions(sf::Time dt)
{
    bool collisionDetected = false;

    for (const auto& car : enemyCars)
    {
        if (playerSprite.getGlobalBounds().intersects(car.sprite.getGlobalBounds()))
        {
            collisionDetected = true;
            break;
        }
    }

    if (!collisionDetected) {
        for (const auto& enemy : riverEnemies)
        {
            if (playerSprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds()))
            {
                collisionDetected = true;
                break;
            }
        }
    }

    if (collisionDetected)
    {
        triggerDeath();
        return;
    }
}


//void Scene_Frogger::update(sf::Time dt)
//{
//    
//}

void Scene_Frogger::killPlayer()
{
    if (gameFinished)
        return;

    std::cout << "Player is dead. Game Over." << std::endl;
    gameOver = true;
}



void Scene_Frogger::sAnimation(sf::Time dt)
{
    static sf::IntRect walkFrame(12, 8, 26, 37);
    static sf::IntRect jumpFrame(45, 7, 27, 39);
    static sf::IntRect dyingFrames[2] = {
        sf::IntRect(81, 5, 29, 41),
        sf::IntRect(125, 18, 41, 25)
    };

    const float dyingFrameDuration = 0.1f;
    const float totalDyingDuration = 0.2f;

    switch (m_playerAnimState)
    {
        case PlayerAnimState::Walking:
            playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
            playerSprite.setTextureRect(walkFrame);
            break;
        case PlayerAnimState::Jumping:
            playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
            playerSprite.setTextureRect(jumpFrame);
            break;
        case PlayerAnimState::Dying:
        {
            m_animTimer += dt.asSeconds();
            m_dyingTotalTime += dt.asSeconds();
            if (m_animTimer >= dyingFrameDuration)
            {
                m_animTimer = 0.f;
                m_dyingFrameIndex = (m_dyingFrameIndex + 1) % 2;
            }
            playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
            playerSprite.setTextureRect(dyingFrames[m_dyingFrameIndex]);
            if (m_dyingTotalTime >= totalDyingDuration)
            {
                killPlayer();
                return;
            }
            break;
        }
    }
}

