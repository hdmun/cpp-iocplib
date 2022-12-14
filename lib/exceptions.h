#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__
#include <exception>

class WinSockException
	: public std::exception {
public:
	WinSockException(const char* message, int code) noexcept
		: std::exception(message, code) {}
};

#endif // !__EXCEPTIONS_H__
