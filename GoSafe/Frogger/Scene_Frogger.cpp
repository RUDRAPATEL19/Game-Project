#include "Scene_Frogger.h"
#include "Scene_Menu.h"  
#include <cstdlib> 
#include <cmath> 
#include <iostream>
#include "MusicPlayer.h"

Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , _levelPath(levelPath)
{
    if (levelPath.find("level2.txt") != std::string::npos)
        currentLevel = 2;
    else
        currentLevel = 1;

    init(levelPath);
}


void Scene_Frogger::init(const std::string& path)
{
    // clears previous session
    enemyCars.clear();
    logs.clear();
    riverEnemies.clear();
    powerUps.clear();
    drones.clear();
    trafficSignals.clear();

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
    float playerY = designHeight - playerBounds.height / 2.f - 85.f;
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

    // --- Initialize Finish Line ---
    finishLineSprite.setTexture(Assets::getInstance().getTexture("finish"));
    sf::FloatRect finishBounds = finishLineSprite.getLocalBounds();
    finishLineSprite.setOrigin(finishBounds.width / 2.f, finishBounds.height / 2.f);

    // Position at top-center of the screen
    float finishY = 20.f;  // Small margin from top
    float finishX = designWidth / 2.f;
    finishLineSprite.setPosition(finishX, finishY);

    finishLineSprite.setScale(1.0f, 1.0f);

    // Spawn enemy cars.
    float laneTop1 = designHeight * 0.06f;
    float laneTop2 = designHeight * 0.16f;
    spawnEnemyCar(sf::Vector2f(-100.f, laneTop1), 100.f * scaleFactorX);
    spawnEnemyCar(sf::Vector2f(designWidth + 100.f, laneTop2), -120.f * scaleFactorX);

    float laneBottom1 = designHeight * 0.63f;
    float laneBottom2 = designHeight * 0.84f;
    spawnEnemyCar(sf::Vector2f(-100.f, laneBottom1), 110.f * scaleFactorX);
    spawnEnemyCar(sf::Vector2f(designWidth + 100.f, laneBottom2), -130.f * scaleFactorX);

    // Define river lane positions.
    const float riverLaneUpper = designHeight * 0.30f;
    const float riverLaneLower = designHeight * 0.43f;
    float logEnemyOffset = 20.f;


    spawnLog(sf::Vector2f(1200.f, riverLaneLower), -90.f * scaleFactorX);
    spawnRiverEnemy(sf::Vector2f(-150.f, riverLaneLower), -90.f * scaleFactorX);

    // Spawn safe river power-up.
    sf::Vector2f powerUpPos(designWidth / 2.f, designHeight * 0.80f);
    spawnPowerUp(powerUpPos, 0.f);

    safeRiverSpawnDelay = 5.f + static_cast<float>(std::rand() % 6);
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

    if (signal.state == SignalState::Green)
        signal.sprite.setColor(sf::Color::Green);
    else if (signal.state == SignalState::Yellow)
        signal.sprite.setColor(sf::Color::Yellow);
    else
        signal.sprite.setColor(sf::Color::Red);

    signal.stateTimer = 0.f;
}


void Scene_Frogger::initTrafficSignals() {
    // --- Signal 1
    TrafficSignal signal1;
    signal1.sprite.setTexture(Assets::getInstance().getTexture("goSafe"));
    signal1.sprite.setTextureRect(sf::IntRect(114, 193, 30, 63));
    sf::FloatRect bounds = signal1.sprite.getLocalBounds();
    signal1.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    const sf::Vector2u winSize = _game->window().getSize();
    float posX1 = 50.f;
    float posY1 = winSize.y * 0.725f;
    signal1.sprite.setPosition(posX1, posY1);
    signal1.sprite.setScale(1.5f, 1.5f);
    signal1.state = SignalState::Green;
    signal1.stateTimer = 0.f;
    signal1.activated = false;
    signal1.sequenceOrder = 1;
    trafficSignals.push_back(signal1);

}




