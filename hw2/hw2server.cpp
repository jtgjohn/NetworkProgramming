#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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
	for (int i = 0; i < wordsList.size(); i++) {
		std::cout << wordsList[i];
	}

}