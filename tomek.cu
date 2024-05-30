#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cuda_runtime.h>

#define DEFAULT_MAIN_FILE "main.txt"
#define DEFAULT_REMOVE_FILE "remove.txt"
#define DEFAULT_OUTPUT_FILE "output.txt"

__device__ bool device_strncmp(const char* str1, const char* str2, int len) {
    for (int i = 0; i < len; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

__global__ void removeWordsKernel(const char* text, const char** words, const int* wordLens, int numWords, int textLen, char* result) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < textLen) {
        bool isWord = false;
        for (int i = 0; i < numWords; i++) {
            int wordLen = wordLens[i];
            if (idx + wordLen <= textLen && device_strncmp(&text[idx], words[i], wordLen)) {
                for (int j = 0; j < wordLen; j++) {
                    result[idx + j] = ' ';
                }
                idx += wordLen - 1; // Move index to the end of the word
                isWord = true;
                break;
            }
        }
        if (!isWord) {
            result[idx] = text[idx];
        }
    }
}

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

int main(int argc, char* argv[]) {
    std::string mainFile = (argc > 1) ? argv[1] : DEFAULT_MAIN_FILE;
    std::string removeFile = (argc > 2) ? argv[2] : DEFAULT_REMOVE_FILE;
    std::string outputFile = (argc > 3) ? argv[3] : DEFAULT_OUTPUT_FILE;

    std::string text;
    readFileToString(mainFile, text);
    std::vector<std::string> removeWords = readFileToVector(removeFile);

    int textLen = text.size();
    int numWords = removeWords.size();
    std::vector<int> wordLens(numWords);

    for (int i = 0; i < numWords; i++) {
        wordLens[i] = removeWords[i].length();
    }

    // Prepare text and words for GPU
    char* d_text;
    char** d_words;
    int* d_wordLens;
    char* d_result;
    cudaMalloc((void**)&d_text, textLen * sizeof(char));
    cudaMalloc((void**)&d_words, numWords * sizeof(char*));
    cudaMalloc((void**)&d_wordLens, numWords * sizeof(int));
    cudaMalloc((void**)&d_result, textLen * sizeof(char));

    cudaMemcpy(d_text, text.c_str(), textLen * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(d_wordLens, wordLens.data(), numWords * sizeof(int), cudaMemcpyHostToDevice);

    // Allocate and copy words to device
    char** h_words = (char**)malloc(numWords * sizeof(char*));
    for (int i = 0; i < numWords; i++) {
        cudaMalloc((void**)&h_words[i], wordLens[i] * sizeof(char));
        cudaMemcpy(h_words[i], removeWords[i].c_str(), wordLens[i] * sizeof(char), cudaMemcpyHostToDevice);
    }
    cudaMemcpy(d_words, h_words, numWords * sizeof(char*), cudaMemcpyHostToDevice);

    // Initialize result on the device with the original text
    cudaMemcpy(d_result, d_text, textLen * sizeof(char), cudaMemcpyDeviceToDevice);

    int blockSize = 256;
    int numBlocks = (textLen + blockSize - 1) / blockSize;

    removeWordsKernel<<<numBlocks, blockSize>>>(d_text, (const char**)d_words, d_wordLens, numWords, textLen, d_result);

    cudaDeviceSynchronize(); // Ensure the kernel has completed

    char* result = new char[textLen + 1];
    result[textLen] = '\0'; // Null-terminate the result string
    cudaMemcpy(result, d_result, textLen * sizeof(char), cudaMemcpyDeviceToHost);

    std::ofstream outFile(outputFile);
    if (outFile.is_open()) {
        outFile.write(result, textLen);
        outFile.close();
    } else {
        std::cerr << "Error opening output file: " << outputFile << std::endl;
    }

    // Free allocated memory
    cudaFree(d_text);
    cudaFree(d_words);
    cudaFree(d_wordLens);
    cudaFree(d_result);
    for (int i = 0; i < numWords; i++) {
        cudaFree(h_words[i]);
    }
    free(h_words);
    delete[] result;

    return 0;
}
