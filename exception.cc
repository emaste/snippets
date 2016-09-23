#include <stdio.h>

int
main(int argc, char *argv[])
{
	try {
		throw 5;
	} catch (int &e) {
		printf("caught int %d\n", e);
		return 0;
	} catch (...) {
		printf("caught other exception\n");
		return 1;
	}
	printf("did not catch exception\n");
	return 1;
}

