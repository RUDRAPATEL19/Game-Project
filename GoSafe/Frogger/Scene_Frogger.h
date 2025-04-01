//
// Created by David Burchill on 2023-09-27.
//

#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"

enum class SignalState { Green, Yellow, Red };
enum class DroneState { Following, Charging, Firing, Cooldown };

struct EnemyCar {
    sf::Sprite sprite;
    float speed;
};

struct PowerUp {
    sf::Sprite sprite;
    float speed;
    bool active;
};

struct RiverEnemy {
    sf::Sprite sprite;
    float speed;
};

struct Log {
    sf::Sprite sprite;
    float speed;
};

struct TrafficSignal {
    sf::Sprite sprite;
    SignalState state;
    float stateDuration;  
    float stateTimer = 0.f;
    int sequenceOrder;     
    bool activated = false;  
};

struct Drone {
    sf::Sprite sprite;
    sf::Vector2f targetPos;
    float speed;
    DroneState state = DroneState::Following;
    float stateTimer = 0.f;
    // You can add a laser hitbox if desired:
    sf::RectangleShape laserHitbox; // optional, for visualization/collision
};





class Scene_Frogger : public Scene {

private:
    std::vector<Drone> drones;
    std::vector<TrafficSignal> trafficSignals;
    sf::Sprite playerSprite;
    std::vector<sf::Sprite> enemies;
    sf::Sprite enemySprite;

    std::vector<EnemyCar> enemyCars;
    std::vector<Log> logs;
    std::vector<RiverEnemy> riverEnemies;

    void spawnSafeRiver(const sf::Vector2f& position, float speed);

    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

    sf::Vector2f startPosition; 
    int gameOverOption = 0;
    bool gameOver = false;
    bool isJumping = false;
    float jumpTimer = 0.f;            
    const float jumpDuration = 0.4f;    
    const float jumpHeight = 40.f;  
    const float jumpForward = 70.f;
    sf::Vector2f jumpStartPosition;
    

    float verticalVelocity = 0.f;
    const float jumpSpeed = -350.f;
    const float gravity = 800.f;
    float groundY;

    bool inRiverLockMode = false;
    float riverLockTimer = 0.f;
    const float riverLockDuration = 10.f;

    float riverLogSpawnDelay = 2.f;
    float riverLogSpawnTimer = 0.f;

    bool isHopping = false;
    float hopTimer = 0.f;
    const float hopDuration = 0.4f;
    const float hopHeight = 30.f;

    bool onLog = false;           
    int currentLogIndex = -1;

    bool safePassageActivated = false;
    float safePassageTimer = 0.f;
    const float safePassageDuration = 5.f; // Safe passage lasts 3 seconds

    std::vector<PowerUp> powerUps;
    bool hasSafeRiver = false;

    bool			m_drawTextures{ true };
    bool			m_drawAABB{ false };
    bool			m_drawGrid{ false };
    sf::Text        m_text;
    sf::Time        m_timer;
    float           m_maxHeight;
    int             m_score;
    
    int             m_reachGoal;

    sf::Texture     backgroundTexture;
    sf::Sprite      backgroundSprite;
    void            sMovement(sf::Time dt);
    void            sCollisions(sf::Time dt);
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);
    void            spawnEnemyCar(const sf::Vector2f& position, float speed);
    void            spawnLog(const sf::Vector2f& position, float speed);
    void            spawnRiverEnemy(const sf::Vector2f& position, float speed);

    void spawnDrone(const sf::Vector2f& position, float speed);

    
    void            spawnPowerUp(const sf::Vector2f& position, float speed);
    void            safeRiver();

    float           safeRiverTimer = 0.f;        
    const           float safeRiverDuration = 7.f;
    float           safeRiverSpawnTimer = 0.f;
    float           safeRiverSpawnDelay = 0.f;
    void	        onEnd() override;
    void            playerMovement();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);

bool                gameFinished = false;        
int                 finishOption = 0;             
bool                puzzleCheckTriggered = false;


    void            killPlayer();
    void            updateScore();

    void            loadLevel(const std::string& path);
    void            loadBackground();

    sf::FloatRect   getViewBounds();

public:
    Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath);

    int m_lives;
    void resetPlayer();

    std::string getHoverMessage();

    void init(const std::string& path);
    void initTrafficSignals();
    void update(sf::Time dt) override;
    void handleDroneHit();
    void sDoAction(const Command& action) override;
    void sRender() override;
};
