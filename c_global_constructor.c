#include <stdio.h>

__attribute__((constructor)) void
ctor(void)
{
	puts(__func__);
}

int
main(int argc, char *argv[])
{
	puts(__func__);
}
