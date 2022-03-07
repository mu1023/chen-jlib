#pragma once
#ifndef _CHEN_NETBUFFER_H_
#define _CHEN_NETBUFFER_H_

#include<cstring>
#include<vector>
#include<NetDef.h>
#include<CommonDefine.h>



class	NetBuffer
{
public:
	static const UInt32 CheapPrepend = 8;
	static const UInt32 InitialSize = 8192;
	static const UInt32	MaxSize = 1000000;

	explicit NetBuffer(UInt32 initialSize = InitialSize)
		: m_Data(CheapPrepend + initialSize),
		m_BeginIndex(CheapPrepend),
		m_EndIndex(CheapPrepend)
	{
	}

	void swap(NetBuffer& rhs)
	{
		m_Data.swap(rhs.m_Data);
		std::swap(m_BeginIndex, rhs.m_BeginIndex);
		std::swap(m_EndIndex, rhs.m_EndIndex);
	}
	UInt32 ReadableBytes() const
	{
		return m_EndIndex - m_BeginIndex;
	}

	UInt32 WritableBytes() const
	{
		return m_Data.size() - m_EndIndex;
	}
	UInt32 RependableBytes()const
	{
		return m_BeginIndex;
	}
	const Int8* Peek() const
	{
		return &m_Data[m_BeginIndex];
	}
	void Retrieve(UInt32 len)
	{
		if (len < ReadableBytes())
		{
			m_BeginIndex += len;
		}
		else
		{
			RetrieveAll();
		}
	}
	void RetrieveAll()
	{
		m_BeginIndex = CheapPrepend;
		m_EndIndex = CheapPrepend;
	}
	template<typename T>
	void RetrieveType()
	{
		Retrieve(sizeof T);
	}
	bool Align()
	{
		if (m_BeginIndex == CheapPrepend)
		{
			return true;
		}
		UInt32 readable = ReadableBytes();
		std::copy(m_Data.begin() + m_BeginIndex,
			m_Data.begin() + m_EndIndex,
			m_Data.begin() + CheapPrepend);

		m_BeginIndex = CheapPrepend;
		m_EndIndex = m_BeginIndex + readable;
		return true;
	}
	bool EnsureWritableBytes(UInt32 len)
	{
		if (WritableBytes() < len)
		{
			if (WritableBytes() + RependableBytes() < len + CheapPrepend)
			{
				m_Data.resize(m_EndIndex + len);
			}
			else
			{
				Align();
			}
		}
		return true;
	}
	Int8* BeginRead()
	{
		return &m_Data[m_BeginIndex];
	}
	Int8* BeginWrite()
	{
		return &m_Data[m_EndIndex];
	}
	void UnWrite(UInt32 len)
	{
		if (m_BeginIndex + len > m_EndIndex)
		{
			RetrieveAll();
		}
		else
		{
			m_EndIndex -= len;
		}
	}
	bool Append(const char* data, UInt32 len)
	{
		if (!EnsureWritableBytes(len))
		{
			return false;
		}
		std::copy(data, data + len, BeginWrite());
		m_EndIndex += len;
		return true;
	}
	void HasWritten(UInt32 len)
	{
		m_EndIndex += len;
	}

	bool Append(const void* /*restrict*/ data, UInt32 len)
	{
		return Append(static_cast<const char*>(data), len);
	}

	bool AppendInt32(Int32 x)
	{
		Int32 be32 = htonl(x);
		return Append(&be32, sizeof be32);
	}

	bool AppendInt16(Int16 x)
	{
		Int16 be16 = htons(x);
		return Append(&be16, sizeof be16);
	}
	bool AppendInt8(Int8 x)
	{
		return Append(&x, sizeof x);
	}



	Int32 PeekInt32() const
	{
		if (ReadableBytes() < sizeof Int32)
		{
			return 0;
		}
		Int32 be32 = 0;
		::memcpy(&be32, Peek(), sizeof be32);
		return ntohl(be32);
	}

	Int16 PeekInt16() const
	{
		if (ReadableBytes() < sizeof Int16)
		{
			return 0;
		}
		Int16 be16 = 0;
		::memcpy(&be16, Peek(), sizeof be16);
		return  ntohs(be16);
	}

	Int8 PeekInt8() const
	{
		if (ReadableBytes() < sizeof Int8)
		{
			return 0;
		}
		Int8 x = *Peek();
		return x;
	}

	Int32 ReadInt32()
	{
		Int32 result = PeekInt32();
		RetrieveType<Int32>();
		return result;
	}
	Int16 ReadInt16()
	{
		Int16 result = PeekInt16();
		RetrieveType<Int16>();
		return result;
	}
	Int8 ReadInt8()
	{
		Int8 result = PeekInt8();
		RetrieveType<Int8>();
		return result;
	}

	bool Repend(const void* data, size_t len)
	{
		if (m_BeginIndex < len)
		{
			return false;
		}
		m_BeginIndex -= len;
		const UInt8* d = static_cast<const UInt8*>(data);
		std::copy(d, d + len, &m_Data[m_BeginIndex]);
		return true;
	}
	bool RependInt32(Int32 x)
	{
		Int32 be32 = htonl(x);
		return  Repend(&be32, sizeof be32);
	}
	bool RependInt16(Int16 x)
	{
		Int16 be16 = htons(x);
		return  Repend(&be16, sizeof be16);
	}
	bool RependInt8(Int8 x)
	{
		return  Repend(&x, sizeof x);
	}
	void Shrink(size_t reserve)
	{
		Align();
		EnsureWritableBytes(ReadableBytes() + reserve);
		m_Data.shrink_to_fit();
	}
private:
	std::vector<Int8>				m_Data;
	UInt32							m_BeginIndex;
	UInt32							m_EndIndex;
};


#endif // 