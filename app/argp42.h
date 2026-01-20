#ifndef ARGP42_H
#define ARGP42_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

#define MAX_INTERVAL 2147483647.0

typedef struct s_arg42
{
    char type;
    char *arg;
    const char *helper;
} t_arg42;

typedef struct s_opts {
    unsigned long count;
    unsigned long interval;
    int ttl;
    unsigned long timeout;
    unsigned long linger;
    int verbose;
    int help;
    int has_interval;

} t_opts;

typedef struct s_arg42_args
{
    int argc;
    char **argv;
} t_arg42_args;

typedef struct s_args
{
    int verbose;
    int help;
    char *host;
}   t_args;

t_arg42 * find_option(t_arg42 *opts, const char type);

int argp42_parse(
    t_arg42_args *main_args,
    t_arg42 *opts,
    int (*handler)(int, char *, void *),
    void *user
);

void print_help(t_arg42 *opts);
size_t parse_number(const char *arg, size_t max, int allow_zero);

#endif