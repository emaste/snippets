#include <stdio.h>

#define PRINT_SIZE(x) printf("sizeof %s: %zu\n", #x, sizeof(x))

int
main(int argc, char *argv[])
{
	PRINT_SIZE(short);
	PRINT_SIZE(int);
	PRINT_SIZE(long);
	PRINT_SIZE(long long);
	PRINT_SIZE(float);
	PRINT_SIZE(double);
	PRINT_SIZE(long double);
	PRINT_SIZE(void *);
	PRINT_SIZE(size_t);
}