float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Scene_Frogger::handleDroneHit() {
    if (!m_playerIsHit) {
        m_playerIsHit = true;
        triggerDeath();
    }
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
        const float verticalOffset = 187.f;
        const float yStopThreshold = 5.f;

        // Shadow always placed at expected laser hit position
        float shadowOffsetY = 42.f + 200.f;  // same as laser offset + laser height
        sf::Vector2f shadowPos = drone.sprite.getPosition();
        shadowPos.y += 42.f + 200.f;  // same offset as before

        drone.shadowSprite.setPosition(shadowPos);

        // Scale the shadow based on Y-position perspective
        float shadowScale = scene->getPerspectiveScale(shadowPos.y);
        drone.shadowSprite.setScale(shadowScale, shadowScale);


        sf::Vector2f hoverPos = playerPos - sf::Vector2f(0.f, verticalOffset);

        float dx = hoverPos.x - dronePos.x;
        float dy = hoverPos.y - dronePos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (std::abs(dx) <= alignmentThreshold
            && std::abs(dronePos.y - hoverPos.y) <= yStopThreshold)
        {
            drone.state = DroneState::Charging;
            drone.stateTimer = 0.f;
        }
        else if (dist > 0.f)
        {
            sf::Vector2f direction = (hoverPos - dronePos) / dist;
            drone.sprite.move(direction * (drone.speed * 2.f) * dt.asSeconds());
        }
        break;
    }


    case DroneState::Charging:
    {

        // Shadow always placed at expected laser hit position
        float shadowOffsetY = 42.f + 200.f;  // same as laser offset + laser height
        sf::Vector2f shadowPos = drone.sprite.getPosition();
        shadowPos.y += 42.f + 200.f;  // same offset as before

        drone.shadowSprite.setPosition(shadowPos);

        // Scale the shadow based on Y-position perspective
        float shadowScale = scene->getPerspectiveScale(shadowPos.y);
        drone.shadowSprite.setScale(shadowScale, shadowScale);



        drone.stateTimer += dt.asSeconds();
        if (drone.stateTimer >= chargeDuration) {
            drone.state = DroneState::Firing;
            drone.stateTimer = 0.f;
            // Initialize laser hitbox:
            drone.laserHitbox.setSize(sf::Vector2f(80.f, 200.f));
            drone.laserHitbox.setFillColor(sf::Color(255, 0, 0, 200));
            drone.laserHitbox.setOrigin(drone.laserHitbox.getSize().x / 2.f, 0.f);
            drone.laserHitbox.setPosition(drone.sprite.getPosition());
        }
        break;
    }
    case DroneState::Firing:
    {
        // Shadow always placed at expected laser hit position
        float shadowOffsetY = 42.f + 200.f;  // same as laser offset + laser height
        sf::Vector2f shadowPos = drone.sprite.getPosition();
        shadowPos.y += 42.f + 200.f;  // same offset as before

        drone.shadowSprite.setPosition(shadowPos);

        // Scale the shadow based on Y-position perspective
        float shadowScale = scene->getPerspectiveScale(shadowPos.y);
        drone.shadowSprite.setScale(shadowScale, shadowScale);


        drone.stateTimer += dt.asSeconds();
        float laserOffsetY = 42.f;
        drone.laserHitbox.setPosition(drone.sprite.getPosition().x,
            drone.sprite.getPosition().y + laserOffsetY);
        
        if (drone.stateTimer >= fireDuration) {
            drone.state = DroneState::Cooldown;
            drone.stateTimer = 0.f;
            drone.laserHitbox.setFillColor(sf::Color::Transparent);
        }
        break;
    }


    case DroneState::Cooldown:
    {
        float shadowOffsetY = 42.f + 200.f;  // same as laser offset + laser height
        sf::Vector2f shadowPos = drone.sprite.getPosition();
        shadowPos.y += 42.f + 200.f;  // same offset as before

        drone.shadowSprite.setPosition(shadowPos);

        // Scale the shadow based on Y-position perspective
        float shadowScale = scene->getPerspectiveScale(shadowPos.y);
        drone.shadowSprite.setScale(shadowScale, shadowScale);


        drone.stateTimer += dt.asSeconds();
        if (drone.stateTimer >= cooldownDuration) {
            drone.state = DroneState::Following;
            drone.stateTimer = 0.f;
        }
        break;
    }
    }
}

float Scene_Frogger::getPerspectiveScale(float y)
{
    const float minScale = 0.4f;  // scale near top
    const float maxScale = 1.0f;  // scale near bottom
    float t = y / static_cast<float>(_game->window().getSize().y);
    return minScale + (maxScale - minScale) * t;
}

float Scene_Frogger::getPerspectiveScalePlayer(float y)
{
    const float minScale = 0.9f;  // scale near top
    const float maxScale = 1.8f;  // scale near bottom
    float t = y / static_cast<float>(_game->window().getSize().y);
    return minScale + (maxScale - minScale) * t;
}

float Scene_Frogger::getPerspectiveScaleDrone(float y)
{
    const float minScale = 1.2f;  // scale near top
    const float maxScale = 4.2f;  // scale near bottom
    float t = y / static_cast<float>(_game->window().getSize().y);
    return minScale + (maxScale - minScale) * t;
}





