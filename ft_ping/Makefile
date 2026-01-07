NAME = ft_ping
CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS =
SRCS = ft_ping.c
OBJS = $(SRCS:.c=.o)
DEPS = ft_ping.h
INCLUDES = -I.

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	
re: fclean all