#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <assert.h>

using namespace std;

class Player{
private:
	string name;
	char piece;
public:	
	Player(string name, char piece){
		this->name = name;	this->piece = piece;
	}
	string getName(){
		return this->name;
	}
	char getPiece(){
		return this->piece;
	}
};

class Board{
private:
	vector<vector<char>> board;
	int size;
public:
	Board(){}

	Board(int size){
		this->size = size;
		// Creating Board
		board.resize(size, vector<char>(size, ' '));
	}

	bool isFull(){
		for(int i=0; i<size; ++i){
			for(int j=0; j<size; ++j){
				if(board[i][j] == ' '){
					return false;
				}
			}
		}
		return true;
	}

	bool isPossibleXY(int x, int y){
		return board[x][y] == ' ';
	}

	void insertXY(int x, int y, char piece){
		board[x][y] = piece;
	}

	char returnPiece(int x, int y){
		return board[x][y];
	}

	void printBoard(){
		cout << "Printing Board" << endl;
		for(int i=0; i<this->size; ++i){
			for(int j=0; j<this->size; ++j){
				cout << "| " << board[i][j];
			}
			cout << " |" << endl;
		}
	}

};

class Game{
private:
	int numPlayers;
	int boardSize;
	deque<Player> players;
	Board board;
	bool isWinner;
	string winnerName;
	
	bool isPlayerWinner(char piece){
		bool vertical = false;
		// for vertical
		for(int curCol=0; curCol<boardSize; ++curCol){
			bool tempVisited = true;
			for(int i=0; i<boardSize; ++i){
				if(board.returnPiece(i, curCol) != piece){
					tempVisited = false;
					break;
				}
			}
			if(tempVisited == true){
				vertical = true;
				break;
			}
		}

		if(vertical == true)	return true;

		bool horizontal = false;
		// for horizontal
		for(int curRow=0; curRow<boardSize; ++curRow){
			bool tempVisited = true;
			for(int j=0; j<boardSize; ++j){
				if(board.returnPiece(curRow, j) != piece){
					tempVisited = false;
					break;
				}
			}
			if(tempVisited == true){
				horizontal = true;
				break;
			}
		}

		if(horizontal == true)	return true;

		// for diagonal
		bool diagonal = true;
		for(int i=0; i<boardSize; ++i){
			if(board.returnPiece(i, i) != piece){
				diagonal = false;
				break;
			}
		}

		bool reverseDiagonal = true;
		for(int i=0; i<boardSize; ++i){
			if(board.returnPiece(i, boardSize-1-i) != piece){
				reverseDiagonal = false;
				break;
			}
		}

		return reverseDiagonal;
	}

public:
	Game(int numPlayers, int boardSize){
		this->numPlayers = numPlayers;
		this->boardSize = boardSize;

		// Creating players
		for(int i=0; i<numPlayers; ++i){
			string name;
			char symbol;
			cout << "Please input Player " << i+1 << " name:";
			cin >> name;
			cout << "Please input symbol:";
			cin >> symbol;
			players.push_back(Player(name, symbol));
		}

		// Creating board
		board = Board(boardSize);
	}

	bool startGame(){
		isWinner = true;
		while(isWinner){
			if(board.isFull() == true){
				isWinner = false;	// The game is tie
				break;
			}

			// print the board
			board.printBoard();

			Player curPlayer = players.front();
			players.pop_front();
			// Take coordinates from curPlayer
			int x, y;
			cout << "Enter the coordinates player:" << curPlayer.getName() << endl;
			cin >> x >> y;

			if(board.isPossibleXY(x, y) == false){
				cout << "Select correct coordinates"<<endl;
				players.push_front(curPlayer);
				continue;
			}else{	// Possible to add the coordinates
				board.insertXY(x, y, curPlayer.getPiece());
				players.push_back(curPlayer);

				bool isCurPlayerWinner = isPlayerWinner(curPlayer.getPiece());
				if(isCurPlayerWinner == true){
					winnerName = curPlayer.getName();
					break;
				}else{
					continue;
				}
			}
		}

		return isWinner;
	}

	string getWinnerName(){
		assert(isWinner);
		return this->winnerName;
	}
};

int main(){
	/*
	freopen("input.txt", "r", stdin);
	freopen("output.txt", "w", stdout);

	string name;
	cin >> name;
	cout << name;
	*/
	freopen("input.txt", "r", stdin);
	int totalPlayers, boardSize;
	cout << "Please enter number of players:";
	cin >> totalPlayers;
	cout << "Please enter board size";
	cin >> boardSize;

	Game game = Game(totalPlayers, boardSize);
	if(game.startGame() == true){
		cout << "Winner is:" << game.getWinnerName() << endl;
	}else{
		cout << "The game was a tie" << endl;
	}

}
