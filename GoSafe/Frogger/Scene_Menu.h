#pragma once

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Leaf.h"


class Scene_Menu : public Scene
{
private:
    std::vector<std::string> _menuStrings; 
    std::vector<std::string> _levelPaths;     
    int _menuIndex;
    bool m_firstFrame = true;
    void buildMenu();
    unsigned int titleFontSize;
    unsigned int optionFontSize;
    unsigned int infoFontSize;

    std::string _title;

    // Falling leaves effect.
    std::vector<Leaf> fallingLeaves;
    void initLeaves();
    void updateLeaves(sf::Time dt);
    void renderLeaves();

    void init();
    virtual void onEnd() override;

public:
    Scene_Menu(GameEngine* gameEngine);
    void onBegin(); 
    virtual void update(sf::Time dt) override;
    virtual void sRender() override;
    virtual void sDoAction(const Command& action) override;
};
