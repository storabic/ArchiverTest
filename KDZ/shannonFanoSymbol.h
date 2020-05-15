#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

class Symbol
{
private:
	int _amount;
	unsigned char _char;
	char _bit;
	std::string _code;
	Symbol* _left;
	Symbol* _right;
public:

	Symbol()
	{
		_code = "";
		_left = nullptr;
		_right = nullptr;
	}

	Symbol(char bit) : Symbol()
	{
		_bit = bit;
	}

	Symbol(char ch, int number) : Symbol()
	{
		_char = ch;
		_amount = number;
	}

	~Symbol()
	{
		delete _left;
		delete _right;
	}

	Symbol* getLeft()
	{
		return _left;
	}

	void setLeft(Symbol* left)
	{
		_left = left;
	}

	Symbol* getRight()
	{
		return _right;
	}

	void setRight(Symbol* right)
	{
		_right = right;
	}

	unsigned char getChar()
	{
		return _char;
	}

	void setChar(unsigned char ch)
	{
		_char = ch;
	}

	int getAmount()
	{
		return _amount;
	}

	void addToCode(char c)
	{
		_code += c;
	}

	void buildCodes(std::ofstream& file, std::vector<std::string>& codes, std::string code);
};