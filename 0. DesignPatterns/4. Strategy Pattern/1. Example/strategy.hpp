#ifndef STRATEGY_INTERFACE
#define STRATEGY_INTERFACE
// strategy class would be sort of an interface
// It only defined those methods which we want to implement for various different strategies
class strategyI{
public:
    virtual int meeleattack(int a, int b) = 0;
    virtual int rangeattack(int a, int b) = 0;
    virtual int getPlayingLevel() = 0; 
};

#endif