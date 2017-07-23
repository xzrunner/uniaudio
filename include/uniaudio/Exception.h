#ifndef _UNIAUDIO_EXCEPTION_H_
#define _UNIAUDIO_EXCEPTION_H_

#include <exception>
#include <string>

namespace ua
{

class Exception : public std::exception
{
public:
	Exception(const std::string& msg);
	Exception(const char* fmt, ...);
	virtual ~Exception() throw() {}

	inline virtual const char* What() const throw() { 
		return m_message.c_str(); 
	}

private:
	std::string m_message;

}; // Exception 

}

#endif // _UNIAUDIO_EXCEPTION_H_