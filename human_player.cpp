/*
 * Filename: human_player.cpp
 */

#include "human_player.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int str2uint(string str, unsigned &r) {
	int t;	
	r = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			t = r * 10 + str[i] - '0';
			if (t < r) {
				cout << "Input overflow!" << endl;
				return -1;
			}
			r = t;	
		} else {
			return -1;
		}
	}
	return 0;
}
vector <string> split(string str, char delim) {
	vector <string> splitStrings;
	size_t curr, prev = 0;
	curr = str.find(delim);
	while (curr != string::npos) {
		splitStrings.push_back(str.substr(prev, curr - prev));
		prev = curr + 1;
		curr = str.find(delim, prev);
	}
	splitStrings.push_back(str.substr(prev, curr - prev));
	return splitStrings;
}

Action HumanPlayer::Act(GameState game_state) {
	Action action_to_return;
	string buff;
	vector <string> splitStrings;
	int count;
	unsigned amount;
	
	while (1) {
		cout << "Please enter command: f(fold), c(check), r <NUM> (raise <amount>)" 
				<< endl;
		getline(cin, buff);
		count = 0;
		splitStrings = split(buff, ' ');

		// delete empty spaces
		while (count < splitStrings.size()) {
			if (splitStrings[count] != "") {
				//cout << splitStrings[count] << endl;
				count++;
			} else {
				splitStrings.erase(splitStrings.begin() + count);
			}
		}

		if (count == 0 || count > 2) {
			cout << "Invalid input!" << endl;
			continue;
		}
		
		if (count == 1) {
			if (splitStrings[0] == "f") {
				action_to_return.action = 0;
				return action_to_return;
			}
			if (splitStrings[0] == "c") {
				action_to_return.action = 1;
				return action_to_return;
			}
			cout << "Invalid input!" << endl;
			continue;
		}

		if (count == 2) {
			if (splitStrings[0] != "r") {
				cout << "Invalid input!" << endl;
				continue;
			}
			if (str2uint(splitStrings[1], amount) < 0) {
				cout << "Invalid input!" << endl;
				continue;
			}
			action_to_return.action = 2;
			action_to_return.amount = amount;
			return action_to_return;
		}
		cerr << "Program should not reach here!" << endl;
	}		
}