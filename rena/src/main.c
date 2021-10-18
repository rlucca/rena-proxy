#include <rena.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    struct rena *all_modules = NULL;
    int ret_code = rena_setup(argc, argv, &all_modules);
    if (ret_code < 0)
    {
        return ret_code;
    }

    return rena_run(&all_modules);
}
