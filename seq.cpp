#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define DEFAULT_MAIN_FILE "main.txt"
#define DEFAULT_REMOVE_FILE "remove.txt"
#define DEFAULT_OUTPUT_FILE "output.txt"

void readFileToString(const std::string& filename, std::string& data) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        data = buffer.str();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::vector<std::string> readFileToVector(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    if (file.is_open()) {
        while (getline(file, line)) {
			line.pop_back(); // remove newline character
            lines.push_back(line);
        }
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    return lines;
}

bool strcmp_custom(const std::string& str1, const std::string& str2) {
	// std::cout << "" << "" << "str1: " << str1 << " str2: " << str2 << std::endl;
	for (size_t i = 0; i < str1.length(); ++i) {
		if (str1[i] != str2[i]) {
			return false;
		}
	}
	return true;
}

void removeWords(std::string& text, const std::vector<std::string>& words) {
    size_t textLen = text.length();
    for (size_t idx = 0; idx < textLen; ++idx) {
        for (const auto& word : words) {
            size_t wordLen = word.length();
			std::string analized_str = text.substr(idx, wordLen);
            if (idx + wordLen <= textLen && strcmp_custom(word, analized_str)) { 
				// std::cout << "Removing: " << word << " at " << idx << std::endl;
				text.replace(idx, wordLen, wordLen, ' ');
				idx += wordLen - 1;
				break; 
            }
        }
    }
}

int main(int argc, char* argv[]) {
    std::string mainFile = (argc > 1) ? argv[1] : DEFAULT_MAIN_FILE;
    std::string removeFile = (argc > 2) ? argv[2] : DEFAULT_REMOVE_FILE;
    std::string outputFile = (argc > 3) ? argv[3] : DEFAULT_OUTPUT_FILE;

    std::string text;
    readFileToString(mainFile, text);
    std::vector<std::string> removeWordsList = readFileToVector(removeFile);

    removeWords(text, removeWordsList);

    std::ofstream outFile(outputFile);
    if (outFile.is_open()) {
        outFile << text;
        outFile.close();
    } else {
        std::cerr << "Error opening output file: " << outputFile << std::endl;
    }

    return 0;
}
