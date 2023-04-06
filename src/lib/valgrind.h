/**
 * This is used only for checking the code source using valgrind
 */

#ifdef VALGRIND_MODE
	#include <valgrind/drd.h>
	//redefine this macro
	#define EXEC_ONLY_IN_VALGRIND_MODE(x) x
#else
	#define EXEC_ONLY_IN_VALGRIND_MODE(x)
#endif
