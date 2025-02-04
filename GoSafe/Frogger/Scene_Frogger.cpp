#include "Scene_Frogger.h"
#include "Scene_Menu.h"  // Ensure this is present in Scene_Frogger.cpp

#include <iostream>

Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
{
    init(levelPath);
}

void Scene_Frogger::init(const std::string& path)
{
    // Load the level from the provided file path
    loadLevel(path);

    // Load the background image
    loadBackground();

    // Register the Escape key to return to the menu
    registerAction(sf::Keyboard::Escape, "MENU");

    // Initialize game variables
    m_score = 0;
    m_lives = 3;
    m_maxHeight = 0;
}

void Scene_Frogger::loadBackground()
{
    // Attempt to load the background image
    if (!backgroundTexture.loadFromFile("../assets/Textures/background01.png")) {
        std::cerr << "Error: Could not load background image!\n";
    }
    backgroundSprite.setTexture(backgroundTexture);

    // Scale the background to fit the window
    backgroundSprite.setScale(
        _game->window().getSize().x / backgroundSprite.getLocalBounds().width,
        _game->window().getSize().y / backgroundSprite.getLocalBounds().height
    );
}

void Scene_Frogger::loadLevel(const std::string& path)
{
    // Placeholder: Read level file (to be implemented)
    std::cout << "Loading level from: " << path << std::endl;

    // Here, you would parse `path` to create lanes, obstacles, and player positions.
}

void Scene_Frogger::onEnd()
{
    std::cout << "Exiting Scene_Frogger..." << std::endl;
}

void Scene_Frogger::update(sf::Time dt)
{
    sMovement(dt);
    sCollisions();
    sUpdate(dt);
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

void Scene_Frogger::sRender()
{
    // Clear the window
    _game->window().clear();

    // Draw the background image
    _game->window().draw(backgroundSprite);

    // Placeholder: Draw entities, player, obstacles, UI elements, etc.
    // Example: _game->window().draw(m_player->getSprite());

    // Display the updated window
    _game->window().display();
}

// Movement logic (to be implemented)
void Scene_Frogger::sMovement(sf::Time dt)
{
    // Placeholder for player and entity movement logic
}

// Collision detection logic (to be implemented)
void Scene_Frogger::sCollisions()
{
    // Placeholder for collision logic with obstacles
}

// Updates the game state (to be implemented)
void Scene_Frogger::sUpdate(sf::Time dt)
{
    // Placeholder for updating score, lives, and progress
}

// Animation updates (to be implemented)
void Scene_Frogger::sAnimation(sf::Time dt)
{
    // Placeholder for handling animations
}
