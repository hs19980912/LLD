#include "player.hpp"
#include "strategy.hpp"
#include "various_strategies.hpp"

int main(){
    level1strategy level1;
    level2strategy level2;

    Player p;
    p.setStrategy(&level1);

    p.printCurrentLevel();

    return 0;

}