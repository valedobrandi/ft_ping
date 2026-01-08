NAME = ft_ping
CC = cc
CFLAGS = -Wall -Wextra -Werror
SRCS = ft_ping.c arg42.c
OBJS = $(SRCS:.c=.o)

INCLUDES = -I.

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	
re: fclean all