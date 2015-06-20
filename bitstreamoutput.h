#include <iostream>
using namespace std;
#include <assert.h>
#include "typedef.h"
#if 0
class OutputByteStream
{
public:
	
	OutputByteStream(std::ostream& ostream)
		: m_NumFutureBytes(0)
		, m_FutureBytes(0)
		, m_output(ostream)
	{
		ostream.exceptions(std::ostream::eofbit | std::ostream::badbit);
	}

	void reset()
	{
		m_NumFutureBytes = 0;
		m_FutureBytes = 0;
	}

	bool eofBeforeNBytes(UInt n)
	{
		assert(n <= 4);
		if (m_NumFutureBytes >= n)
			return false;

		n -= m_NumFutureBytes;
		try
		{
			for (UInt i = 0; i < n; i++)
			{
				m_FutureBytes = (m_FutureBytes << 8) | m_output.write();
				m_NumFutureBytes++;
			}
		}
		catch (...)
		{
			return true;
		}
		return false;
	}

	uint32_t peekBytes(UInt n)
	{
		eofBeforeNBytes(n);
		return m_FutureBytes >> 8 * (m_NumFutureBytes - n);
	}

	uint8_t readByte()
	{
		if (!m_NumFutureBytes)
		{
			uint8_t byte = m_output.get();
			return byte;
		}
		m_NumFutureBytes--;
		uint8_t wanted_byte = m_FutureBytes >> 8 * m_NumFutureBytes;
		m_FutureBytes &= ~(0xff << 8 * m_NumFutureBytes);
		return wanted_byte;
	}

	uint32_t readBytes(UInt n)
	{
		uint32_t val = 0;
		for (UInt i = 0; i < n; i++)
			val = (val << 8) | readByte();
		return val;
	}

#if RExt__DECODER_DEBUG_BIT_STATISTICS
	UInt GetNumBufferedBytes() const { return m_NumFutureBytes; }
#endif

private:
	UInt m_NumFutureBytes; /* number of valid bytes in m_FutureBytes */
	uint32_t m_FutureBytes; /* bytes that have been peeked */
	std::ostream& m_output; /* Input stream to read from */
};
#endif