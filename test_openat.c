/*
 * Demonstration of open/openat return values and errors in capability mode,
 * and possible change with respect to paths containing "..".
 *
 * This is a basic single threaded demonstration of expected return values,
 * and does not address concurrency or consider attempts to subvert the
 * capability mode sandbox.
 */

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
	char cwd[MAXPATHLEN];
	char *abspath;
	int dirfd, subdirfd;
	int fd;

	system("rm -rf testdir");

	(void)getcwd(cwd, sizeof(cwd));
	asprintf(&abspath, "%s/testdir/d1/f1", cwd);

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
	assert(symlinkat("../testdir/d1", dirfd, "lup") == 0);
	assert(symlinkat("../..", dirfd, "d1/d2/d3/ld1") == 0);
	assert(symlinkat("../../f1", dirfd, "d1/d2/d3/lf1") == 0);

	// Basic passing cases.
	assert_openat(dirfd, "d1/d2/d3/f3");
	assert_openat(dirfd, "d1/d2/d3/../../f1");
	assert_openat(dirfd, "l3/f3");
	assert_openat(dirfd, "l3/../../f1");
	assert_openat(dirfd, "../testdir/d1/f1");
	assert_openat(dirfd, "lup/f1");
	assert_openat(dirfd, "l3/ld1");
	assert_openat(dirfd, "l3/lf1");
	assert((fd = open(abspath, O_RDONLY)) >= 0);
	assert(close(fd) == 0);
	assert_openat(dirfd, abspath);

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
	// Absolute paths not capable
	assert_openat_fail(dirfd, abspath, ENOTCAPABLE);

#if 1	// How Capsicum actually works.
	assert_openat_fail(dirfd, "d1/d2/d3/../../f1", ENOTCAPABLE);
	assert_openat_fail(dirfd, "l3/../../f1", ENOTCAPABLE);
	assert_openat_fail(dirfd, "l3/ld1", ENOTCAPABLE);
	assert_openat_fail(dirfd, "l3/lf1", ENOTCAPABLE);
#else	// How Capsicum could work. These paths are still under dirfd.
	assert_openat(dirfd, "d1/d2/d3/../../f1");
	assert_openat(dirfd, "l3/../../f1");
	assert_openat(dirfd, "l3/ld1");
	assert_openat(dirfd, "l3/lf1");
#endif

	assert_openat_fail(dirfd, "../testdir/d1/f1", ENOTCAPABLE);
	assert_openat_fail(dirfd, "lup/f1", ENOTCAPABLE);

	free(abspath);
}
