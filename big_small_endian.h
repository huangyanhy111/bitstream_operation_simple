#ifndef __big_small_endian__
#define __big_small_endian__
#include "typedef.h"
uint32_t endiantransfer(uint32_t x)
{
	uint8_t a, b, c, d;
	a = (uint8_t)(x & 0xff);
	b = (uint8_t)((x & 0xff00) >> 8);//2. 字符类型转换的优先级高于移位，所以用括号把移位操作括起来~
	c = (uint8_t)((x & 0xff0000) >> 16);
	d = (uint8_t)((x & 0xff000000) >> 24);
	//printf("0x%x 0x%x 0x%x 0x%x\n",a,b,c,d);
	x = (a << 24) | (b << 16) | (c << 8) | d;
	//printf("after transfered x is 0x%x\n", x);
	return x;
}

#endif