void Scene_Frogger::update(sf::Time dt)
{
    // Toggle pause
    static bool pPreviouslyPressed = false;
    bool pCurrentlyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::P);

    if (pCurrentlyPressed && !pPreviouslyPressed) {
        m_paused = !m_paused;
        std::cout << (m_paused ? "Game Paused\n" : "Game Resumed\n");
    }
    pPreviouslyPressed = pCurrentlyPressed;

    if (m_paused) return;

    const sf::Vector2u winSize = _game->window().getSize();

    static float runSoundTimer = 0.f;
    const float runSoundInterval = 0.3f;

    static bool bKeyPressedLastFrame = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
        if (!bKeyPressedLastFrame) {
            m_showBoundingBoxes = !m_showBoundingBoxes;  // Toggle on press
        }
        bKeyPressedLastFrame = true;
    }
    else {
        bKeyPressedLastFrame = false;
    }


    if (_waitingToRespawn)
    {
        _respawnDelayTimer += dt.asSeconds();

        if (_respawnDelayTimer >= RESPWAN_DELAY)
        {
            init(_levelPath);
            m_playerAnimState = PlayerAnimState::Idle;
            _waitingToRespawn = false;
            _respawnDelayTimer = 0.f;
        }
    }
    else
    {
        // --- Jump Mechanic ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            if (!isJumping && !jumpSoundPlayed)
            {
                isJumping = true;
                jumpTimer = 0.f;
                jumpStartPosition = playerSprite.getPosition();
                m_playerAnimState = PlayerAnimState::Jumping;

                MusicPlayer::getInstance().play("hop", false);
                MusicPlayer::getInstance().play("hop", false);

                jumpSoundPlayed = true;
            }
        }
        else
        {
            jumpSoundPlayed = false;
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
                m_playerAnimState = PlayerAnimState::Idle;
            }
        }

        //if (m_playerIsHit)
        //{
        //    m_playerAnimState = PlayerAnimState::Dying;
        //    m_animTimer = 0.f;      // Reset the timer
        //    m_dyingFrameIndex = 0;  // Start from the first dying frame
        //}



        // --- Log Riding ---
        bool foundLog = false;
        for (int i = 0; i < logs.size(); ++i) {
            sf::FloatRect logBounds = logs[i].sprite.getGlobalBounds();

            const float heightReduction = 0.4f;
            const float widthReduction = 0.1f;

            logBounds.top += logBounds.height * (heightReduction / 2.f);
            logBounds.height *= (1.f - heightReduction);
            logBounds.left += logBounds.width * (widthReduction / 2.f);
            logBounds.width *= (1.f - widthReduction);

            if (playerSprite.getGlobalBounds().intersects(logBounds)) {
                foundLog = true;
                currentLogIndex = i;
                playerSprite.move(logs[i].speed * dt.asSeconds(), 0.f);
                break;
            }
        }
        onLog = foundLog;


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

        if (!isJumping && !gameFinished && m_playerAnimState != PlayerAnimState::Dying)
        {
            float riverTop = winSize.y * 0.22f;
            float riverBottom = winSize.y * 0.49f;
            float playerCenterY = playerSprite.getGlobalBounds().top + playerSprite.getGlobalBounds().height / 2.f;

            if (playerCenterY >= riverTop && playerCenterY <= riverBottom && !onLog)
            {
                if (hasSafeRiver && sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
                {
                    std::cout << "Safe River power-up activated by Z!" << std::endl;
                    safeRiver();
                    hasSafeRiver = false;
                    safeRiverTimer = 0.f;
                    std::cout << "Player sliding in river!" << std::endl;
                    m_playerAnimState = PlayerAnimState::Sliding;
                }
                else if (!hasSafeRiver)
                {
                    std::cout << "Player in river without log and no power-up! Dying." << std::endl;
                    triggerDeath();
                }
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

        // Set player animation state based on movement (if not jumping or dying)
        if (!isJumping && m_playerAnimState != PlayerAnimState::Dying)
        {
            if (movement != sf::Vector2f(0.f, 0.f))
                m_playerAnimState = PlayerAnimState::Running;
            else
                m_playerAnimState = PlayerAnimState::Idle;
        }


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            auto menu = std::make_shared<Scene_Menu>(_game);
            menu->fromPausedGame = true;
            _game->changeScene("MENU", menu, false); // false means don't end current scene
            return;
        }



        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            runSoundTimer += dt.asSeconds();
            if (runSoundTimer >= runSoundInterval)
            {
                if (MusicPlayer::getInstance().getStatus() != sf::SoundSource::Playing)
                {
                    MusicPlayer::getInstance().play("run", false);
                }
                runSoundTimer = 0.f;
            }
        }
        else
        {
            runSoundTimer = 0.f;
        }

        // --- Apply Perspective Scaling to Player ---
        float scale = getPerspectiveScalePlayer(playerSprite.getPosition().y);
        playerSprite.setScale(scale, scale);

        sf::FloatRect bounds = playerSprite.getLocalBounds();
        playerSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);


    }


    if (gameOver)
    {
        return;
    }



    // --- Win Condition Check (Game Finished) ---
    if (!gameFinished && playerSprite.getPosition().y <= 5.f)
    {
        gameFinished = true;
        finishOption = 0;
        std::cout << "Game Finished: You Won!" << std::endl;
    }

    


    if (gameOver || gameFinished)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            auto menu = std::make_shared<Scene_Menu>(_game);
            menu->fromPausedGame = true;
            _game->changeScene("MENU", menu, false);
            return;
        }


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            finishOption = 0;  // Choose "Play Again"
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            finishOption = 1;  // Choose "Back To Menu"
        }

        // Process the Enter key.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
        {
            if (finishOption == 0)
            {
                std::cout << "Play Again selected!" << std::endl;
                _game->changeScene("PLAY", std::make_shared<Scene_Frogger>(_game, "../assets/level1.txt"), true);
            }
            else if (finishOption == 1)
            {
                std::cout << "Back To Menu selected!" << std::endl;
                _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
            }
        }
        return; // Skip further update processing.
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
				spawnPowerUp(powerUpPos, 0.f);
                safeRiverSpawnTimer = 0.f;
                safeRiverSpawnDelay = 5.f + static_cast<float>(std::rand() % 6);
            }
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

   



    // --- Update Traffic Signals ---
    for (auto& signal : trafficSignals)
    {
        signal.stateTimer += dt.asSeconds();

        if (signal.state == SignalState::Green && signal.stateTimer >= 3.f)
        {
            signal.state = SignalState::Yellow;
            signal.stateTimer = 0.f;
            signal.sprite.setTextureRect(sf::IntRect(30, 192, 30, 64));
        }
        else if (signal.state == SignalState::Yellow && signal.stateTimer >= 2.f)
        {
            signal.state = SignalState::Red;
            signal.stateTimer = 0.f;
            signal.sprite.setTextureRect(sf::IntRect(73, 192, 29, 62));
        }
        else if (signal.state == SignalState::Red && signal.stateTimer >= 2.f)
        {
            signal.state = SignalState::Green;
            signal.stateTimer = 0.f;
            signal.sprite.setTextureRect(sf::IntRect(114, 193, 30, 63));
        }
    }


    // --- Determine Global Speed Multiplier ---
    float speedMultiplier = 1.0f;
    bool anyRed = false;
    bool anyYellow = false;

    for (const auto& signal : trafficSignals)
    {
        if (signal.state == SignalState::Red) {
            anyRed = true;
            break;  // Red overrides yellow.
        }
        else if (signal.state == SignalState::Yellow) {
            anyYellow = true;
        }
    }
    if (anyRed)
        speedMultiplier = 0.0f;
    else if (anyYellow)
        speedMultiplier = 0.5f;


    // --- Update Enemy Cars (Single Loop) ---
    for (auto& car : enemyCars)
    {
        car.sprite.move(car.speed * speedMultiplier * dt.asSeconds(), 0.f);

        sf::FloatRect carBounds = car.sprite.getGlobalBounds();
        if (car.speed > 0 && carBounds.left > _game->window().getSize().x)
            car.sprite.setPosition(-carBounds.width, car.sprite.getPosition().y);
        else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0)
            car.sprite.setPosition(_game->window().getSize().x, car.sprite.getPosition().y);
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
    



    const float designWidth = 2560.f;
    const float designHeight = 1600.f;

    // Lane positions from init code:
    float laneTop1 = designHeight * 0.06f;
    float laneTop2 = designHeight * 0.16f;
    float laneBottom1 = designHeight * 0.63f;
    float laneBottom2 = designHeight * 0.84f;
    float riverLaneUpper = designHeight * 0.30f;
    float riverLaneLower = designHeight * 0.43f;

    float logEnemyOffset = 20.f; // Vertical offset for logs in upper lane

    const float clearance = 100.f;

    // Increase spawn probability values as desired.
    float vehicleSpawnChance = (currentLevel == 2) ? 0.10f : 0.05f;
    float logSpawnChance = 0.02f; 
    float enemySpawnChance = 0.09f;


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


    // --- Spawn Logs ---
    if (static_cast<float>(std::rand()) / RAND_MAX < logSpawnChance * dt.asSeconds())
    {
        int lane = std::rand() % 2;
        float laneY = (lane == 0) ? riverLaneUpper : riverLaneLower;

        float startX = (lane == 0) ? -200.f : designWidth + 200.f;
        float speed = (lane == 0) ? 80.f * scaleFactorX : -90.f * scaleFactorX;

        if (isLaneClearForLog(laneY, startX, clearance))
            spawnLog(sf::Vector2f(startX, laneY), speed);
    }


    // --- Spawn River Enemies ---
    float laneY1 = riverLaneUpper;
    float laneY2 = riverLaneLower;

    float startX1 = -150.f;
    float startX2 = designWidth + 150.f;

    float speed1 = 80.f * scaleFactorX;     // moving right
    float speed2 = -90.f * scaleFactorX;    // moving left

    if ((float)std::rand() / RAND_MAX < enemySpawnChance * dt.asSeconds()) {
        if (isLaneClearForRiverEnemy(laneY1, startX1, clearance))
            spawnRiverEnemy({ startX1, laneY1 }, speed1);
    }

    if ((float)std::rand() / RAND_MAX < enemySpawnChance * dt.asSeconds()) {
        if (isLaneClearForRiverEnemy(laneY2, startX2, clearance))
            spawnRiverEnemy({ startX2, laneY2 }, speed2);
    }



    // --- Safe Passage Effect on Enemy Cars ---
    const float signalTolerance = 50.f;

    for (auto& car : enemyCars)
    {
        float speedMultiplier = 1.0f;

        for (const auto& signal : trafficSignals)
        {
            if (signal.state == SignalState::Red)
            {
                anyRed = true;
                break;
            }
            else if (signal.state == SignalState::Yellow)
            {
                anyYellow = true;
            }
        }

        if (anyRed)
            speedMultiplier = 0.0f;  
        else if (anyYellow)
            speedMultiplier = 0.5f;  

        // --- Update Enemy Car Movement ---
        for (auto& car : enemyCars)
        {
            car.sprite.move(car.speed * speedMultiplier * dt.asSeconds(), 0.f);

            sf::FloatRect carBounds = car.sprite.getGlobalBounds();
            if (car.speed > 0 && carBounds.left > _game->window().getSize().x)
                car.sprite.setPosition(-carBounds.width, car.sprite.getPosition().y);
            else if (car.speed < 0 && (carBounds.left + carBounds.width) < 0)
                car.sprite.setPosition(_game->window().getSize().x, car.sprite.getPosition().y);
        }
    }

        
    // --- Update Logs ---
    for (auto& log : logs)
    {
        log.sprite.move(log.speed * dt.asSeconds(), 0.f);
        sf::FloatRect logBounds = log.sprite.getGlobalBounds();
        if (log.speed > 0 && logBounds.left > winSize.x)
            log.sprite.setPosition(-logBounds.width - 50.f, log.originalY);
        else if (log.speed < 0 && (logBounds.left + logBounds.width) < 0)
            log.sprite.setPosition(winSize.x + 50.f, log.originalY);

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
            enemy.sprite.setPosition(-enemyBounds.width - 100.f, enemy.originalY);
        else if (enemy.speed < 0 && (enemyBounds.left + enemyBounds.width) < 0)
            enemy.sprite.setPosition(winSize.x + 100.f, enemy.originalY);

    }


    // --- Update Drones ---
    for (auto& drone : drones)
    {
        updateDrone(drone, dt, winSize, playerSprite, this);

        // Apply perspective scaling
        //float scale = getPerspectiveScalePlayer(drone.sprite.getPosition().y);
        //drone.sprite.setScale(scale, scale);
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
        if (std::abs(log.sprite.getPosition().y - laneY) < 25.f)
        {
            if (std::abs(log.sprite.getPosition().x - spawnX) < clearance * 0.75f)
                return false;
        }
    }
    return true;
}

