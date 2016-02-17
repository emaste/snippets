#include <link.h>
#include <stdio.h>

static int
callback(struct dl_phdr_info *dlpi, size_t sz, void *arg)
{
	(void)sz;
	(void)arg;
	printf("%s: %p\n", dlpi->dlpi_name, (void *)dlpi->dlpi_addr);
	return (0);
}

int
main(int argc, char *argv[])
{
	dl_iterate_phdr(callback, NULL);
	return (0);
}
