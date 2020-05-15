#include "lz77.h"

LZ77::LZ77(int buff, int dict)
{
	_buffKB = buff;
	_dictKB = dict;
	if (buff < 1 || dict < 1)
		throw std::exception();
	_buffLength = buff * 1024 - 1;
	_dictLength = dict * 1024 - 1;
	//Устанавливаем количество битов на запись элементов тройки
	_bitsPerLength = log2(buff) + 1;
	_bitsPerOffset = log2(dict) + 1;
}

int* LZ77::prefixFunction(int histBegin, int prevBegin, int prevEnd)
{
	int prevBufLength = prevEnd - prevBegin + 1;
	int histBufLength = prevBegin - histBegin;
	int size = 2 * prevBufLength + histBufLength + 1;
	int* maxPrefixLengths = new int[size];
	maxPrefixLengths[0] = 0;
	int prefLen = 0;

	for (int i = 1; i < prevBufLength; ++i)
	{
		while (prefLen > 0 && _readBuffer[prefLen + prevBegin] != _readBuffer[i + prevBegin])
			prefLen = maxPrefixLengths[prefLen - 1];

		if (_readBuffer[prefLen + prevBegin] == _readBuffer[i + prevBegin])
			++prefLen;

		maxPrefixLengths[i] = prefLen;
	}

	prefLen = 0;
	maxPrefixLengths[prevBufLength] = prefLen;

	for (int i = prevBufLength + 1; i < size; ++i)
	{
		if (prefLen == prevBufLength)
			prefLen = maxPrefixLengths[prefLen - 1];

		while (prefLen > 0 &&
			_readBuffer[(prefLen < prevBufLength) ? prefLen + prevBegin : prefLen - prevBufLength - 1 + histBegin] !=
			_readBuffer[i - prevBufLength - 1 + histBegin])
			prefLen = maxPrefixLengths[prefLen - 1];

		if (_readBuffer[i - prevBufLength - 1 + histBegin] ==
			_readBuffer[(prefLen < prevBufLength) ? prefLen + prevBegin : prefLen - prevBufLength - 1 + histBegin])
			++prefLen;

		maxPrefixLengths[i] = prefLen;
	}
	return maxPrefixLengths;
}

void LZ77::findNode(int& offset, int& length, char& symbol, int histBegin, int prevBegin, int prevEnd)
{
	length = 0;
	offset = 0;
	symbol = _readBuffer[prevBegin];
	int prevBufLength = prevEnd - prevBegin + 1;
	int histBufLength = prevBegin - histBegin;
	int* prefixLengths = prefixFunction(histBegin, prevBegin, prevEnd);
	int pos = histBufLength + prevBufLength + 1;
	for (int i = prevBufLength + 1; i < 2 * prevBufLength + histBufLength + 1; ++i)
	{
		if (length < prefixLengths[i] && i - prefixLengths[i] + 1 < pos && i - prefixLengths[i] + 1 >= 0)
		{
			length = prefixLengths[i];
			offset = histBufLength - (i - prevBufLength - length);
			symbol = _readBuffer[prevBegin + length];
		}
	}
	delete[] prefixLengths;
}

void LZ77::writeNumAsBits(int num, int bitsNumber, int& writePos, int& bitsLeftInChar)
{
	unsigned int curBit = ((unsigned int)1) << (bitsNumber - 1);
	int bitsLeftInNum = bitsNumber;
	while (bitsLeftInNum > 0)
	{
		if (writePos < DEFAULT_BUFFER_SIZE)
		{
			//Побитовая запись числа непосредственно
			_writeBuffer[writePos] <<= 1;
			if ((num & curBit) > 0)
				_writeBuffer[writePos] |= 1;

			curBit >>= 1;
			--bitsLeftInChar;
			--bitsLeftInNum;
			if (bitsLeftInChar == 0)
			{
				++writePos;
				bitsLeftInChar = CHAR_BIT;
			}
		}
		else
		{
			//Вывод и очистка буфера вывода
			_output.write(_writeBuffer, DEFAULT_BUFFER_SIZE);
			writePos = 0;
		}
	}
}

void LZ77::encode(const std::string& inputFile, const std::string& outputFile)
{
	_input.open(inputFile, std::ios::in | std::ios::binary | std::ios::ate);
	_output.open(outputFile, std::ios::out | std::ios::binary);
	lz77();
	_input.close();
	_output.close();
}

