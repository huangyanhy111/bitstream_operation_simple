#ifndef __bitstreaminput__h__
#define __bitstreaminput__h__

#include <iostream>
using namespace std;
#include <assert.h>
#include "typedef.h"
#include "big_small_endian.h"
#include <cmath>
class InputByteStream
{
public:
	
	InputByteStream(std::istream& istream)
		: m_NumFutureBytes(0)
		, m_FutureBytes(0)
		, m_Input(istream)
	{
		istream.exceptions(std::istream::eofbit | std::istream::badbit);
	}

	void reset()
	{
		m_NumFutureBytes = 0;
		m_FutureBytes = 0;
	}

	bool eofBeforeNBytes(uint32_t n)
	{
		assert(n <= 4);
		if (m_NumFutureBytes >= n)
			return false;

		n -= m_NumFutureBytes;
		try
		{
			for (uint32_t i = 0; i < n; i++)
			{
				m_FutureBytes = (m_FutureBytes << 8) | m_Input.get();
				m_NumFutureBytes++;
			}
		}
		catch (...)
		{
			return true;
		}
		return false;
	}

	uint32_t peekBytes(uint32_t n)
	{
		eofBeforeNBytes(n);
		return m_FutureBytes >> 8 * (m_NumFutureBytes - n);
	}

	uint8_t readByte()
	{
		if (!m_NumFutureBytes)
		{
			uint8_t byte = m_Input.get();
			return byte;
		}
		m_NumFutureBytes--;
		uint8_t wanted_byte = m_FutureBytes >> 8 * m_NumFutureBytes;
		m_FutureBytes &= ~(0xff << 8 * m_NumFutureBytes);
		return wanted_byte;
	}

	uint32_t readBytes(uint32_t n)
	{
		uint32_t val = 0;
		for (uint32_t i = 0; i < n; i++)
			val = (val << 8) | readByte();
		return val;
	}


public:
	uint32_t m_NumFutureBytes; /* number of valid bytes in m_FutureBytes */
	uint32_t m_FutureBytes; /* bytes that have been peeked */
	std::istream& m_Input; /* Input stream to read from */
};


class BitStream:public InputByteStream
{
public:
	BitStream(istream &myistream) :InputByteStream(myistream)
	{
		IN_bytealigned = true;
		IN_bitbuffer = 0;
		IN_numbitbuffer = 0;
	}
	uint8_t read_bit()
	{
		if (!IN_numbitbuffer)
		{
			IN_bitbuffer = InputByteStream::readByte();
			IN_numbitbuffer += 8;
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
	bool isbytealigned()
	{
		if (!IN_numbitbuffer)return true;
		return false;
	}

public:
	//InputByteStream bytestream;
	//uint8_t bitlocation;
	bool IN_bytealigned;
	uint8_t IN_bitbuffer;
	uint32_t IN_numbitbuffer;

};

class IOBitStream :public BitStream
{
public:
	IOBitStream(istream &myistream, uint8_t *outbuf, uint32_t *outbufsize):BitStream(myistream), 
	OUT_bytealigned(true), 
	OUT_bitbuffer(0),
	OUT_numbitbuffer(0)
	{
		outputbuf = outbuf;
		OUT_bufsize_IN_BYTES = outbufsize;
	}
	void write_bit(uint32_t bitvalue)
	{
		//uint32_t bitvalue2 = endiantransfer(bitvalue);
		assert(bitvalue == 0 || bitvalue == 1);
		OUT_bitbuffer = (OUT_bitbuffer << 1) | bitvalue;
		++OUT_numbitbuffer;
		if (OUT_numbitbuffer == 8)
		{
			*outputbuf = OUT_bitbuffer;
			outputbuf++;
			*OUT_bufsize_IN_BYTES = *OUT_bufsize_IN_BYTES + 1;
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
	uint8_t *outputbuf; //外部传进来的最终码流存储器
	uint32_t *OUT_bufsize_IN_BYTES;//记录BYTE数量
};

#endif