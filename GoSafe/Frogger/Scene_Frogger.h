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


struct EnemyCar {
    sf::Sprite sprite;
    float speed; // positive: moves right; negative: moves left
};

struct PowerUp {
    sf::Sprite sprite;
    float speed;    // If you want it to move (for now, it can be 0)
    bool active;    // To track if it’s still available
};


struct Log {
    sf::Sprite sprite;
    float speed;
};

struct RiverEnemy {
    sf::Sprite sprite;
    float speed;
};

class Scene_Frogger : public Scene {

private:

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

    bool isHopping = false;
    float hopTimer = 0.f;
    const float hopDuration = 0.4f;
    const float hopHeight = 30.f;

    bool onLog = false;           
    int currentLogIndex = -1;

    std::vector<PowerUp> powerUps;
    bool hasSafeRiver = false;

    bool			m_drawTextures{ true };
    bool			m_drawAABB{ false };
    bool			m_drawGrid{ false };

    sf::Text        m_text;
    sf::Time        m_timer;
    float           m_maxHeight;
    int             m_score;
    int             m_lives;
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

    void            resetPlayer();
    void            spawnPowerUp(const sf::Vector2f& position, float speed);
    void            safeRiver();

    float           safeRiverTimer = 0.f;        // Timer tracking how long the power-up remains active
    const           float safeRiverDuration = 7.f; // Duration (in seconds) for which the power-up is available
    float           safeRiverSpawnTimer = 0.f;  // Timer for spawning the power-up
    float           safeRiverSpawnDelay = 0.f;  // Random delay between spawns (in seconds)
    void	        onEnd() override;
    void            playerMovement();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);

    void            spawnLane1();
    void            spawnLane2();
    void            spawnLane3();
    void            spawnLane4();
    void            spawnLane5();
    void            spawnLane6();
    void            spawnLane7();
    void            spawnLane8();
    void            spawnLane9();
    void            spawnLane10();
    void            spawnGoal();
    void            spawnLives();

    void            killPlayer();
    void            updateScore();

    void            loadLevel(const std::string& path);
    void            loadBackground();

    sf::FloatRect   getViewBounds();

public:
    Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath);

    void init(const std::string& path);
    void update(sf::Time dt) override;
    void sDoAction(const Command& action) override;
    void sRender() override;
};
