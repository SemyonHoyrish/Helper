#pragma once
#include <iostream>
#include <cctype>
#include <vector>

#ifdef COMMONLIB_EXPORTS
#define COMMONLIB_API __declspec(dllexport)
#else
#define COMMONLIB_API __declspec(dllimport)
#endif

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


static inline void tolowercase(std::string& s) {
	for (size_t i = 0; i < s.length(); i++) {
		s.at(i) = static_cast<char>(tolower(static_cast<unsigned char>(s.data()[i])));
	}
}



#ifdef _DEBUG

#define LOGGING_LEVEL_WARN
#define LOGGING_LEVEL_DEBUG
#define LOGGING_LEVEL_INFO


#ifndef LOGGING_STREAM
	#define LOGGING_STREAM std::cout
#endif

#endif


#ifdef LOGGING_LEVEL_DEBUG
#define DEBUG_LOG_LINE() LOGGING_STREAM << std::endl;
#define DEBUG_LOG_DIVIDER() LOGGING_STREAM << "----------------" << std::endl;
#define DEBUG_LOG_DIVIDERB() LOGGING_STREAM << "================" << std::endl;
#else
#define DEBUG_LOG_LINE()
#define DEBUG_LOG_DIVIDER()
#define DEBUG_LOG_DIVIDERB()
#endif


#ifdef LOGGING_LEVEL_INFO
#ifndef LOGGING_LEVEL_DEBUG
#define LOGGING_LEVEL_DEBUG
#endif

#ifndef LOGGING_LEVEL_WARN
#define LOGGING_LEVEL_WARN
#endif

#define DEBUG_LOG_INFO(msg) LOGGING_STREAM << "[INFO] at (" << __FILE__ << ":" << __LINE__ << ") of [" << __FUNCTION__ << "]: " << msg << std::endl;
#else
#define DEBUG_LOG_INFO(msg)
#endif

