NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread
IFLAGS	= -I include

SRCDIR	= src
SRCS	= $(SRCDIR)/main.c \
		  $(SRCDIR)/parse.c \
		  $(SRCDIR)/init.c \
		  $(SRCDIR)/coder.c \
		  $(SRCDIR)/dongle.c \
		  $(SRCDIR)/scheduler.c \
		  $(SRCDIR)/monitor.c \
		  $(SRCDIR)/log.c \
		  $(SRCDIR)/utils.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re