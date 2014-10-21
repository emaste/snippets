#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>

int
main(int argc, char *argv[])
{
	char buf[256];
	int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
	size_t size = sizeof(buf);
	int pid = -1, rv;

	if (argc > 1) {
		pid = atoi(argv[1]);
		mib[3] = pid;
	}

	rv = sysctl(mib, sizeof(mib) / sizeof(mib[0]), buf, &size, NULL, 0);
	if (rv == 0)
		printf("kern.proc.pathname.%d = %s\n", pid, buf);
	else
		printf("kern.proc.pathname.%d returned errno %d (%s)\n", pid, errno, strerror(errno));
}
