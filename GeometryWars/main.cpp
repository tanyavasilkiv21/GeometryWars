#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include "Game.h"

int main() 
{
    std::srand(static_cast<unsigned int>(std::time(0)));
    Game g("config.txt");
    g.run();
    return 0;
}