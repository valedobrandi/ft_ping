#ifndef ARGP42_H
#define ARGP42_H

typedef struct s_arg42
{
    char type;
    const char *helper;
} t_arg42;

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
#endif