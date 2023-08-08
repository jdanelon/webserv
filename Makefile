NAME = webserv

SRCS_PATH = ./srcs

OBJS_PATH = ./objs

SRCS = main.cpp \
		serverFunctions.cpp \
		utils.cpp \
		webserv.cpp \
		parser/Parser.cpp \
		parser/ParserHelper.cpp \
		parser/ParserException.cpp \
		parser/Server.cpp \
		parser/Location.cpp \
		connection/ClientConnection.cpp \
		http/HttpRequest.cpp

OBJS = $(addprefix $(OBJS_PATH)/, $(SRCS:.cpp=.o))

OBJ_DIRS = $(OBJS_PATH) $(OBJS_PATH)/parser $(OBJS_PATH)/connection $(OBJS_PATH)/http

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address

$(NAME): $(OBJ_DIRS) $(OBJS)
	$(CC) -g $(OBJS) $(FLAGS) -o $(NAME)

all: $(NAME)

$(OBJ_DIRS):
	@mkdir -p $@

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
