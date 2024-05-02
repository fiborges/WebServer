<<<<<<< HEAD
# Definição de Cores
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m # No Color

# Configurações gerais
=======
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

>>>>>>> origin/parser
NAME		=	webserver
INCLUDES	=	-I include
CC			=	clang++
RM			=	rm -rf
<<<<<<< HEAD
CFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address
DIR_SRCS	=	srcs
DIR_BUILD	=	build
SRCS		=	$(DIR_SRCS)/main.cpp $(DIR_SRCS)/parser.cpp
=======
CFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address -g
DIR_SRCS	=	srcs
DIR_BUILD	=	build
SRCS		=	$(DIR_SRCS)/main.cpp $(DIR_SRCS)/parser.cpp $(DIR_SRCS)/parser_utils.cpp $(DIR_SRCS)/parserConfig.cpp
>>>>>>> origin/parser
BUILD		=	$(subst $(DIR_SRCS), $(DIR_BUILD), $(SRCS:.cpp=.o))


$(NAME): $(BUILD)
<<<<<<< HEAD
=======
	@echo "${GREEN}Executable $(NAME) created!${NC}"
>>>>>>> origin/parser
	@echo " _           _   _                                  ";
	@echo " \ \        / / | |                                 ";
	@echo "  \ \  /\  / /__| |__  ___  ___ _ ____   _____ _ __ ";
	@echo "   \ \/  \/ / _ \ '_ \/ __|/ _ \ '__\ \ / / _ \ '__|";
	@echo "    \  /\  /  __/ |_| \__ \  __/ |   \ V /  __/ |   ";
	@echo "     \/  \/ \___|_.__/|___/\___|_|    \_/ \___|_|   ";
	@echo "                                            		   ";                                          
	@-$(CC) $(CFLAGS) $(BUILD) -o $(NAME)
<<<<<<< HEAD
	@echo "${GREEN}Executable $(NAME) created!${NC}"
=======
>>>>>>> origin/parser

$(DIR_BUILD)/%.o	:	$(DIR_SRCS)/%.cpp
	@mkdir -p $(DIR_BUILD)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "${GREEN}Compilation of $< OK!${NC}"

all: loading_effect $(NAME)

clean:
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)
	@echo "${RED}Cleaned!!${NC}"

fclean:	clean
	@$(RM) $(NAME)
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)

re:		fclean all