bool Scene_Frogger::isLaneClearForRiverEnemy(float laneY, float spawnX, float clearance)
{
    for (const auto& enemy : riverEnemies)
    {
        if (std::abs(enemy.sprite.getPosition().y - laneY) < 30.f)
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
            _game->changeScene("MENU", std::make_shared<Scene_Menu>(_game), true);
        }
    }
}


void Scene_Frogger::sRender() {
    _game->window().clear();
    // Draw the background.
    _game->window().draw(backgroundSprite);
    _game->window().draw(finishLineSprite);

    



    // Draw enemy cars.
    for (const auto& car : enemyCars) {
        _game->window().draw(car.sprite);
    }

    {
        const float ICON_SCALE = 1.3f;
        const float PADDING = 20.f;
        const float SPACING = 20.f;

        sf::Sprite lifeIcon;
        lifeIcon.setTexture(Assets::getInstance().getTexture("goSafe"));
        lifeIcon.setTextureRect(sf::IntRect(12, 1, 28, 63));
        lifeIcon.setScale(ICON_SCALE, ICON_SCALE);

        float iconAreaWidth = m_lives * (lifeIcon.getGlobalBounds().width + SPACING) - SPACING;

        float extraWidth = 100.f;
        float extraHeight = 60.f;

        float desiredWidth = iconAreaWidth + extraWidth;
        float desiredHeight = lifeIcon.getGlobalBounds().height + extraHeight;

        sf::Sprite lifeBg;
        lifeBg.setTexture(Assets::getInstance().getTexture("lifebg"));
        sf::FloatRect bgBounds = lifeBg.getLocalBounds();
        lifeBg.setScale(desiredWidth / bgBounds.width, desiredHeight / bgBounds.height);
        lifeBg.setPosition(PADDING - 10.f, PADDING - 10.f);
        _game->window().draw(lifeBg);

        // Draw icons
        float iconX = PADDING + (extraWidth - 2 * PADDING) / 2.f;
        float iconY = PADDING + (extraHeight - 2 * PADDING) / 2.f;
        for (int i = 0; i < m_lives; ++i)
        {
            lifeIcon.setPosition(iconX, iconY);
            _game->window().draw(lifeIcon);
            iconX += lifeIcon.getGlobalBounds().width + SPACING;
        }
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
        _game->window().draw(drone.shadowSprite);
        _game->window().draw(drone.sprite);
        if (drone.state == DroneState::Firing)
        {
            _game->window().draw(drone.laserHitbox);
        }
    }

    if (m_paused)
    {
        // Draw semi-transparent overlay
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(_game->window().getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));  // Slightly transparent black
        _game->window().draw(overlay);

        // Draw "Paused" text
        sf::Text pauseText;
        pauseText.setFont(Assets::getInstance().getFont("main"));
        pauseText.setString("Resume (P)");
        pauseText.setCharacterSize(100);
        pauseText.setFillColor(sf::Color::Green);
        pauseText.setStyle(sf::Text::Bold);
        pauseText.setOutlineThickness(3.f);
        pauseText.setOutlineColor(sf::Color::Black);

        sf::FloatRect bounds = pauseText.getLocalBounds();
        pauseText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        pauseText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);

        _game->window().draw(pauseText);
    }



    if (m_showBoundingBoxes)
    {
        auto drawBounds = [&](const sf::FloatRect& bounds) {
            sf::RectangleShape rect;
            rect.setPosition(bounds.left, bounds.top);
            rect.setSize({ bounds.width, bounds.height });   
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(sf::Color::Cyan);  // Single color
            rect.setOutlineThickness(2.f);
            _game->window().draw(rect);
            };

        // Player leg box
        auto full = playerSprite.getGlobalBounds();
        const float legPortion = 0.25f;
        sf::FloatRect playerLegBox{
            full.left,
            full.top + full.height * (1.f - legPortion),
            full.width,
            full.height * legPortion
        };
        drawBounds(playerLegBox);

        // Enemy cars
        for (const auto& car : enemyCars) {

            const float bottomPortion = 0.6f;

            sf::FloatRect bounds = car.sprite.getGlobalBounds();
            float reducedHeight = bounds.height * bottomPortion;

            // Start Y from the bottom portion
            float offsetY = bounds.height * (1.f - bottomPortion);

            sf::FloatRect customBox(bounds.left, bounds.top + offsetY, bounds.width, reducedHeight);

            sf::RectangleShape debugRect;
            debugRect.setPosition(customBox.left, customBox.top);
            debugRect.setSize({ customBox.width, customBox.height });
            debugRect.setFillColor(sf::Color::Transparent);
            debugRect.setOutlineColor(sf::Color::Cyan);
            debugRect.setOutlineThickness(1.f);
            _game->window().draw(debugRect);
        }


        // Logs
        for (const auto& log : logs) {
            sf::FloatRect bounds = log.sprite.getGlobalBounds();

            const float heightReduction = 0.4f;  // Match update logic
            const float widthReduction = 0.1f;

            bounds.top += bounds.height * (heightReduction / 2.f);
            bounds.height *= (1.f - heightReduction);
            bounds.left += bounds.width * (widthReduction / 2.f);
            bounds.width *= (1.f - widthReduction);

            drawBounds(bounds);
        }


        // River enemies
        for (const auto& enemy : riverEnemies)
            drawBounds(enemy.sprite.getGlobalBounds());

        // Drone laser
        for (const auto& drone : drones)
        {
            if (drone.state == DroneState::Firing)
            {
                auto laserBounds = drone.laserHitbox.getGlobalBounds();
                const float killPortion = 0.10f;
                sf::FloatRect killBox{
                    laserBounds.left,
                    laserBounds.top + laserBounds.height * (1.f - killPortion),
                    laserBounds.width,
                    laserBounds.height * killPortion
                };
                drawBounds(killBox);
            }
        }
    }


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
        sf::Vector2u winSize = _game->window().getSize();

        // Draw a semi-transparent overlay
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(winSize));
        overlay.setFillColor(sf::Color(0, 0, 0, 200)); // More opaque than before
        _game->window().draw(overlay);

        // Draw the Final Message
        sf::Text finalText;
        finalText.setFont(Assets::getInstance().getFont("main"));
        finalText.setStyle(sf::Text::Bold);
        finalText.setOutlineThickness(4.f);
        finalText.setOutlineColor(sf::Color::Black);
        finalText.setCharacterSize(160);  // Double the normal size—for emphasis

        if (gameOver)
        {
            finalText.setString("Game Over");
            finalText.setFillColor(sf::Color::Red);
        }
        else
        {
            finalText.setString("You Won!");
            finalText.setFillColor(sf::Color::Yellow);
        }
        sf::FloatRect finalBounds = finalText.getLocalBounds();
        finalText.setOrigin(finalBounds.left + finalBounds.width / 2.f,
            finalBounds.top + finalBounds.height / 2.f);
        finalText.setPosition(winSize.x / 2.f, winSize.y / 3.f);
        _game->window().draw(finalText);

        // --- Draw the Selectable Options ---
        sf::Text selectionText;
        selectionText.setFont(Assets::getInstance().getFont("main"));
        selectionText.setStyle(sf::Text::Bold);
        selectionText.setOutlineThickness(1.5f);
        selectionText.setOutlineColor(sf::Color::Black);
        selectionText.setCharacterSize(80);

        std::string playOpt, menuOpt;
        if (finishOption == 0)
        {
            playOpt = "> Play Again <";
            menuOpt = "  Back To Menu";
        }
        else
        {
            playOpt = "  Play Again";
            menuOpt = "> Back To Menu <";
        }
        // Combine into a two-line string:
        std::string selectionStr = playOpt + "\n" + menuOpt;
        selectionText.setString(selectionStr);

        sf::FloatRect selBounds = selectionText.getLocalBounds();
        selectionText.setOrigin(selBounds.left + selBounds.width / 2.f,
            selBounds.top + selBounds.height / 2.f);
        selectionText.setPosition(winSize.x / 2.f, winSize.y * 0.55f);
        _game->window().draw(selectionText);

        sf::Text instructionsText;
        instructionsText.setFont(Assets::getInstance().getFont("main"));
        instructionsText.setStyle(sf::Text::Bold);
        instructionsText.setOutlineThickness(1.5f);
        instructionsText.setOutlineColor(sf::Color::Black);
        instructionsText.setCharacterSize(48);

        sf::FloatRect instrBounds = instructionsText.getLocalBounds();
        instructionsText.setOrigin(instrBounds.left + instrBounds.width / 2.f,
            instrBounds.top + instrBounds.height / 2.f);
        instructionsText.setPosition(winSize.x / 2.f, winSize.y * 0.7f);
        _game->window().draw(instructionsText);

        return;
    }

    //_game->window().display();
}




