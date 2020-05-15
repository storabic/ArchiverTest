#include "shannonFano.h"

void ShannonFano::encode(const std::string& inputFile, const std::string& outputFile)
{
	std::vector<int> frequencies(UCHAR_MAX + 1, 0);
	_input.open(inputFile, std::ios::in | std::ios::binary | std::ios::ate);
	long long size = _input.tellg();
	_input.seekg(0, std::ios::beg);
	char symbol;
	while (_input.get(symbol))
	{
		++frequencies[(unsigned char)symbol];
	}
	for (int i = 0; i < frequencies.size(); ++i)
		if (frequencies[i] != 0)
			addChance((unsigned char)i, frequencies[i]);
	build();
	_input.clear();
	_input.seekg(0, std::ios::beg);
	_output.open(outputFile, std::ios::out | std::ios::trunc | std::ios::binary);
	_output << size << ';' << _symbols.size();
	_codes = std::vector<std::string>(UCHAR_MAX + 1, "");
	_root->buildCodes(_output, _codes, "");
	int readBufferSize = size < DEFAULT_BUFFER_SIZE ? size : DEFAULT_BUFFER_SIZE;
	char* readBuffer = new char[readBufferSize];
	char* writeBuffer = new char[DEFAULT_BUFFER_SIZE];
	int writePos = 0;
	int bitsCap = CHAR_BIT;
	while (size > 0)
	{
		_input.read(readBuffer, readBufferSize);
		for (int readPos = 0; readPos < readBufferSize; ++readPos)
		{
			unsigned char currentChar = (unsigned char)readBuffer[readPos];
			for (int codePos = 0; codePos < _codes[currentChar].length(); ++codePos)
			{
				if (writePos < DEFAULT_BUFFER_SIZE)
				{
					writeBuffer[writePos] <<= 1;
					if (_codes[currentChar][codePos] == '1')
						writeBuffer[writePos] |= 1;
					--bitsCap;
					if (bitsCap == 0)
					{
						bitsCap = CHAR_BIT;
						++writePos;
					}
				}
				else
				{
					_output.write(writeBuffer, writePos);
					writePos = 0;
				}
			}
		}
		readBufferSize = size < readBufferSize ? size : readBufferSize;
		size -= readBufferSize;
	}
	_input.close();
	if (bitsCap > 0)
	{
		writeBuffer[writePos] <<= bitsCap;
		_output.write(writeBuffer, writePos + 1);
	}
	_output.close();
	delete[] readBuffer;
	delete[] writeBuffer;
}

void ShannonFano::build()
{
	delete _root;
	_root = new Symbol();
	fano_recursive(0, _symbols.size() - 1, _root);
}

void ShannonFano::fano_recursive(int l, int r, Symbol* symbol)
{
	if (l >= r)
	{
		symbol->setChar(_symbols[r]->getChar());
		symbol->setRight(nullptr);
		symbol->setLeft(nullptr);
		return;
	}
	int m = find_med(l, r);
	for (int i = l; i <= m; ++i)
		_symbols[i]->addToCode('0');
	for (int i = m + 1; i <= r; ++i)
		_symbols[i]->addToCode('1');

	Symbol* left = new Symbol('0');
	Symbol* right = new Symbol('1');
	symbol->setLeft(left);
	symbol->setRight(right);
	fano_recursive(l, m, left);
	fano_recursive(m + 1, r, right);
}

int ShannonFano::find_med(int l, int r)
{
	int sum_from_left = 0;
	for (int i = l; i < r; ++i)
		sum_from_left += _symbols[i]->getAmount();
	int sum_from_right = _symbols[r]->getAmount();
	int m = r;
	int d;
	do
	{
		d = sum_from_left - sum_from_right;
		--m;
		sum_from_left -= _symbols[m]->getAmount();
		sum_from_right += _symbols[m]->getAmount();
	} while (abs(sum_from_left - sum_from_right) <= d);
	return m;
}

void ShannonFano::addChance(unsigned char ch, int chance)
{
	_symbols.push_back(new Symbol(ch, chance));
}

Symbol* ShannonFano::recoverCoding()
{
	char ch;
	char bit;
	_input >> ch >> bit;
	Symbol* newSymbol = new Symbol(bit);
	if (ch == 'N')
	{
		newSymbol->setLeft(recoverCoding());
		newSymbol->setRight(recoverCoding());
	}
	else
	{
		_input.get(ch);
		newSymbol->setChar((unsigned char)ch);
	}
	return newSymbol;
}

void ShannonFano::decode(const std::string& inputFile, const std::string& outputFile)
{
	long long size;
	char trash;
	int numberOfChars;
	_input.open(inputFile, std::ios::in | std::ios::binary);
	_input >> size >> trash >> numberOfChars;
	_root = recoverCoding();
	Symbol* node = _root;

	//CHAR_BIT - 7
	int shift = 7;
	long long readBufferSize = size < DEFAULT_BUFFER_SIZE ? size : DEFAULT_BUFFER_SIZE;
	_readBuffer = new char[readBufferSize];
	_writeBuffer = new char[DEFAULT_BUFFER_SIZE];
	long long begin = _input.tellg();
	_input.seekg(0, std::ios::end);
	long long end = _input.tellg() - begin;
	_input.seekg(begin, std::ios::beg);
	int readPos = readBufferSize;
	int writePos = 0;
	_output.open(outputFile, std::ios::out | std::ios::binary);
	while (size > 0)
	{
		if (readPos >= readBufferSize)
		{
			readBufferSize = (end < readBufferSize) ? end : readBufferSize;
			end -= readBufferSize;
			_input.read(_readBuffer, readBufferSize);
			readPos = 0;
		}
		if (writePos >= DEFAULT_BUFFER_SIZE)
		{
			_output.write(_writeBuffer, writePos);
			writePos = 0;
		}

		if (shift < 0)
		{
			shift = 7;
			++readPos;
		}
		else
		{
			if ((_readBuffer[readPos] & (1 << shift)) == 0)
				node = node->getLeft();
			else
				node = node->getRight();
			--shift;
			if (!node->getLeft())
			{
				_writeBuffer[writePos] = node->getChar();
				++writePos;
				--size;
				node = _root;
			}
		}
	}
	if (writePos != 0)
		_output.write(_writeBuffer, writePos);
	_input.close();
	_output.close();
	delete[] _readBuffer;
	delete[] _writeBuffer;
}

std::string ShannonFano::toString()
{
	return "shan";
}