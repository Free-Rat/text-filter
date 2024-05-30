#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>

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
            lines.push_back(line);
        }
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    return lines;
}

bool cpu_strncmp(const char* str1, const char* str2, int len) {
    for (int i = 0; i < len; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

void removeWords(const std::string& text, const std::vector<std::string>& removeWords, std::string& result) {
    int textLen = text.size();
    int numWords = removeWords.size();
    std::vector<int> wordLens(numWords);
    for (int i = 0; i < numWords; i++) {
        wordLens[i] = removeWords[i].length();
    }

    result = text;

    #pragma omp parallel for
    for (int idx = 0; idx < textLen; idx++) {
        for (int i = 0; i < numWords; i++) {
            int wordLen = wordLens[i];
            if (idx + wordLen <= textLen && cpu_strncmp(&text[idx], removeWords[i].c_str(), wordLen)) {
                for (int j = 0; j < wordLen; j++) {
                    result[idx + j] = ' ';
                }
                idx += wordLen - 1; // Move index to the end of the word
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
    std::vector<std::string> removeWords = readFileToVector(removeFile);

    std::string result;
    removeWords(text, removeWords, result);

    std::ofstream outFile(outputFile);
    if (outFile.is_open()) {
        outFile.write(result.c_str(), result.size());
        outFile.close();
    } else {
        std::cerr << "Error opening output file: " << outputFile << std::endl;
    }

    return 0;
}