void Scene_Frogger::triggerDeath()
{
    if (m_playerAnimState == PlayerAnimState::Dying)
        return;     // already dying

    // lose one life
    --m_lives;

    // start dying animation
    m_playerAnimState = PlayerAnimState::Dying;
    m_animTimer = 0.f;
    m_dyingTotalTime = 0.f;
    m_dyingFrameIndex = 0;

    isJumping = false;
    verticalVelocity = 0.f;

    // start the respawn countdown (unless it was last life)
    if (m_lives > 0)
    {
        _waitingToRespawn = true;
        _respawnDelayTimer = 0.f;
    }
    else
    {
        gameOver = true;
    }
}



void Scene_Frogger::spawnPowerUp(const sf::Vector2f& position, float speed)
{
    PowerUp pu;
    pu.sprite.setTexture(Assets::getInstance().getTexture("powerup"));      // Use new sprite
    pu.sprite.setScale(1.0f, 1.0f);                                         // Tweak for visual size
    pu.sprite.setPosition(position);

    pu.speed = speed;
    pu.active = true;

    powerUps.push_back(pu);
}






void Scene_Frogger::spawnEnemyCar(const sf::Vector2f& position, float speed)
{
    EnemyCar car;

    // Define textures and whether each one faces right by default
    struct CarType {
        std::string textureName;
        bool facesRight;
    };

    const CarType carTypes[4] = {
        { "redcar",   false },   // redcar faces left by default
        { "bluecar",  false },   // bluecar faces left by default
        { "yellowcar", true },   // yellowcar faces right by default
        { "greencar",  true }    // greencar faces right by default
    };

    int carIndex = std::rand() % 4;
    const CarType& selectedCar = carTypes[carIndex];

    car.sprite.setTexture(Assets::getInstance().getTexture(selectedCar.textureName));

    // Set scale with perspective
    float scale = getPerspectiveScale(position.y);

    // Determine direction: Flip horizontally if speed and default facing don't align
    float scaleX = scale;
    if ((speed > 0 && !selectedCar.facesRight) || (speed < 0 && selectedCar.facesRight)) {
        scaleX = -scale;  // Flip horizontally
    }

    car.sprite.setScale(scaleX, scale); // Set both X and Y scale

    // Center the origin
    sf::FloatRect bounds = car.sprite.getLocalBounds();
    car.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    car.sprite.setPosition(position);
    car.speed = speed;

    enemyCars.push_back(car);
}




