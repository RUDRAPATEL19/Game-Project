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




void Scene_Frogger::init(const std::string& path)
{
    spawnDrone(sf::Vector2f(200.f, 200.f), 100.f);


    initTrafficSignals();
    //  Player Sprite Initialization
    playerSprite.setTexture(Assets::getInstance().getTexture("Entities"));
    playerSprite.setTextureRect(sf::IntRect(223, 201, 27, 41));
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
    signal1.sprite.setTextureRect(sf::IntRect(127, 209, 41, 46));

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


    TrafficSignal signal2;
    signal2.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    signal2.sprite.setTextureRect(sf::IntRect(127, 209, 41, 46));

    sf::FloatRect bounds2 = signal2.sprite.getLocalBounds();
    signal2.sprite.setOrigin(bounds2.width / 2.f, bounds2.height / 2.f);

    signal2.sprite.setColor(sf::Color::Magenta);

    float posX1 = 450.f;
    float posY1 = winSize.y * 0.805f;
    signal2.sprite.setPosition(posX1, posY1);

    signal2.state = SignalState::Green;
    signal2.sequenceOrder = 2;
    signal2.activated = false;

    trafficSignals.push_back(signal2);
}

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Scene_Frogger::handleDroneHit() {
    m_lives--;
    std::cout << "Lives remaining: " << m_lives << std::endl;
    resetPlayer();
    if (m_lives <= 0)
        _game->quitLevel(); // Or set gameOver to true.
}


