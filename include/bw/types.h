/* types.h -- Quick and easy machine independent types

Copyright (C) 1997-2013, Brian Bray


*/


// Option
#define LITTLENDIAN



namespace bw {

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#ifdef LITTLEENDIAN

typedef short int16le;
typedef unsigned short uint16le;
typedef int int32le;
typedef unsigned int uint32le;
typedef long long int64le;
typedef unsigned long long uint64le;

struct int16be {
	unsigned short m_;
	operator short() {
		return (m_&0xff)<<8) | (m_>>8);
	}
	int16be(short s) {
		m_ = ((s&0xff)<<8) | ((s>>8)&0xff);
	}
};
struct uint16be {
	unsigned short m_;
	operator unsigned short() {
		return (m_&0xff)<<8) | (m_>>8);
	}
	uint16be(unsigned short s) {
		m_ = ((s&0xff)<<8) | (s>>8);
	}
};
struct int32be {
	uint16be m_h;
	uint16be m_l;
	operator int() {
		return int(m_h)<<16) | int(m_l);
	}
	int32be(int s) {
		m_l = (s&0xffff);
		m_h = (s>>16)&0xffff;
	}
};
struct uint32be {
	uint16be m_h;
	uint16be m_l;
	operator unsigned int() {
		return int(m_h)<<16) | int(m_l);
	}
	uint32be(unsigned int s) {
		m_l = (s&0xffff);
		m_h = s>>16;
	}
};
struct int64be {
	uint32be m_h;
	uint32be m_l;
	operator long long() {
		return unsigned long long(unsigned int(m_h))<<32) | unsigned int(m_l);
	}
	int64be(long long s) {
		m_l = (s&0xffffffff);
		m_h = (s>>32)&0xffffffff;
	}
};
struct uint64be {
	uint32be m_h;
	uint32be m_l;
	operator unsigned long long() {
		return unsigned long long(unsigned int(m_h))<<32) | unsigned int(m_l);
	}
	uint64be(unsigned long long s) {
		m_l = (s&0xffffffff);
		m_h = s>>32;
	}
};


#else

typedef short int16be;
typedef unsigned short uint16be;
typedef int int32be;
typedef unsigned int uint32be;
typedef long long int64be;
typedef unsigned long long uint64be;

struct int16le {
	unsigned short m_;
	operator short() {
		return (m_&0xff)<<8) | (m_>>8);
	}
	int16le(short s) {
		m_ = ((s&0xff)<<8) | ((s>>8)&0xff);
	}
};
struct uint16le {
	unsigned short m_;
	operator unsigned short() {
		return (m_&0xff)<<8) | (m_>>8);
	}
	uint16le(unsigned short s) {
		m_ = ((s&0xff)<<8) | (s>>8);
	}
};
struct int32le {
	uint16le m_l;
	uint16le m_h;
	operator int() {
		return int(m_h)<<16) | int(m_l);
	}
	int32le(int s) {
		m_l = (s&0xffff);
		m_h = (s>>16)&0xffff;
	}
};
struct uint32le {
	uint16le m_l;
	uint16le m_h;
	operator unsigned int() {
		return int(m_h)<<16) | int(m_l);
	}
	uint32le(unsigned int s) {
		m_l = (s&0xffff);
		m_h = s>>16;
	}
};
struct int64le {
	uint32le m_l;
	uint32le m_h;
	operator long long() {
		return unsigned long long(unsigned int(m_h))<<32) | unsigned int(m_l);
	}
	int64le(long long s) {
		m_l = (s&0xffffffff);
		m_h = (s>>32)&0xffffffff;
	}
};
struct uint64le {
	uint32le m_l;
	uint32le m_h;
	operator unsigned long long() {
		return unsigned long long(unsigned int(m_h))<<32) | unsigned int(m_l);
	}
	uint64le(unsigned long long s) {
		m_l = (s&0xffffffff);
		m_h = s>>32;
	}
};

#endif	// LITTLENDIAN

}   // namespace