void Scene_Frogger::spawnLog(const sf::Vector2f& position, float speed)
{
    Log log;
    log.sprite.setTexture(Assets::getInstance().getTexture("raft"));  // Use raft texture
    log.originalY = position.y;

    // Scale with perspective
    float scale = getPerspectiveScale(position.y);
    log.sprite.setScale(scale, scale);

    // Center the log's origin
    sf::FloatRect bounds = log.sprite.getLocalBounds();
    log.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    log.sprite.setPosition(position);
    log.speed = speed;

    logs.push_back(log);
}

const sf::Vector2f Scene_Frogger::SAFE_RIVER_SCALE(1.5f, 1.5f);
const sf::Vector2f Scene_Frogger::RIVER_ENEMY_SCALE(1.5f, 1.5f);


void Scene_Frogger::spawnRiverEnemy(const sf::Vector2f& position, float speed)
{
    RiverEnemy enemy;
    enemy.sprite.setTexture(Assets::getInstance().getTexture("shark"));


    sf::FloatRect bounds = enemy.sprite.getLocalBounds();
    enemy.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    float scale = getPerspectiveScale(position.y);
    float scaleX = (speed > 0) ? -scale : scale;
    enemy.sprite.setScale(scaleX, scale);

    enemy.sprite.setPosition(position);
    enemy.originalY = position.y; 
    enemy.speed = speed;

    riverEnemies.push_back(enemy);
}



