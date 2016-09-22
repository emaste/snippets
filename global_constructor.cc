#include <stdio.h>

class constructor {
public:
	constructor() {
		puts(__func__);
	}
};

constructor c;

int
main(int argc, char *argv[])
{
	puts(__func__);
}
