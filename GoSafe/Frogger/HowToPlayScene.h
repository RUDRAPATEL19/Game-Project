#pragma once

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Leaf.h"

class HowToPlayScene : public Scene {
public:
    HowToPlayScene(GameEngine* gameEngine);

    virtual void update(sf::Time dt) override;
    virtual void sRender() override;
    virtual void sDoAction(const Command& action) override;
    virtual void onEnd() override;

private:
    std::vector<Leaf> fallingLeaves;

    void initLeaves();
    void updateLeaves(sf::Time dt);
    void renderLeaves();
};
