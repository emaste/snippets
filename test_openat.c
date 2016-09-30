#include <sys/capsicum.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
touchat(int dirfd, const char *name)
{
	int fd;

	assert((fd = openat(dirfd, name, O_CREAT | O_TRUNC | O_WRONLY,
	    0777)) >= 0);
	assert(close(fd) == 0);
}

static void
assert_openat(int dirfd, const char *name)
{
	int fd;

	assert((fd = openat(dirfd, name, O_RDONLY)) >= 0);
	assert(close(fd) == 0);
}

static void
assert_openat_fail(int dirfd, const char *name, int expected_errno)
{
	int fd;

	assert((fd = openat(dirfd, name, O_RDONLY)) < 0);
	assert(errno == expected_errno);
}

int
main(int argc, char *argv[])
{
	int dirfd, subdirfd;
	int fd;

	system("rm -rf testdir");

	// Set up test tree.
	assert(mkdir("testdir", 0777) == 0);
	assert((dirfd = open("testdir", O_RDONLY)) >= 0);
	assert(mkdirat(dirfd, "d1", 0777) == 0);
	assert(mkdirat(dirfd, "d1/d2", 0777) == 0);
	assert(mkdirat(dirfd, "d1/d2/d3", 0777) == 0);
	touchat(dirfd, "d1/f1");
	touchat(dirfd, "d1/d2/f2");
	touchat(dirfd, "d1/d2/d3/f3");
	assert(symlinkat("d1/d2/d3", dirfd, "l3") == 0);

	// Basic passing cases.
	assert_openat(dirfd, "d1/d2/d3/f3");
	assert_openat(dirfd, "d1/d2/d3/../../f1");
	assert_openat(dirfd, "l3/f3");
	assert_openat(dirfd, "l3/../../f1");
	assert_openat(dirfd, "../testdir/d1/f1");

	// Basic failing cases.
	assert_openat_fail(dirfd, "does-not-exist", ENOENT);
	assert_openat_fail(dirfd, "l3/does-not-exist", ENOENT);

	// Capability mode cases.
	assert(cap_enter() == 0);
	// open() not permitted in capability mode
	assert((fd = open("testdir", O_RDONLY)) < 0);
	assert(errno == ECAPMODE);
	// AT_FDCWD not permitted in capability mode
	assert_openat_fail(AT_FDCWD, "d1/f1", ECAPMODE);
	// Relative path above dirfd not capable
	assert_openat_fail(dirfd, "..", ENOTCAPABLE);
	assert((subdirfd = openat(dirfd, "l3", O_RDONLY)) >= 0);
	assert_openat_fail(subdirfd, "../../f1", ENOTCAPABLE);

#if 1	// How Capsicum actually works.
	assert_openat_fail(dirfd, "d1/d2/d3/../../f1", ENOTCAPABLE);
	assert_openat_fail(dirfd, "l3/../../f1", ENOTCAPABLE);
#else	// How Capsicum could work. These paths are still under dirfd.
	assert_openat(dirfd, "d1/d2/d3/../../f1");
	assert_openat(dirfd, "l3/../../f1");
#endif

	assert_openat_fail(dirfd, "../testdir/d1/f1", ENOTCAPABLE);
}
