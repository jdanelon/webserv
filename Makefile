NAME = webserv

SRCS_PATH = ./srcs

OBJS_PATH = ./objs

SRCS = webserv.cpp \
		utils.cpp \
		Parser.cpp \
		ParserHelper.cpp \
		ParserException.cpp \
		Server.cpp \
		Location.cpp

OBJS = $(addprefix $(OBJS_PATH)/, $(SRCS:.cpp=.o))

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

$(NAME): $(OBJS)
	$(CC) -g $(OBJS) $(FLAGS) -o $(NAME)

all: $(NAME)

$(OBJS_PATH)/%.o: $(SRCS_PATH)/%.cpp
	$(CC) -g $(FLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJS_PATH)/*.o

fclean: clean
	@$(RM) $(NAME)

re: fclean all
