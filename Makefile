RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

NAME		=	webserver
INCLUDES	=	-I include
CC			=	g++
RM			=	rm -rf
CFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address
DIR_SRCS	=	srcs
DIR_BUILD	=	build
SRCS		=	$(DIR_SRCS)/main.cpp $(DIR_SRCS)/parser.cpp\
				$(DIR_SRCS)/parser_utils.cpp $(DIR_SRCS)/parserConfig.cpp\
				$(DIR_SRCS)/RequestParser.cpp $(DIR_SRCS)/erros.cpp\
				$(DIR_SRCS)/get.cpp $(DIR_SRCS)/CGI.cpp
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

download:
	@wget https://cdn.intra.42.fr/document/document/24735/tester > /dev/null 2>&1
	@wget https://cdn.intra.42.fr/document/document/24736/ubuntu_cgi_tester > /dev/null 2>&1
	@wget https://cdn.intra.42.fr/document/document/24737/cgi_tester > /dev/null 2>&1
	@wget https://cdn.intra.42.fr/document/document/24738/ubuntu_tester > /dev/null 2>&1
	@echo "\n[${WHITE}${GREEN}WebServer mandatory project testers$(RESET)${WHITE}] ${GREEN}--> $(SBLINK)Downloaded and Ready${RESET}\n"

large:
	@for i in $$(seq 100000 -1 1); do \
		echo "[$$i] Our group FFB is the Best!!!"; \
	done > largefile.txt
	@echo "[0] ==> LAST LINE 😁😁😁 <==" >> largefile.txt
	@echo "\n ${GREEN}largefile.txt${WHITE} ${}--> $(SBLINK)Created and Ready${RESET}\n"

clean:
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)
	@echo "${RED}Cleaned!!${NC}"
	@test -f tester && rm tester > /dev/null 2>&1 || true
	@test -f ubuntu_cgi_tester && rm ubuntu_cgi_tester > /dev/null 2>&1 || true
	@test -f cgi_tester && rm cgi_tester > /dev/null 2>&1 || true
	@test -f ubuntu_tester && rm ubuntu_tester > /dev/null 2>&1 || true
	@test -f largefile.txt && rm largefile.txt > /dev/null 2>&1 || true
	
fclean:	clean
	@$(RM) $(NAME)
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)

re:		fclean all