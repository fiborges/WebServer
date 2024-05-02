NAME		=	webserver

INCLUDES	=	-I include

CC			=	clang++

RM			=	rm -rf

CFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

DIR_SRCS	=	srcs
DIR_BUILD	=	build

SRCS		=	$(DIR_SRCS)/main.cpp $(DIR_SRCS)/parser.cpp $(DIR_SRCS)/parser_utils.cpp $(DIR_SRCS)/parserConfig.cpp

BUILD		=	$(subst $(DIR_SRCS), $(DIR_BUILD), $(SRCS:.cpp=.o))

$(NAME): $(BUILD)
		@-$(CC) $(CFLAGS) $(BUILD) -o $(NAME)
		@echo "Executable $(NAME) created!"

$(DIR_BUILD)/%.o	:	$(DIR_SRCS)/%.cpp
			@mkdir -p $(DIR_BUILD)
			@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
			@echo "Compilation OK!"

all: $(NAME)

clean:
		@$(RM) $(BUILD)
		@$(RM) -r $(DIR_BUILD)
		@echo "Cleaned!!"

fclean:	clean
		@$(RM) $(NAME)
		@$(RM) $(BUILD)
		@$(RM) -r $(DIR_BUILD)

re:		fclean all

PHONY:	all clean fclean re