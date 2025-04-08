#ifndef LEAF_H
#define LEAF_H

#include <SFML/Graphics.hpp>

struct Leaf {
    sf::Sprite sprite;
    float fallSpeed;
    float drift; // horizontal drift speed (can be negative)
};

#endif // LEAF_H
