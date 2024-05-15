# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test4.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/05/15 14:24:59 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import sys
import os

def save_file(folder_name, file_name, file_content):
		if not os.path.exists(folder_name):
			os.makedirs(folder_name)

		with open(os.path.join(folder_name, file_name), 'wb') as f:
			f.write(file_content)

if __name__ == '__main__':
		if len(sys.argv) != 4:
			print("Usage: python script.py folder_name file_name file_content")
			sys.exit(1)

		folder_name = sys.argv[1]
		file_name = sys.argv[2]
		file_content = sys.argv[3].encode('utf-8')

		save_file(folder_name, file_name, file_content)
		print('File saved successfully!')
