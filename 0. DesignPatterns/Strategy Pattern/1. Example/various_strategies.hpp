#include "strategy.hpp"
#ifndef STRATEGIES
#define STRATEGIES
class level1strategy: public strategyI{
public:
    int meeleattack(int a, int b) override{
        return 1 * (a + b);
    }
    int rangeattack(int a, int b) override{
        return 1 * (a + b);
    }
    int getPlayingLevel() override{
        return 1;
    }
};

class level2strategy: public strategyI{
public:
    int meeleattack(int a, int b) override{
        return 2 * (a + b);
    }
    int rangeattack(int a, int b) override{
        return 2 * (a + b);
    }
    int getPlayingLevel() override{
        return 2;
    }
};

class level3strategy: public strategyI{
public:
    int meeleattack(int a, int b) override{
        return 3 * (a + b);
    }
    int rangeattack(int a, int b) override{
        return 3 * (a + b);
    }
    int getPlayingLevel() override{
        return 3;
    }
};

#endif