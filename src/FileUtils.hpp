#ifndef FILEUTILS_HPP
#define FILEUTILS_HPP

#include "Common.hpp"

bool getFileSize(FILE* f, size_t* pSize);
bool getFileSize(const std::string& fileName, size_t* pSize);

bool readFile(FILE* f, std::vector<uint8_t>* pData);
bool readFile(FILE* f, std::vector<char>* pData);
bool readFile(FILE* f, std::string* pData);

bool readFile(const std::string& fileName, std::vector<uint8_t>* pData);
bool readFile(const std::string& fileName, std::vector<char>* pData);
bool readFile(const std::string& fileName, std::string* pData);

bool writeFile(FILE* f, const std::vector<uint8_t>& data);
bool writeFile(FILE* f, const std::vector<char>& data);
bool writeFile(FILE* f, const std::string& data);

bool writeFile(const std::string& fileName, const std::vector<uint8_t>& data);
bool writeFile(const std::string& fileName, const std::vector<char>& data);
bool writeFile(const std::string& fileName, const std::string& data);

void getLines(const std::vector<uint8_t>& data, std::vector<std::string>* pLines);
void getLines(const std::vector<char>& data, std::vector<std::string>* pLines);
void getLines(const std::string& data, std::vector<std::string>* pLines);

bool readLines(FILE* f, std::vector<std::string>* pLines);
bool readLines(const std::string& fileName, std::vector<std::string>* pLines);

#endif
