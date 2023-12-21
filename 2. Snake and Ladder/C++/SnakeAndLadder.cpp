#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <string>
#include <deque>

using namespace std;

class Dice{
public:
    int dices;
    Dice(){}

    Dice(int number){
        this->dices = number;
    }

    int throwDice(){
        int ans = 0;
        int diceCount = dices;
        while(diceCount-->0){
            int num;
            cout << "Enter dice number:";
            cin >> num;
            ans+= num;    // TODO : generate random number
        }
        return ans;
    }
};

class Cell{
public:
    bool isSnakeOrLadder;
    int jump;
    Cell(){
        this->isSnakeOrLadder = false;
    }
};

class Player{
public:
    string name;
    int position;
    Player(){}

    Player(string name){
        this->name = name;
        this->position = 0;
    }
};

class Board{
public:
    int size;
    vector<Cell> cells;
    Board(){}

    Board(int size){
        this->size = size;
        cells.resize(size+1);     // no special Cells
    }

    void addJump(int start, int end){
        cells[start].isSnakeOrLadder = true;
        cells[start].jump = end;
    }
    bool possibleToAddSnake(int start, int end){
        return(start>end && start>=1 && start<=size && end>=1 && end<=size);
    }
    bool possibleToAddLadder(int start, int end){
        return(start<end && start>=1 && start<=size && end>=1 && end<=size);
    }

    void addSnakesAndLadders(){
        int snakes;
        cout << "Enter number of snakes:";
        cin >> snakes;
        int snakesAdded = 0;
        do{
            int start, end;
            cout << "Enter the start of snake:" << endl;
            cin >> start;
            cout << "Enter the end of snake:" << endl;
            cin >> end;
            if(!possibleToAddSnake(start, end)){
                cout << "Invalid start and end for snake entered, Please enter again!!" << endl;
                continue;
            }
            addJump(start, end);
            snakesAdded+=1;

        }while(snakesAdded <snakes);

        int ladders;
        cout << "Enter number of ladders:" << endl;
        cin >> ladders;
        int laddersAdded = 0;
        do{
            int start, end;
            cout << "Enter the start of ladder:" << endl;
            cin >> start;
            cout << "Enter the end of ladder:" << endl;
            cin >> end;
            if(!possibleToAddLadder(start, end)){
                cout << "Invalid start and end for ladder entered, Please enter again!!" << endl;
                continue;
            }
            addJump(start, end);
            laddersAdded+=1;

        }while(laddersAdded <ladders);
    }

    void addSnakesAndLaddersOnBoard(){
        addSnakesAndLadders();
    }

    int moveToPos(int position){
        if(!(position>=1 && position<=size)){
            cout << "Wrong dice entered!" << endl;
            return -1;
        }
        int finalPos = position;
        if(cells[position].isSnakeOrLadder == true){
            finalPos = cells[position].jump;
        }
        return finalPos;
    }
};

class Game{
public:
    list<Player> players;
    Board board;
    Dice dice;
    int numPlayers;
    int size;

    Player winner;

    Game(int numPlayers, int size, int numDices){
        this->numPlayers = numPlayers;
        this->size = size;
        // Creating players
        for(int i=0; i<numPlayers; ++i){
            string name;
            cout << "Enter the name of player " << i+1 <<" :";
            cin >> name;
            players.push_back(Player(name));
        }

        // Creating Board
        board = Board(size);
        board.addSnakesAndLaddersOnBoard();

        // Creating Dice
        dice = Dice(numDices);
    }

    void startGame(){
        cout << "Game started" << endl;
        bool winnerFound = false;
        while(!winnerFound){
            Player curPlayer = players.front();
            players.pop_front();

            cout << "Player currently playing:" << curPlayer.name << endl;

            int move = dice.throwDice();
            int nextPos = curPlayer.position + move;

            if(nextPos > size){
                players.push_back(curPlayer);
                continue;
            }

            int finalPosition = board.moveToPos(nextPos);
            cout << "Next Position to go:" << finalPosition << endl;
            if(finalPosition == size){
                cout << "Winner found:" << curPlayer.name << endl;
                winner = curPlayer;
                winnerFound = true;
                continue;
            }

            curPlayer.position = finalPosition;
            players.push_back(curPlayer);
        }
    }

    string getWinner(){
        return winner.name;
    }
};

int main(){
    int players, size, numDices;
    cout << "Enter the no of players";
    cin >> players;
    cout << "Enter the size of board";
    cin >> size;
    cout << "Enter number of dices in game";
    cin >> numDices;

    Game game = Game(players, size, numDices);
    game.startGame();   // Game will always have a winner
    cout << "Winner of the game is:" << game.getWinner();

}