void Scene_Frogger::spawnDrone(const sf::Vector2f& position, float speed) {
    Drone drone;
    // Initialize shadow rectangle
    drone.shadowSprite.setTexture(Assets::getInstance().getTexture("shadow"));
    drone.shadowSprite.setOrigin(
        drone.shadowSprite.getLocalBounds().width / 2.f,
        drone.shadowSprite.getLocalBounds().height / 2.f
    );

    drone.sprite.setTexture(Assets::getInstance().getTexture("goSafe"));
    drone.sprite.setTextureRect(sf::IntRect(19, 81, 57, 68));
    sf::FloatRect bounds = drone.sprite.getLocalBounds();
    drone.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    drone.sprite.setPosition(position);
    drone.speed = speed;
    drone.sprite.setScale(scaleFactorX, scaleFactorY);
    const sf::Vector2u winSize = _game->window().getSize();
    drone.targetPos.x = static_cast<float>(std::rand() % winSize.x);
    drone.targetPos.y = static_cast<float>(std::rand() % winSize.y);

    std::cout << "Spawning drone at: " << position.x << ", " << position.y << std::endl;
    drones.push_back(drone);
}


void Scene_Frogger::sMovement(sf::Time dt)
{

}

void Scene_Frogger::sCollisions(sf::Time dt)
{
    // player leg bounds
    auto full = playerSprite.getGlobalBounds();
    const float legPortion = 0.25f;
    sf::FloatRect legBox{
        full.left,
        full.top + full.height * (1.f - legPortion),
        full.width,
        full.height * legPortion
    };

    bool collision = false;
    for (const auto& car : enemyCars)
    {
        sf::FloatRect carBounds = car.sprite.getGlobalBounds();
        const float carBottomPortion = 0.6f;
        float carReducedHeight = carBounds.height * carBottomPortion;
        float carOffsetY = carBounds.height * (1.f - carBottomPortion);

        sf::FloatRect carHitBox{
            carBounds.left,
            carBounds.top + carOffsetY,
            carBounds.width,
            carReducedHeight
        };

        if (legBox.intersects(carHitBox))
        {
            collision = true;
            break;
        }
    }


    if (!collision)
    {
        for (auto& enemy : riverEnemies)
        {
            if (legBox.intersects(enemy.sprite.getGlobalBounds()))
            {
                collision = true;
                break;
            }
        }
    }

    // Drone’s laser check
    for (auto& drone : drones)
    {
        if (drone.state == DroneState::Firing)
        {
            // only bottom 2% of the laser hurts
            auto fl = drone.laserHitbox.getGlobalBounds();
            const float killPortion = 0.02f;
            sf::FloatRect killBox{
                fl.left,
                fl.top + fl.height * (1.f - killPortion),
                fl.width,
                fl.height * killPortion
            };

            if (legBox.intersects(killBox))
            {
                collision = true;
                break;
            }
        }
    }
    if (collision && !m_playerIsHit)
    {
        triggerDeath();
    }
}



