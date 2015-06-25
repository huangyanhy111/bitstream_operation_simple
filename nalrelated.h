#ifndef __nalrelated__h__
#define __nalrelated__h__
#include "bitstreaminput.h"
#include <vector>

//nal production
void GetNalUnit(IOBitStream &bs, vector<uint8_t> &nalUnit)
{
	//(1)prefix之前必然为0 到了尾部就直接读完不另取nal
	while ((bs.eofBeforeNBytes(24 / 8) || bs.peekBytes(24 / 8) != 0x000001)
		&& (bs.eofBeforeNBytes(32 / 8) || bs.peekBytes(32 / 8) != 0x00000001))
	{
		uint8_t leading_zero_8bits = bs.readByte();
		assert(leading_zero_8bits == 0);
	}
	if (bs.peekBytes(24 / 8) != 0x000001)
	{
		uint8_t zero_byte = bs.readByte();
		assert(zero_byte == 0);
	}
	//(2) NB, (1) guarantees that the next three bytes are 0x00 00 01 
	uint32_t start_code_prefix_one_3bytes = bs.readBytes(24 / 8);
	assert(start_code_prefix_one_3bytes == 0x000001);
	//(3)读入nal内容
	while (bs.eofBeforeNBytes(24 / 8) || bs.peekBytes(24 / 8) > 2)
	{
		nalUnit.push_back(bs.readByte());
	}
	// NB, (3) guarantees there are at least three bytes of trailing bits available or none 
	while ((bs.eofBeforeNBytes(24 / 8) || bs.peekBytes(24 / 8) != 0x000001)
		&& (bs.eofBeforeNBytes(32 / 8) || bs.peekBytes(32 / 8) != 0x00000001))
	{
		uint8_t trailing_zero_8bits = bs.readByte();
		assert(trailing_zero_8bits == 0);
	}
}

//nal classes for read and write operation
class myNAL_r:public vector<uint8_t>
{
public:
	myNAL_r(vector<uint8_t> &nalUnit):pos(0),IN_bitbuffer(0),IN_numbitbuffer(0){ myNALUnit = &nalUnit; }
	uint8_t read_bit()
	{
		if (!IN_numbitbuffer)
		{
			IN_bitbuffer = (*myNALUnit)[pos];
			IN_numbitbuffer += 8;
			pos++;
			assert(pos < (*myNALUnit).size());
		}
		uint8_t wanted_bit = IN_bitbuffer;
		wanted_bit = wanted_bit >> 7;
		IN_bitbuffer = IN_bitbuffer << 1;
		IN_numbitbuffer--;
		return wanted_bit;
	}
	uint32_t read_bits(int n)
	{
		assert(n <= 32);
		uint32_t wanted_bits = 0;
		for (int i = 0; i < n; ++i)
		{
			wanted_bits = (wanted_bits << 1) | read_bit();
		}
		return wanted_bits;
	}
public:
	vector<uint8_t> *myNALUnit;
	uint8_t IN_bitbuffer;
	uint8_t IN_numbitbuffer;
	int pos;
};

class myNAL_rw :public myNAL_r
{
public:
	myNAL_rw(vector<uint8_t> &nalUnit) :myNAL_r(nalUnit){}
	
public:
	void write_bit(uint32_t bitvalue)
	{
		//uint32_t bitvalue2 = endiantransfer(bitvalue);
		assert(bitvalue == 0 || bitvalue == 1);
		OUT_bitbuffer = (OUT_bitbuffer << 1) | bitvalue;
		++OUT_numbitbuffer;
		if (OUT_numbitbuffer == 8)
		{
			(*myNALUnit).push_back(OUT_bitbuffer);
			OUT_numbitbuffer = 0;
		}
	}
	void write_bits(uint32_t writevalue, uint8_t bitnum)
	{
		assert(writevalue < pow(2, bitnum));
		//uint32_t writevalue2 = endiantransfer(writevalue);//大数端小数端转换 小->大
		for (uint8_t i = 0; i < bitnum; i++)
		{
			write_bit((writevalue >> (bitnum - i - 1)) & 0x0001);
		}
	}
public:
	bool OUT_bytealigned;
	uint8_t OUT_bitbuffer; //一字节的先入先出队列
	uint32_t OUT_numbitbuffer;//0~7 0代表缓存中没有一个bit的数据
};

//nal operations inside nals
void convertPlayloadToRBSP(vector<uint8_t> &nalUnitBuf, bool isVclNalUnit)
{
	UInt zeroCount = 0;
	vector<uint8_t>::iterator it_read, it_write;

	UInt pos = 0;
	//bitstream->clearEmulationPreventionByteLocation();
	for (it_read = it_write = nalUnitBuf.begin(); it_read != nalUnitBuf.end(); it_read++, it_write++, pos++)
	{
		assert(zeroCount < 2 || *it_read >= 0x03);
		if (zeroCount == 2 && *it_read == 0x03)
		{
			//bitstream->pushEmulationPreventionByteLocation(pos);
			pos++;
			it_read++;
			zeroCount = 0;

			if (it_read == nalUnitBuf.end())
			{
				break;
			}
			assert(*it_read <= 0x03);
		}
		zeroCount = (*it_read == 0x00) ? zeroCount + 1 : 0;
		*it_write = *it_read;
	}
	assert(zeroCount == 0);

	if (isVclNalUnit)
	{
		// Remove cabac_zero_word from payload if present
		Int n = 0;

		while (it_write[-1] == 0x00)
		{
			it_write--;
			n++;
		}

		if (n > 0)
		{
			printf("\nDetected %d instances of cabac_zero_word", n / 2);
		}
	}

	nalUnitBuf.resize(it_write - nalUnitBuf.begin());
}
#endif