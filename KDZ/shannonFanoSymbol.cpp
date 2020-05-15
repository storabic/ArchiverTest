#include "shannonFanoSymbol.h"

void Symbol::buildCodes(std::ofstream& file, std::vector<std::string>& codes, std::string code)
{
	if (_left)
	{
		file << 'N' << (unsigned char)_bit;
		_left->buildCodes(file, codes, code + '0');
		_right->buildCodes(file, codes, code + '1');
	}
	else
	{
		file << 'L' << _bit << _char;
		codes[_char] = code;
	}
}