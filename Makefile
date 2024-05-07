RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

NAME		=	webserver
INCLUDES	=	-I include
CC			=	g++
RM			=	rm -rf
CFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address -g
DIR_SRCS	=	srcs
DIR_BUILD	=	build
SRCS		=	$(DIR_SRCS)/main.cpp $(DIR_SRCS)/parser.cpp $(DIR_SRCS)/parser_utils.cpp $(DIR_SRCS)/parserConfig.cpp $(DIR_SRCS)/URIparser.cpp 
BUILD		=	$(subst $(DIR_SRCS), $(DIR_BUILD), $(SRCS:.cpp=.o))


$(NAME): $(BUILD)
	@echo "${GREEN}Executable $(NAME) created!${NC}"
	@echo " _           _   _                                  ";
	@echo " \ \        / / | |                                 ";
	@echo "  \ \  /\  / /__| |__  ___  ___ _ ____   _____ _ __ ";
	@echo "   \ \/  \/ / _ \ '_ \/ __|/ _ \ '__\ \ / / _ \ '__|";
	@echo "    \  /\  /  __/ |_| \__ \  __/ |   \ V /  __/ |   ";
	@echo "     \/  \/ \___|_.__/|___/\___|_|    \_/ \___|_|   ";
	@echo "                                            		   ";                                          
	@-$(CC) $(CFLAGS) $(BUILD) -o $(NAME)

$(DIR_BUILD)/%.o	:	$(DIR_SRCS)/%.cpp
	@mkdir -p $(DIR_BUILD)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "${GREEN}Compilation of $< OK!${NC}"

all: $(NAME)

clean:
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)
	@echo "${RED}Cleaned!!${NC}"

fclean:	clean
	@$(RM) $(NAME)
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)

re:		fclean all