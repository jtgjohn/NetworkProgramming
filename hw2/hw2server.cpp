#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

int main(int argc, char* argv[]) {

	printf("Test\n");

	std::string dictionary = argv[1];
	std::vector<std::string> wordsList;

	std::ifstream input(dictionary.c_str());

	if (!input) {
		std::cout << "Cannot open input file" << std::endl;
	}

	for (std::string line; getline(input, line);) {
		wordsList.push_back(line);
		//std::cout << line << std::endl;
	}

	std::cout << "END of file" << std::endl;
	//for (int i = 0; i < wordsList.size(); i++) {
	//	std::cout << wordsList[i];
	//}

	//int randomIndex = rand() % wordsList.size();

	//randomWord = wordsList[randomIndex];

	while(1) {
		int randomIndex = rand() % wordsList.size();

		std::string randomWord = wordsList[randomIndex];

		std::cout << randomWord << std::endl;

		//create a char vector from the chosen word
		std::vector<char> wordInfo;

		for (int i = 0; i < randomWord.size(); i++) {
			wordInfo.push_back(tolower(randomWord[i]));
		}

		std::cout << "Enter guesses" << std::endl;

		while(1) {
			
			std::vector<char> tempWord = wordInfo;

			std::string guess;

			std::cin >> guess;

			std::cout << "Guess " << guess << " " << guess.size() << std::endl;
			std::cout << std::endl;
			std::cout << "Word " << randomWord << " " << randomWord.size() << std::endl;

			if (guess.size() != randomWord.size()) {
				std::cout << "Wrong length" << std::endl;
				continue;
			}

			int numCorrect = 0;
			int numPlaced = 0;

			//get num correct
			for (int i = 0; i < guess.size(); i++) {
				std::vector<char>::iterator itr = std::find(tempWord.begin(), tempWord.end(), tolower(guess[i]));

				if (itr != tempWord.end()) {
					int loc = std::distance(tempWord.begin(), itr);
					tempWord.erase(tempWord.begin() + loc);
					numCorrect++;
				}
				//else the element wasn't found

			}

			//get the num placed
			for (int i = 0; i < guess.size(); i++) {
				if (tolower(guess[i]) == tolower(randomWord[i])) {
					numPlaced++;
				}
			}

			std::cout << "UName guessed " << guess << ": " << numCorrect << " letter(s) were correct and " << numPlaced << " letter(s) were correctly placed." << std::endl;

		}

	}

}