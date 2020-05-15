/*
// КДЗ по дисциплине Алгоритмы и структуры данных, 2019-2020 уч.год;
// Иванов Даниил, БПИ182, 07.04.2020;
// Среда разработки - Visual Studio 2017;
// Состав проекта: main.cpp, coder.h, shannonFanoSymbol.h, shannonFanoSymbol.cpp, shannonFano.h, shannonFano.cpp, lz77.h, lz77.cpp;
// Cделано:
	1. Сжатие и распаковка методом Шеннона - Фано,
	2. Сжатие и распаковка методом LZ77,
	3. Проведен вычислительный эксперимент, 
	4. Построены таблицы и графики,
	5. Оформлен отчёт.
	Для измерения времени выполнения использовалось chrono lib
// Не сделано:
	1. Сжатие и распаковка методом LZW(доп. задание)
*/

#include <iostream>
#include <fstream>
#include <chrono>
#include <Windows.h>

#include "coder.h"
#include "shannonFano.h"
#include "lz77.h"

const int bytes = 1024;
const std::string pathToTables = "Tables/";
const std::string pathToSamples = "DATA/Samples/";
const std::string pathToSource = "DATA/Sources/";
const std::vector<std::string> sourceFiles = { "1.bmp.BMP", "2.bwbmp.BMP", "3.bwjpg.jpg", "4.dll.DLL",
											   "5.docx.docx", "6.jpg.jpg", "7.mp3.mp3", "8.pdf.pdf",
											   "9.pptx.pptx", "10.rtf.rtf", "11.txt.txt" };

const int NUMBER_OF_TESTS = 10;

std::string replaceDots(double a)
{
	std::string s = std::to_string(a);
	for (char & i : s)
		if (i == '.')
			i = ',';
	return s;
}

void calculateFrequenciesAndEntropy()
{
	std::ifstream file;
	std::ofstream table;
	for (std::string fileName : sourceFiles)
	{
		std::vector<int> frequencies(UCHAR_MAX + 1, 0);
		file.open(pathToSource + fileName, std::ios::in | std::ios::ate | std::ios::binary);
		double fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		char symbol;
		while (file.get(symbol))
		{
			++frequencies[(unsigned char)symbol];
		}
		file.close();

		table.open(pathToTables + fileName.substr(0, fileName.rfind('.') + 1) + "csv", std::ios::out | std::ios::trunc);
		double entropy = 0;
		for (int i = 0; i < frequencies.size(); ++i)
		{
			double temp = frequencies[i] / fileSize;
			table << i << ';' << replaceDots(temp) << std::endl;
			if (frequencies[i] != 0)
				entropy -= temp * log2(temp);
		}
		table << "Entropy H" << ";" << replaceDots(entropy);
		table.close();
	}
}

void calculateCompression()
{
	std::ofstream tableSF(pathToTables + "SF.csv", std::ios::out);
	std::ofstream tableLZ775(pathToTables + "LZ775.csv", std::ios::out);
	std::ofstream tableLZ7710(pathToTables + "LZ7710.csv", std::ios::out);
	std::ofstream tableLZ7720(pathToTables + "LZ7720.csv", std::ios::out);
	for (std::string fileName : sourceFiles)
	{
		std::ifstream temp(pathToSource + fileName, std::ios::in | std::ios::ate);
		double originalSize = temp.tellg();
		temp.close();

		temp.open(pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "shan", std::ios::in | std::ios::ate);
		std::string s = pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "shan";
		tableSF << fileName << ';' << replaceDots(temp.tellg() / originalSize) << std::endl;
		temp.close();

		temp.open(pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "lz775", std::ios::in | std::ios::ate);
		tableLZ775 << fileName << ';' << replaceDots(temp.tellg() / originalSize) << std::endl;
		temp.close();

		temp.open(pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "lz7710", std::ios::in | std::ios::ate);
		tableLZ7710 << fileName << ';' << replaceDots(temp.tellg() / originalSize) << std::endl;
		temp.close();

		temp.open(pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "lz7720", std::ios::in | std::ios::ate);
		tableLZ7720 << fileName << ';' << replaceDots(temp.tellg() / originalSize) << std::endl;
		temp.close();
	}
	tableSF.close();
	tableLZ775.close();
	tableLZ7710.close();
	tableLZ7720.close();
}

void test(std::vector<Coder*> coders)
{
	for (Coder* coder : coders)
	{
		std::cout << coder->toString() << " compresses:" << std::endl;
		std::ofstream table;
		table.open(pathToTables + coder->toString() + ".csv");
		for (std::string fileName : sourceFiles)
		{
			std::cout << "\t" << fileName << std::endl;
			std::string pathToCompressed = pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + coder->toString();
			std::string pathToDecompressed = pathToSamples + fileName.substr(0, fileName.rfind('.') + 1) + "un" + coder->toString();
			std::chrono::steady_clock::time_point begin;
			std::chrono::steady_clock::time_point end;
			std::chrono::nanoseconds compressionTime{};
			std::chrono::nanoseconds decompressionTime{};
			table << fileName << ";";
			for (int i = 0; i < 1; ++i)
			{
				begin = std::chrono::steady_clock::now();
				coder->encode(pathToSource + fileName, pathToCompressed);
				end = std::chrono::steady_clock::now();
				compressionTime += end - begin;

				begin = std::chrono::steady_clock::now();
				coder->decode(pathToCompressed, pathToDecompressed);
				end = std::chrono::steady_clock::now();
				decompressionTime += end - begin;
			}
			std::string res = replaceDots(((double)compressionTime.count()) / NUMBER_OF_TESTS) + ";" +
				replaceDots(((double)decompressionTime.count()) / NUMBER_OF_TESTS);
		}
		table.close();
	}
}

int main()
{
	std::vector<Coder*> coders = { (Coder*) new LZ77(5 - 4, 4), (Coder*) new LZ77(10 - 8, 8), (Coder*) new LZ77(20 - 16, 16),
								   (Coder*) new ShannonFano() };
	test(coders);
	calculateFrequenciesAndEntropy();
	calculateCompression();
	std::cout << "The testing is over. Everything is OK";
	return 0;
}