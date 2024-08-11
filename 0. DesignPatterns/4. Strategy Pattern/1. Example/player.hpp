#include "strategy.hpp"
#include "various_strategies.hpp"
#include <cstddef>
#include <iostream>

using namespace::std;

#ifndef PLAYER
#define PLAYER
class Player{
public:
    Player(){
        strategy = NULL;
    }
    void setStrategy(strategyI* curStrategy){
        this->strategy = curStrategy;
    }

    int playMeeleAttack(int a, int b){
        return strategy->meeleattack(a, b);
    }

    int playRangeAttack(int a, int b){
        return strategy->rangeattack(a, b);
    }

    void printCurrentLevel(){
        int curLevel = strategy->getPlayingLevel();
        cout << "current level: " << curLevel << endl;
    }
private:
    strategyI* strategy;    
};

#endif