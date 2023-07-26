NAME = webserv

SRCS_PATH = ./srcs

OBJS_PATH = ./objs

SRCS = main.cpp \
		serverFunctions.cpp \
		utils.cpp \
		WebServ.cpp \
		parser/Parser.cpp \
		parser/ParserHelper.cpp \
		parser/ParserException.cpp \
		parser/Server.cpp \
		parser/Location.cpp

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
	@$(RM) $(OBJS_PATH)/parser/*.o
	@$(RM) $(OBJS_PATH)/request/*.o
	@$(RM) $(OBJS_PATH)/response/*.o

fclean: clean
	@$(RM) $(NAME)

re: fclean all
