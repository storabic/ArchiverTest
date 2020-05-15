#pragma once
#include <vector>
#include <fstream>
#include <string>

class Coder
{
protected:
	std::ifstream _input;
	std::ofstream _output;
	char* _readBuffer;
	char* _writeBuffer;

public:
	const int DEFAULT_BUFFER_SIZE = 2 << 20;

    virtual void encode(const std::string& inputFile, const std::string& outputFile) = 0;

	virtual void decode(const std::string& inputFile, const std::string& outputFile) = 0;

	virtual std::string toString() = 0;
};