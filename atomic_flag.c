#include <stdatomic.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    atomic_flag flag = ATOMIC_FLAG_INIT;
    printf("First test and set: %d\n", atomic_flag_test_and_set(&flag));
    printf("Second test and set: %d\n", atomic_flag_test_and_set(&flag));
    atomic_flag_clear(&flag);
    printf("Test and set after clear: %d\n", atomic_flag_test_and_set(&flag));
    return 0;
}
