#include "bitstreaminput.h"
#include "typedef.h"
#include "nalrelated.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
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
	uint8_t *pbuf = buf;   //输出缓存               
	uint32_t bufsize = 0;  //输出数据大小
	uint32_t read_nbits = 0;
	uint8_t write_8bits = 0;
	uint32_t write_nbits = 0;
	IOBitStream bitstream(bitstreamFile, buf, &bufsize);

	while (!bitstream.eofBeforeNBytes(1) && bufsize < 100000)
	{	
#if 0
//test class1 byte读取
		uint32_t test = bitstream.read_bits(8);
		uint8_t test4 = test;
		*buf = test4;
		buf += 1;
		bufsize += 1;
//test class2 bit读取
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
//test class3 bit读写
		for (int count = 0; count < 5; ++count)
		{
			//bitstream.write_bits(23, 16);
			bitstream.write_bits(255, 8);
			
		}

		for (int count2 = 0; count2 < 3; ++count2)
		{
			bitstream.write_bits(1, 3);
			bitstream.write_bits(1, 5);
		}
#endif
		vector<uint8_t> nalUnit;
		GetNalUnit(bitstream, nalUnit);//去00000001 prefix
		convertPlayloadToRBSP(nalUnit, (nalUnit[0] & 64) == 0);//去0x03
		myNAL_rw nalUnit_rw(nalUnit);
		/*
		do some changes
		*/

		AddEmulationPrevention(nalUnit);
		AddStartCodePrefix(nalUnit);
		while (1)
		{
			uint32_t a = nalUnit_rw.read_bits(8);
			nalUnit_rw.write_bits(1, 4);
		}
		
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