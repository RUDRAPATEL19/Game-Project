
#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"

enum class SignalState { Green, Yellow, Red };
enum class DroneState { Following, Charging, Firing, Cooldown };
enum class PlayerAnimState { Idle, Running, Jumping, Sliding, Dying };

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
    float originalY = 0.f;

};

struct Log {
    sf::Sprite sprite;
    float speed;
    float originalY = 0.f;

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
    sf::RectangleShape laserHitbox;
    sf::Sprite shadowSprite;
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

    bool isLaneClearForEnemyCar(float laneY, float spawnX, float clearance);
    bool isLaneClearForLog(float laneY, float spawnX, float clearance);
    bool isLaneClearForRiverEnemy(float laneY, float spawnX, float clearance);

    void spawnSafeRiver(const sf::Vector2f& position, float speed);

    std::string _levelPath;

    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

    bool    _waitingToRespawn = false;
    float   _respawnDelayTimer = 0.f;
    constexpr static float RESPWAN_DELAY = 2.f;

    sf::Vector2f startPosition; 
    int gameOverOption = 0;
    bool gameOver = false;
    bool isJumping = false;
    float jumpTimer = 0.f;            
    const float jumpDuration = 0.4f;    
    const float jumpHeight = 80.f;  
    const float jumpForward = 150.f;
    sf::Vector2f jumpStartPosition;
    
    int m_lives{ 3 };
    bool    m_playerIsHit = false;

    
    //void drawDebugLanes();
    bool m_showBoundingBoxes = false;


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
    //void            sUpdate(sf::Time dt);
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
    //void            playerMovement();
    //void            adjustPlayerPosition();
    //void            checkPlayerState();
    //void	        registerActions();
    //void            spawnPlayer(sf::Vector2f pos);

bool                gameFinished = false;        
int                 finishOption = 0;             


    void            killPlayer();
    //void            updateScore();

    void            triggerDeath();

    void            loadLevel(const std::string& path);
    void            loadBackground();

    //sf::FloatRect   getViewBounds();

public:
    Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath);

    PlayerAnimState m_playerAnimState = PlayerAnimState::Idle; // default state
    float m_animTimer = 0.f;          // used for dying animation
    int m_dyingFrameIndex = 0;        // Index for the dying animation frame
    float m_dyingTotalTime = 0.f; // Total time elapsed in dying state

    static const sf::Vector2f SAFE_RIVER_SCALE;
    static const sf::Vector2f RIVER_ENEMY_SCALE;

    float getPerspectiveScale(float y);
    float getPerspectiveScalePlayer(float y);
    float getPerspectiveScaleDrone(float y);

    unsigned int designWidth = 2560;
    unsigned int designHeight = 1600;
    float scaleFactorX = 2560.f / 480.f; // approx 5.33
    float scaleFactorY = 1600.f / 600.f; // approx 2.67
    float slideTimer = 0.f;
    const float slideDuration = 0.5f;
    int currentLevel;
    sf::Sprite finishLineSprite;

    bool jumpSoundPlayed = false;
    bool m_paused = false;

    void resetPlayer();
    sf::Vector2f originalStartPosition;

    void init(const std::string& path);
    void initTrafficSignals();
    void update(sf::Time dt) override;
    void handleDroneHit();
    void sDoAction(const Command& action) override;
    void sRender() override;
};
