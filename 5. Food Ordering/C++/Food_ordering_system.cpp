// https://leetcode.com/discuss/study-guide/3231299/Secret-to-cracking-Machine-Coding-Rounds
// https://leetcode.com/discuss/interview-experience/3228851/Flipkart-SDE-2-Feb-2023
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
using namespace std;

class Menu{
	map<string, int> menu;
public:
	Menu(){
	}

	void printMenu(){
		auto it = menu.begin();
		while(it!=menu.end()){
			cout << it->first << "\t\t:" << it->second << endl;
		}
	}

	bool updateMenu(string dishName, int price){
		bool isPresent = menu.count(dishName);
		if(isPresent == false){
			cout << "WARNING: Dish not present in the menu, update valid dish" << endl
			return false;
		}
		// When updaing is possible
		menu[dishName] = price;
		return true;
	}

	bool addToMenu(string dishName, int price){
		bool isPresent = menu.count(dishName);
		if(isPresent == true){
			cout << "WARNING: Dish already present, please add non existing dish" << endl;
			return false;
		}
		// Adding possible
		menu[dishName] = price;
		return true;
	}
};

class Order{

};

class Resturant{
	string name;
	Menu menu;
	set<Order> currentOrders;
	set<Order> pendingOrders;
	int MAX_LIMIT;
	int cur_limit;

public:
	Resturant(){}

	bool add_to_menu(string dishName, int price){
		return menu.addToMenu(dishName, price);
	}

	bool update_in_menu(string dishName, int price){
		return menu.updateMenu(dishName, price);
	}
};

class ResturantMap{
	static unordered_map<string, Resturant> resturantMap;
public:
	ResturantMap(){}

	void initiate(){
		int n;
		cout << "Number of resturant:" << endl;
		cin >> n;

		for(int i=0; i<n; ++i){
			cout << "Name:" << endl;
			cin >> name;
			cout << 
		}		
	}
};

unordered_map<string, Resturant> ResturantMap::resturantMap;

class OrderHelper{
protected:
    // return the name of the resturant to be selected for placing the order
    string virtual select_resturant_by_strategy() = 0;
};

class OrderByRating:public OrderHelper{
public:
    string select_resturant_by_strategy(vector<Resturant>& resVec){
        int maxRating = 0;
        string resturantSelectedName;
        for(int i=0; i<resVec.size(); ++i){
            Resturant& curRes = resVec[i];
            int currentRating = curRes.getRating();
            if(currentRating > maxRating){
            	resturantSelectedName = curRes.getName();
            	maxRating = currentRating;
            }
        }
        return resturantSelectedName;
    }
};

class OrderByPrice:public OrderHelper{
public:
    string select_resturant_by_strategy(vector<Resturant>& resVec){
        int maxRating = 0;
        string resturantSelectedName;
        for(int i=0; i<resVec.size(); ++i){
            Resturant& curRes = resVec[i];
            int currentRating = curRes.getRating();
            if(currentRating > maxRating){
            	resturantSelectedName = curRes.getName();
            	maxRating = currentRating;
            }
        }
        return resturantSelectedName;
    }
}

class OrderManager{
	ResturantMap resturantMap;

    void place_single_order(){
        string dishName;
        string userName;
        string selection;
        OrderHelper* newOrder = NULL;
        switch (selection){
            case "LOWEST":
                curOrderSelector = new OrderHelper();
                break;
            default:
                break;
        }
        
        resNameForOrderPlacing = curOrderSelector->select_resturant_by_strategy();
        
    }


public:
	OrderManager(){}

	void place_orders(){
		int n;
		cout << "Number of orders to be placed" << endl;
		cin >> n;

		for(int i=1; i<=n; ++i){
			place_single_order();
		}
	}

};

class ResturantManager{
	ResturantMap resturantMap;

public:
	ResturantManager(){}

	void add_resturant_menu(){
		string resName;
		string dishName;
		int price;
		bool added = false;
		do{
			cout << "Resturant name:" << endl;
			cin >> resName;		// Assumes restraunt is ALWAYS present
			cout << "dish name:" << endl;
			cin >> dishName;
			cout << "Price:" << endl;
			cin >> price;
			if(resturantMap[resName].add_to_menu(dishName, price)){
				added = true;
			}
		}while(added != true);
	}

	void update_resturant_menu(){
		string resName;
		string dishName;
		int price;
		bool updated = false;
		do{
			cout << "Resturant name:" << endl;
			cin >> resName;		// Assumes restraunt is ALWAYS present
			cout << "dish name:" << endl;
			cin >> dishName;
			cout << "Price:" << endl;
			cin >> price;
			if(resturantMap[resName].update_in_menu(dishName, price)){
				updated = true;
			}
		}while(updated != true);
	}
};

class Application{
	ResturantMap resturantMap;
	ResturantManager resturantManager;
	OrderManager orderManager;

	void initiate(){
		cout << "Application initiating" << endl;
		
		while(true){
			int input;
			cout << "Enter the input:" << cout;
			cin >> input;
			switch (input){
			case 1:
				resturantManager.add_resturant_menu();
				break;
			case 2:
				resturantManager.update_resturant_menu();
				break;
			case 3:
				orderManager.place_orders();
				break;
			case 4:
				// manager.update_order_status();
				break;
			}

		}
	}

public:
	Application(){
		initiate();	// onboarding
	}
};

int main(){
	Application application;

}