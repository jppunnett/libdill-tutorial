#ifndef _DBG_H
#define _DBG_H

#include <string.h>


#define get_errno() \
	(errno == 0 ? "none" : strerror(errno))

#define log_error(msg, ...) \
	do {\
		fprintf(stderr, "[ERROR] %s:%d (errno: %s): " msg "\n",\
			__FILE__, __LINE__, get_errno(), ##__VA_ARGS__);\
		fflush(stderr);\
	} while(0)

#define log_fatal(msg, ...) \
	do {\
		fprintf(stderr, "[FATAL] %s:%d (errno: %s): " msg "\n",\
			__FILE__, __LINE__, get_errno(), ##__VA_ARGS__);\
		fflush(stderr);\
		abort();\
	} while(0)


#endif
