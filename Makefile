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
				$(DIR_SRCS)/get.cpp $(DIR_SRCS)/CGI.cpp $(DIR_SRCS)/CR.cpp
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
	@chmod +x ubuntu_cgi_tester
	@chmod +x ubuntu_tester

large:
	@mkdir -p resources/FILES
	@for i in $$(seq 100000 -1 1); do \
		echo "[$$i] Our group FFB is the Best!!!"; \
	done > resources/FILES/largefile.txt
	@echo "[0] ==> LAST LINE 😁😁😁 <==" >> resources/FILES/largefile.txt
	@echo "\n ${GREEN}largefile.txt${WHITE} ${}--> $(SBLINK)Created and Ready${RESET}\n"
	@wget -O resources/FILES/imagem.jpg "https://wallpapercave.com/wp/wp4575206.jpg" > /dev/null 2>&1
	@echo " ${GREEN}imagem.jpg${WHITE} ${}--> $(SBLINK)Created and Ready${RESET}\n"
	@wget -O resources/FILES/video.mp4 "https://videocdn.cdnpk.net/joy/content/video/free/2018-09/large_preview/180824_TheEarth_39.mp4" > /dev/null 2>&1
	@echo " ${GREEN}video.mp4${WHITE} ${}--> $(SBLINK)Created and Ready${RESET}\n"
	@wget -O resources/FILES/webserver.pdf "https://cdn.intra.42.fr/pdf/pdf/125778/en.subject.pdf" > /dev/null 2>&1
	@echo " ${GREEN}webserver.pdf${WHITE} ${}--> $(SBLINK)Created and Ready${RESET}\n"

leaks:  fclean all
	@valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --track-fds=yes  --log-file=output.log ./webserver 8server2.conf

clean:
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)
	@echo "${RED}Cleaned!!${NC}"
	@test -f tester && rm tester > /dev/null 2>&1 || true
	@test -f ubuntu_cgi_tester && rm ubuntu_cgi_tester > /dev/null 2>&1 || true
	@test -f cgi_tester && rm cgi_tester > /dev/null 2>&1 || true
	@test -f ubuntu_tester && rm ubuntu_tester > /dev/null 2>&1 || true
	@test -f resources/FILES/largefile.txt && rm resources/FILES/largefile.txt > /dev/null 2>&1 || true
	@test -f resources/FILES/imagem.jpg && rm resources/FILES/imagem.jpg > /dev/null 2>&1 || true
	@test -f resources/FILES/video.mp4 && rm resources/FILES/video.mp4 > /dev/null 2>&1 || true
	@test -f resources/FILES/webserver.pdf && rm resources/FILES/video.mp4 > /dev/null 2>&1 || true
	@test -d DATA && rm -rf DATA > /dev/null 2>&1 || true
	@test -d resources/FILES && rm -rf resources/FILES > /dev/null 2>&1 || true
	@test -f output.log && rm output.log > /dev/null 2>&1 || true

fclean:	clean
	@$(RM) $(NAME)
	@$(RM) $(BUILD)
	@$(RM) -r $(DIR_BUILD)

re:		fclean all
