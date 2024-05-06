# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/26 13:56:00 by brolivei          #+#    #+#              #
#    Updated: 2024/05/06 14:30:07 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC=c++
CCFLAGS = -Wall -Wextra -Werror -g -std=c++98

SERVER_TEST = SERVER_TEST

SRCS_SERVER_TEST = main.cpp CGI.cpp

OBJS_SERVER_TEST = $(SRCS_SERVER_TEST:%.cpp=%.o)

all: $(SERVER_TEST)

$(SERVER_TEST): $(OBJS_SERVER_TEST)
					$(CC) $(CCFLAGS) $(OBJS_SERVER_TEST) -o $(SERVER_TEST) -lstdc++

%.o: %.cpp
					$(CC) $(CCFLAGS) -c $< -o $@

clean:
					rm -rf $(OBJS_SERVER_TEST)

fclean: clean
					rm -rf $(SERVER_TEST)

re: fclean all
