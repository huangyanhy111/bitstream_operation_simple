#include "bitstreaminput.h"
#include "typedef.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
using namespace std;

void main()
{
	ifstream bitstreamFile("D:\\bitstream\\Kimono_1920x1080_24_3frame-part1.bin", ifstream::in | ifstream::binary);
	if (!bitstreamFile)
	{
		fprintf(stderr, "\nfailed to open bitstream file for reading\n");
		exit(EXIT_FAILURE);
	}

	//InputByteStream bytestream(bitstreamFile);
	//BitStream bitstream(bitstreamFile);
	uint8_t *buf = (uint8_t *)malloc(sizeof(uint8_t) * 100000);
	uint8_t *pbuf = buf;                  
	uint32_t bufsize = 0;
	uint32_t read_nbits = 0;
	uint8_t write_8bits = 0;
	uint32_t write_nbits = 0;
	IOBitStream bitstream(bitstreamFile, buf, &bufsize);

	//while (!bitstream.eofBeforeNBytes(1) && bufsize < 100000)
	{	
#if 0
//test class1
		uint32_t test = bitstream.read_bits(8);
		uint8_t test4 = test;
		*buf = test4;
		buf += 1;
		bufsize += 1;
//test class2
		while (bitstream.peekBytes(3) != 0x000001 && bitstream.peekBytes(4) != 0x00000001 && !bitstream.eofBeforeNBytes(1))
		{
			read_nbits = bitstream.read_bits(8);
			write_8bits = read_nbits;
			*buf = write_8bits;
			buf += 1;
			bufsize += 1;
		}
		if ((bitstream.peekBytes(3) == 0x000001 || bitstream.peekBytes(4) == 0x00000001) && (!bitstream.eofBeforeNBytes(1)))
		   {
			if (bitstream.peekBytes(4) == 0x00000001)
			{
				read_nbits = bitstream.read_bits(32);
			}
			else if (bitstream.peekBytes(3) == 0x000001)
			{
				read_nbits = bitstream.read_bits(24);

			}
		}
#else
//test class3
		for (int count = 0; count < 5; ++count)
		{
			//bitstream.write_bits(23, 16);
			bitstream.write_bits(255, 8);
			
		}


#endif
	}


	buf = pbuf;
	ofstream outFile("D:\\bitstream\\testout.bin", ofstream::out | ofstream::binary);
	if (!outFile)
	{
		fprintf(stderr, "\nfailed to open bitstream file for writing\n");
		exit(EXIT_FAILURE);
	}
	outFile.write((char*)pbuf, bufsize);


	free(buf);
	bitstreamFile.close();
	outFile.close();
}