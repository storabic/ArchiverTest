#pragma once
#include "coder.h"
#include <cmath>
// fstream, vector, string уже подключён в coder.h

class LZ77 : Coder
{
private:
	int _buffKB;
	int _dictKB;
	int _buffLength;
	int _dictLength;
	int _bitsPerLength;
	int _bitsPerOffset;

public:
	LZ77(int buff, int dict);

	//Префикс функция, ищет совпадения в словаре
	int* prefixFunction(int histBegin, int prevBegin, int prevEnd);

	//Ищет следующую строку
	void findNode(int& offset, int& length, char& symbol, int histBegin, int prevBegin, int prevEnd);

	//Пишет число в битовом формате
	void writeNumAsBits(int num, int bitsNumber, int& writePos, int& bitsLeftInChar);

	//Метод для выполнения кодировки
	void encode(const std::string& inputFile, const std::string& outputFile) override;

	//Непосредственно кодировка
	void lz77();

	//Считывание числа в битовом формате
	int readNumFromBits(int& curByte, int& curBit, int bitsInNum);

	//Непосредственно декодирование
	void decodeBytes();

	//Метод для выполнения декодирования
	void decode(const std::string& inputFileName, const std::string& outputFileName) override;

	//Получение информации об объекте
	std::string toString() override;
};