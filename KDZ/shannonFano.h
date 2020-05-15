#pragma once
#include "coder.h"
#include "shannonFanoSymbol.h"
// fstream, vector, string уже подключён в coder.h

class ShannonFano : Coder
{
private:
	Symbol* _root;
	std::vector<Symbol*> _symbols;
	std::vector<std::string> _codes;
public:
	void encode(const std::string& inputFile, const std::string& outputFile) override;

	void build();

	void fano_recursive(int l, int r, Symbol* symbol);

	int find_med(int l, int r);

	void addChance(unsigned char ch, int chance);

	Symbol* recoverCoding();

	void decode(const std::string& inputFile, const std::string& outputFile);

	std::string toString() override;
};