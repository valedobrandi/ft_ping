#include "argp42.h"
#include <stdio.h>

t_arg42 * 
find_option(t_arg42 *opts, const char *type)
{
    for (int i = 0; opts[i].type; i++) {
        if (strcmp(opts[i].type, type) == 0)
            return &opts[i];
    }
    return NULL;
}

int 
argp42_parse(
    t_arg42_args *main_args,
    t_arg42 *opts,
    int (*handler)(char *, char *, void *),
    void *user
)
{
    char *arg;
    t_arg42 *opt;

    for (int i = 1; i < main_args->argc; i++) 
    {
        arg = main_args->argv[i];

        if (arg[0] == '-')
        {
            opt = find_option(opts, &arg[1]);
            if (!opt)
                return 1;
            if (handler(opt->type, 0, user) != 0)
                return 1;
        }
        else
        {
            if (handler(0, arg, user) != 0)
                return 1;
        }
        
    }
    return 0;
};

size_t parse_number(const char *optarg, size_t max, int allow_zero)
{
    char *p;
    unsigned long n;

    n = strtoul(optarg, &p, 0);
    if (*p) {
        printf("invalid value (`%s' near `%s')", optarg, p);
        exit(1);
    }
    if (n == 0 && !allow_zero) {
        printf("option value too small: %s", optarg);
        exit(1);
    };
    if (max && n > max) {
        printf("option value too big: %s", optarg);
        exit(1);
    };
    return n;
}

void
print_help(t_arg42 *opts)
{
    printf("Usage: ./ft_ping [OPTIONS] host\n\nOptions:\n");
    for (int i = 0; opts[i].type; i++) {
        printf("-%s: %s\n", opts[i].type, opts[i].helper);
    }
}