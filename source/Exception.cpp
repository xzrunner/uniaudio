#include "uniaudio/Exception.h"

#include <cstdarg>

#include <stdio.h>
#include <string.h>

namespace ua
{

Exception::Exception(const CU_STR& msg)
	: m_message(msg)
{
}

Exception::Exception(const char* fmt, ...)
{
	va_list args;
	int size_buffer = 256, size_out;
	char* buffer;
	while (true)
	{
		buffer = new char[size_buffer];
		memset(buffer, 0, size_buffer);

		va_start(args, fmt);
		size_out = vsnprintf(buffer, size_buffer, fmt, args);
		va_end(args);

		// see http://perfec.to/vsnprintf/pasprintf.c
		// if size_out ...
		//      == -1             --> output was truncated
		//      == size_buffer    --> output was truncated
		//      == size_buffer-1  --> ambiguous, /may/ have been truncated
		//       > size_buffer    --> output was truncated, and size_out
		//                            bytes would have been written
		if (size_out == size_buffer || size_out == -1 || size_out == size_buffer-1)
			size_buffer *= 2;
		else if (size_out > size_buffer)
			size_buffer = size_out + 2; // to avoid the ambiguous case
		else
			break;

		delete[] buffer;
	}
	m_message = CU_STR(buffer);
	delete[] buffer;
}

}