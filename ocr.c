// non-portable code using Intel intrinsics and
// assuming 2-complement integers

#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <immintrin.h>

#define COLUMNS_PER_GLYPH 3
#define ROWS_PER_GLYPH 3
#define GLYPHS_PER_ROW 9
#define PATTERNS_PER_ROW (1 << (COLUMNS_PER_GLYPH))
#define ROW_LEN ((GLYPHS_PER_ROW)*(COLUMNS_PER_GLYPH))
#define INVALID_PATTERN 1024
#define INVALID_GLYPH 128

unsigned short patternToDigits[ROWS_PER_GLYPH][PATTERNS_PER_ROW];
unsigned short and[GLYPHS_PER_ROW];
unsigned char val[GLYPHS_PER_ROW];

bool isPow2(unsigned short val)
{
	return (val != 0) && ((val & (-val)) == val);
}

void init()
{
	for (int r = 0; r < ROWS_PER_GLYPH; ++r)
		for (int p = 0; p < PATTERNS_PER_ROW; ++p)
			patternToDigits[r][p] = INVALID_PATTERN;

	patternToDigits[0][0] = 18;		//  0b0000010010;
	patternToDigits[0][2] = 1005;	//  0b1111101101;

	patternToDigits[1][5] = 1;		//  0b0000000001;
	patternToDigits[1][1] = 130;	//  0b0010000010;
	patternToDigits[1][3] = 12;		//  0b0000001100;
	patternToDigits[1][7] = 784;	//  0b1100010000;
	patternToDigits[1][6] = 96;		//  0b0001100000;

	patternToDigits[2][1] = 146;	//  0b0010010010;
	patternToDigits[2][6] = 4;		//  0b0000000100;
	patternToDigits[2][3] = 552;	//  0b1000101000;
	patternToDigits[2][7] = 321;	//  0b0101000001;
}

void recognize(char** rows, char* out, int outSize)
{
	for (int g = 0; g < GLYPHS_PER_ROW; ++g)
	{
		val[g] = INVALID_GLYPH;
		and[g] = 65535;
	}

	for (int r = 0; r < ROWS_PER_GLYPH; ++r)
	{
		for (int g = 0; g < GLYPHS_PER_ROW; ++g)
		{
			unsigned char bin = 0;
			for (int b = 0; b < COLUMNS_PER_GLYPH; ++b)
			{
				bin <<= 1;
				bin |= rows[r][b + g * COLUMNS_PER_GLYPH] != ' ';
			}
			and[g] &= patternToDigits[r][bin];
		}
	}

	bool invalidDigits = false;
	for (int g = 0; g < GLYPHS_PER_ROW; ++g)
	{
		if (isPow2(and[g]))
			val[g] = (unsigned char)(_tzcnt_u32(and[g]));
		else
			invalidDigits = true;
	}

	bool invalidChecksum = false;
	long checksum = 0;
	for (int g = 0; g < GLYPHS_PER_ROW; ++g)
		checksum += val[g] * (GLYPHS_PER_ROW - g);
	invalidChecksum = checksum % 11 != 0;

	for (int g = 0; g < GLYPHS_PER_ROW; ++g)
		out[g] = val[g] != INVALID_GLYPH ? '0' + val[g] : '?';

	out[GLYPHS_PER_ROW] = 0;
	if (invalidDigits)
		strcat_s(out, outSize, " ILL");
	else if (invalidChecksum)
		strcat_s(out, outSize, " ERR");
}

#define OUT_SIZE  ((GLYPHS_PER_ROW) + 4 + 1)

void test1()
{
	char* row1 = "    _  _     _  _  _  _  _ ";
	char* row2 = "  | _| _||_||_ |_   ||_||_|";
	char* row3 = "  ||_  _|  | _||_|  ||_| _|";
	char* test[] = { row1, row2, row3 };
	char out[OUT_SIZE];
	recognize(test, out, OUT_SIZE);
	assert(strcmp("123456789", out) == 0);
}

void test2()
{
	char* row1 = " _  _  _  _  _  _  _  _    ";
	char* row2 = "| || || || || || || ||_   |";
	char* row3 = "|_||_||_||_||_||_||_| _|  |";
	char* test[] = { row1, row2, row3 };
	char out[OUT_SIZE];
	recognize(test, out, OUT_SIZE);
	assert(strcmp("000000051", out) == 0);
}

void test3()
{
	char* row1 = "                           ";
	char* row2 = "|_||_||_||_||_||_||_||_||_|";
	char* row3 = "  |  |  |  |  |  |  |  |  |";
	char* test[] = { row1, row2, row3 };
	char out[OUT_SIZE];
	recognize(test, out, OUT_SIZE);
	assert(strcmp("444444444 ERR", out) == 0);
}

void test4()
{
	char* row1 = "    _  _  _  _  _  _     _ ";
	char* row2 = "|_||_|| || ||_   |  |  | _ ";
	char* row3 = "  | _||_||_||_|  |  |  | _|";
	char* test[] = { row1, row2, row3 };
	char out[OUT_SIZE];
	recognize(test, out, OUT_SIZE);
	assert(strcmp("49006771? ILL", out) == 0);
}

int main()
{
	init();

	test1();
	test2();
	test3();
	test4();
}