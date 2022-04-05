#include"crc16.h"

unsigned short crc16_ccitt(unsigned char* buf, int wlen)
{
	unsigned short tmp;
	unsigned short crc = 0xFFFF;

	for (int i = 0; i < wlen; i++)
	{
		tmp = (unsigned short)((crc >> 8) ^ buf[i]);
		crc = (unsigned short)((crc << 8) ^ crc16tab[tmp]);
	}

	return crc;
}