//void Scene_Frogger::update(sf::Time dt)
//{
//    
//}

void Scene_Frogger::killPlayer()
{
    if (m_lives > 0)
    {
        resetPlayer();
        m_playerIsHit = false;             // allow next death to register
        m_playerAnimState = PlayerAnimState::Idle;
    }
    else
    {
        std::cout << "No lives left. Game Over.\n";
        gameOver = true;
    }
}



void Scene_Frogger::sAnimation(sf::Time dt)
{
    static sf::IntRect idleFrame(12, 1, 28, 63);
    static sf::IntRect runningFrames[2] = {
        sf::IntRect(12, 1, 28, 63),
        sf::IntRect(50, 1, 28, 63)
    };
    static sf::IntRect jumpFrame(55, 0, 27, 64);
    static sf::IntRect slidingFrame(152,25, 38, 32);  // Adjust these coordinates to your sliding sprite area
    static sf::IntRect dyingFrames[2] = {
        sf::IntRect(103, 2, 28, 62),
        sf::IntRect(152, 25, 38, 32)
    };

    // Timers for running and dying animations.
    static float runAnimTimer = 0.f;
    static int runFrameIndex = 0;
    const float runFrameDuration = 0.1f;
    const float dyingFrameDuration = 0.1f;
    const float totalDyingDuration = 0.2f;

    switch (m_playerAnimState)
    {
    case PlayerAnimState::Idle:
        playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
        playerSprite.setTextureRect(idleFrame);
        break;

    case PlayerAnimState::Running:
        runAnimTimer += dt.asSeconds();
        if (runAnimTimer >= runFrameDuration)
        {
            runAnimTimer = 0.f;
            runFrameIndex = (runFrameIndex + 1) % 2;
        }
        playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
        playerSprite.setTextureRect(runningFrames[runFrameIndex]);
        break;

    case PlayerAnimState::Jumping:
        playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
        playerSprite.setTextureRect(jumpFrame);
        break;

    case PlayerAnimState::Sliding:
        playerSprite.setTexture(Assets::getInstance().getTexture("goSafe"));
        playerSprite.setTextureRect(slidingFrame);
        break;

    case PlayerAnimState::Dying:
        m_animTimer += dt.asSeconds();
        m_dyingTotalTime += dt.asSeconds();

        if (m_animTimer >= dyingFrameDuration)
        {
            m_animTimer = 0.f;
            m_dyingFrameIndex = (m_dyingFrameIndex + 1) % 2;
        }

        playerSprite.setTextureRect(dyingFrames[m_dyingFrameIndex]);

        if (m_dyingTotalTime >= totalDyingDuration)
        {
            if (!gameOver)
            {
                // respawn in‐place, with remaining lives intact
                resetPlayer();
                m_playerAnimState = PlayerAnimState::Idle;
            }
            else
            {
                // last life gone – we stay in Dying state
                // let your update() logic pick up gameOver==true and show the
                // Game Over screen/options.
            }
        }
        break;


    }
}
