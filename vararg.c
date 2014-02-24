#include <stdarg.h>

int
sum(int count, ...)
{
	va_list ap;
	int i, sum = 0;

	va_start(ap, count);
	for (i = 0; i < count; i++)
		sum += va_arg(ap, int);
	va_end(ap);

	return (sum);
}

int
main(int argc, char *argv[])
{
	int s = sum(3, 1, 2, 3);
	return (s);
}