// Updates a drone’s behavior based on its state.
void updateDrone(Drone& drone, sf::Time dt, const sf::Vector2u& winSize, const sf::Sprite& playerSprite, Scene_Frogger* scene) {
    sf::Vector2f dronePos = drone.sprite.getPosition();
    sf::Vector2f playerPos = playerSprite.getPosition();

    // Compute the Euclidean distance between the drone and player.
    float dx = playerPos.x - dronePos.x;
    float dy = playerPos.y - dronePos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    // Define behavior thresholds and durations.
    const float followThreshold = 100.f;  // When within 100 pixels, drone stops and charges.
    const float chargeDuration = 2.f;
    const float fireDuration = 2.f;
    const float cooldownDuration = 1.f;

    switch (drone.state)
    {
    case DroneState::Following:
    {
        // Move toward the player's position in both X and Y.
        if (dist > followThreshold) {
            sf::Vector2f direction = playerPos - dronePos;
            if (dist != 0.f)
                direction /= dist;  // Normalize
            drone.sprite.move(direction * drone.speed * dt.asSeconds());
        }
        else {
            // When close enough, transition to Charging.
            drone.state = DroneState::Charging;
            drone.stateTimer = 0.f;
        }
        break;
    }
    case DroneState::Charging:
    {
        // Stay still while charging.
        drone.stateTimer += dt.asSeconds();
        if (drone.stateTimer >= chargeDuration) {
            drone.state = DroneState::Firing;
            drone.stateTimer = 0.f;
            // Initialize the laser hitbox:
            drone.laserHitbox.setSize(sf::Vector2f(40.f, 200.f));  // width 40, height 200
            drone.laserHitbox.setFillColor(sf::Color(255, 0, 0, 200)); // semi-transparent red
            // Center the hitbox horizontally.
            drone.laserHitbox.setOrigin(drone.laserHitbox.getSize().x / 2.f, 0.f);
            drone.laserHitbox.setPosition(drone.sprite.getPosition());
        }
        break;
    }
    case DroneState::Firing:
    {
        // Remain stationary and show laser.
        drone.stateTimer += dt.asSeconds();
        drone.laserHitbox.setPosition(drone.sprite.getPosition());
        // Check for collision between the player's sprite and the laser.
        if (playerSprite.getGlobalBounds().intersects(drone.laserHitbox.getGlobalBounds())) {
            std::cout << "Player hit by drone laser!" << std::endl;
            scene->handleDroneHit();
        }
        if (drone.stateTimer >= fireDuration) {
            drone.state = DroneState::Cooldown;
            drone.stateTimer = 0.f;
            // Hide the laser.
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

    // --- Process Finish Menu Input (When Game is Finished) ---
    if (gameFinished)
    {
        // Process finish menu input (using finishOption)
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
        return; // Skip normal update logic when game is finished.
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
        return; // Skip normal update logic.
    }

    // --- The rest of your normal update logic continues below ---

    // --- Power-Up Spawn Check (Safe River) ---
    {
        bool safeRiverActive = false;
        for (const auto& pu : powerUps) {
            if (pu.active) {
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

    // --- Horizontal Movement (Always applied) ---
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
        std::cout << "X pressed: Checking signal states..." << std::endl;
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

    // --- Jump Mechanic (Vertical jump only) ---
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

    // --- Log Riding (Only when NOT jumping) ---
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

    // --- River Collision Check (if not on log and not safe due to power-up) ---
    if (!isJumping)
    {
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

    // --- Log Lost Check: If riding a log that goes off screen, the player dies. ---
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
        // Calculate distance between enemy car and player.
        float dist = distance(car.sprite.getPosition(), playerSprite.getPosition());

        // If the player is too close (e.g., within 150 pixels), adjust behavior.
        if (dist < 150.f)
        {
            // For example, slow down by setting a lower speed multiplier.
            float speedMultiplier = 0.2f;
            car.sprite.move(car.speed * speedMultiplier * dt.asSeconds(), 0.f);

            // Optionally, you can change direction randomly:
            // (For example, if the car's x position is to the left of the player, set a random positive speed,
            //  or vice versa. This is just one idea.)
            if (playerSprite.getPosition().x < car.sprite.getPosition().x)
                car.speed = std::abs(car.speed) * -1.f; // reverse direction
            else
                car.speed = std::abs(car.speed);
        }
        else
        {
            // Normal movement when player is not close.
            car.sprite.move(car.speed * dt.asSeconds(), 0.f);
        }

        // Wrapping logic (unchanged):
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
            log.sprite.setPosition(-logBounds.width, log.sprite.getPosition().y);
        else if (log.speed < 0 && (logBounds.left + logBounds.width) < 0)
            log.sprite.setPosition(winSize.x, log.sprite.getPosition().y);
    }
    // --- Update River Enemies (Unaffected by safe passage) ---
    for (auto& enemy : riverEnemies)
    {

        float speedMultiplier = 1.0f;
        enemy.sprite.move(enemy.speed * speedMultiplier * dt.asSeconds(), 0.f);
        if (enemy.speed < 0)
            enemy.sprite.setScale(-1.f, 1.f);
        else
            enemy.sprite.setScale(1.f, 1.f);
        sf::FloatRect enemyBounds = enemy.sprite.getGlobalBounds();
        if (enemy.speed > 0 && enemyBounds.left > winSize.x)
            enemy.sprite.setPosition(-enemyBounds.width, enemy.sprite.getPosition().y);
        else if (enemy.speed < 0 && (enemyBounds.left + enemyBounds.width) < 0)
            enemy.sprite.setPosition(winSize.x, enemy.sprite.getPosition().y);
    }


    // Update drones with unpredictable trajectories.
    for (auto& drone : drones)
    {
        updateDrone(drone, dt, winSize, playerSprite, this);
    }

    sCollisions(dt);
    sMovement(dt);
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

    // Check if Game Over or Game Finished and draw overlay.
    if (gameOver || gameFinished) {
        // Draw a semi-transparent overlay.
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        _game->window().draw(overlay);

        // Draw final message.
        sf::Text finalText;
        finalText.setFont(Assets::getInstance().getFont("main"));
        finalText.setCharacterSize(64);
        if (gameFinished) {
            finalText.setString("You Won!");
            finalText.setFillColor(sf::Color::Yellow);
        }
        else if (gameOver) {
            finalText.setString("Game Over");
            finalText.setFillColor(sf::Color::Red);
        }
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


void Scene_Frogger::spawnPowerUp(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    pu.sprite.setTextureRect(sf::IntRect(19, 193, 35, 35));
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

    powerUps.push_back(pu);
}



void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;
    car.sprite.setTexture(Assets::getInstance().getTexture("Entities"));

    sf::IntRect carRects[5] = {
        sf::IntRect(221, 60, 35, 29),
        sf::IntRect(88, 125, 31, 31),
        sf::IntRect(0, 58, 60, 32),
        sf::IntRect(174, 163, 36, 30),
        sf::IntRect(104, 163, 35, 30)
    };
    int carType = std::rand() % 5;
    car.sprite.setTextureRect(carRects[carType]);

    sf::FloatRect bounds = car.sprite.getLocalBounds();
    car.sprite.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);

    car.sprite.setPosition(position);
    car.speed = speed;
    enemyCars.push_back(car);
}



void Scene_Frogger::spawnLog(const sf::Vector2f& position, float speed)
{
    sf::IntRect logRect(70, 29, 160, 35);
    Log log;
    log.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    log.sprite.setTextureRect(logRect);
    sf::FloatRect bounds = log.sprite.getLocalBounds();
    log.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    log.sprite.setPosition(position);
    log.speed = speed;

    logs.push_back(log);
}



void Scene_Frogger::spawnRiverEnemy(const sf::Vector2f& position, float speed)
{
    RiverEnemy enemy;
    enemy.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    enemy.sprite.setTextureRect(sf::IntRect(10, 227, 83, 42));
    sf::FloatRect bounds = enemy.sprite.getLocalBounds();
    enemy.sprite.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    enemy.sprite.setPosition(position);
    enemy.speed = speed;
    riverEnemies.push_back(enemy);
}

void Scene_Frogger::spawnDrone(const sf::Vector2f& position, float speed) {
    Drone drone;
    drone.sprite.setTexture(Assets::getInstance().getTexture("Entities"));
    // Set a texture rectangle that represents your drone.
    drone.sprite.setTextureRect(sf::IntRect(10, 227, 83, 42)); // Adjust these values!
    // Center the drone’s origin.
    sf::FloatRect bounds = drone.sprite.getLocalBounds();
    drone.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    drone.sprite.setPosition(position);
    drone.speed = speed;
    // Set an initial random target position (for testing, within window bounds)
    const sf::Vector2u winSize = _game->window().getSize();
    drone.targetPos.x = static_cast<float>(std::rand() % winSize.x);
    drone.targetPos.y = static_cast<float>(std::rand() % winSize.y);

    drones.push_back(drone);
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
        m_lives--;
        std::cout << "Lives remaining: " << m_lives << std::endl;
        resetPlayer();

        isJumping = false;
        verticalVelocity = 0.f;

        if (m_lives <= 0)
        {
            std::cout << "No lives left. Game Over." << std::endl;
            gameOver = true;
            // Optionally, you could also stop certain timers or game actions here.
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
