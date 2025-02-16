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

    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

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

    void            resetPlayer();
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