void LZ77::lz77()
{
	long long bytesLeft = _input.tellg();
	_input.seekg(0, std::ios::beg);
	_output << bytesLeft << ':' << _bitsPerOffset << ':' << _bitsPerLength << ':' << _dictLength << 'S';
	int readBufSize = (bytesLeft < DEFAULT_BUFFER_SIZE) ? bytesLeft : DEFAULT_BUFFER_SIZE;
	_readBuffer = new char[readBufSize];
	_writeBuffer = new char[DEFAULT_BUFFER_SIZE];
	int histBegin = -_dictLength;
	int prevBegin = 0;
	int prevEnd = (bytesLeft < _buffLength) ? bytesLeft - 1 : _buffLength - 1;
	int writePos = 0;
	int bitsLeftInChar = CHAR_BIT;
	int length = 0;
	int offset = 0;
	char symbol;

	_input.read(_readBuffer, readBufSize);

	while (bytesLeft > 0)
	{
		if (prevBegin + _buffLength > DEFAULT_BUFFER_SIZE)
		{
			for (int i = 0; i < DEFAULT_BUFFER_SIZE - histBegin; ++i)
				_readBuffer[i] = _readBuffer[histBegin + i];
			_input.read(&_readBuffer[DEFAULT_BUFFER_SIZE - histBegin], (histBegin < bytesLeft) ? histBegin : bytesLeft);
			histBegin = 0;
			prevBegin = _dictLength;
			prevEnd = _dictLength + _buffLength - 1;
		}

		if (histBegin < 0)
			if (bytesLeft < _buffLength)
				findNode(offset, length, symbol, 0, prevBegin, prevBegin + bytesLeft - 1);
			else
				findNode(offset, length, symbol, 0, prevBegin, prevEnd);
		else if (bytesLeft < _buffLength)
			findNode(offset, length, symbol, histBegin, prevBegin, prevBegin + bytesLeft - 1);
		else
			findNode(offset, length, symbol, histBegin, prevBegin, prevEnd);

		writeNumAsBits(offset, _bitsPerOffset, writePos, bitsLeftInChar);
		writeNumAsBits(length, _bitsPerLength, writePos, bitsLeftInChar);
		writeNumAsBits(symbol, CHAR_BIT, writePos, bitsLeftInChar);

		histBegin += length + 1;
		prevBegin += length + 1;
		prevEnd += length + 1;
		bytesLeft -= length + 1;
	}

	if (writePos > 0)
	{
		_writeBuffer[writePos] <<= bitsLeftInChar;
		_output.write(_writeBuffer, writePos + 1);
	}

	delete[] _readBuffer;
	delete[] _writeBuffer;
}

int LZ77::readNumFromBits(int& curByte, int& curBit, int bitsInNum)
{
	int number = 0;
	while (bitsInNum > 0)
	{
		if (bitsInNum >= CHAR_BIT - curBit)
		{
			int bitsWritten = CHAR_BIT - curBit;
			for (curBit; curBit < CHAR_BIT; ++curBit)
			{
				number <<= 1;
				if (_readBuffer[curByte] & (1 << CHAR_BIT - curBit - 1))
					number |= 1;
			}
			curBit = 0;
			++curByte;
			bitsInNum -= bitsWritten;
		}
		else
		{
			int end = curBit + bitsInNum;
			for (curBit; curBit < end; ++curBit)
			{
				number <<= 1;
				if (_readBuffer[curByte] & (1 << CHAR_BIT - curBit - 1))
					number |= 1;
			}
			bitsInNum = 0;
		}
	}
	return number;
}

void LZ77::decodeBytes()
{
	int bytesLeft;
	int histBufSize;
	char trash;
	_input >> bytesLeft >> trash >> _bitsPerOffset >> trash >> _bitsPerLength >> trash >> histBufSize >> trash;
	long long start = _input.tellg();
	_input.seekg(0, std::ios::end);
	long long end = _input.tellg();
	_input.seekg(start, std::ios::beg);
	int bitsPerTriple = _bitsPerOffset + _bitsPerLength + CHAR_BIT;

	_readBuffer = new char[DEFAULT_BUFFER_SIZE];
	_writeBuffer = new char[DEFAULT_BUFFER_SIZE];
	int readPos = 0;
	int writePos = 0;
	int writeBegin = 0;
	int curBit = 0;
	int offset = 0;
	int length = 0;
	char symbol;

	_input.read(_readBuffer, DEFAULT_BUFFER_SIZE);

	while (bytesLeft > 0)
	{
		if (bitsPerTriple >= CHAR_BIT * (DEFAULT_BUFFER_SIZE - readPos) - curBit)
		{
			for (int i = 0; i < DEFAULT_BUFFER_SIZE - readPos; ++i)
				_readBuffer[i] = _readBuffer[i + readPos];
			_input.read(&_readBuffer[DEFAULT_BUFFER_SIZE - readPos], readPos);
			readPos = 0;
		}

		offset = readNumFromBits(readPos, curBit, _bitsPerOffset);
		length = readNumFromBits(readPos, curBit, _bitsPerLength);
		symbol = (char)readNumFromBits(readPos, curBit, CHAR_BIT);

		if (writePos + length + 1 > DEFAULT_BUFFER_SIZE)
		{
			_output.write(&_writeBuffer[writeBegin], writePos - writeBegin);
			for (int i = 0; i < histBufSize; ++i)
				_writeBuffer[i] = _writeBuffer[writePos - histBufSize + i];
			writeBegin = histBufSize;
			writePos = histBufSize;
		}

		for (int i = 0; i < length; ++i)
			_writeBuffer[writePos + i] = _writeBuffer[writePos + i - offset];

		writePos += length;
		bytesLeft -= length + 1;
		_writeBuffer[writePos] = symbol;
		++writePos;
	}

	if (writePos > 0)
		_output.write(&_writeBuffer[writeBegin], writePos - writeBegin + bytesLeft);

	delete[]_readBuffer;
	delete[]_writeBuffer;
}

void LZ77::decode(const std::string& inputFileName, const std::string& outputFileName)
{
	_input.open(inputFileName, std::ios::in | std::ios::binary);
	_output.open(outputFileName, std::ios::out | std::ios::binary);
	decodeBytes();
	_input.close();
	_output.close();
}

std::string LZ77::toString()
{
	return "lz77" + std::to_string(_buffKB + _dictKB);
}