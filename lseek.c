#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
lseek_assert(int fd, off_t offset, int whence, off_t expected)
{
	off_t r = lseek(fd, offset, whence);
	printf("lseek(fd, %lld, %d) returns %lld", (long long)offset,
	    whence, (long long)r);
	if (r == expected) {
		printf("\n");
		return;
	}
	printf(", expected %lld\n", (long long)expected);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int fd = open("/usr/lib/libc.a", O_RDONLY);
	lseek_assert(fd, 0, SEEK_SET, 0);
	lseek_assert(fd, 1, SEEK_SET, 1);
	lseek_assert(fd, 4095, SEEK_SET, 4095);
	lseek_assert(fd, 0, SEEK_SET, 0);
	lseek_assert(fd, 1, SEEK_CUR, 1);
	lseek_assert(fd, 1000, SEEK_CUR, 1001);
	printf("OK\n");
	return (0);
}

