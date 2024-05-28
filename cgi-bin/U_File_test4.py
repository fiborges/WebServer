# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test4.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/05/27 14:34:45 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import sys
import os

def save_file(folder_name, file_name):
		if not os.path.exists(folder_name):
			os.makedirs(folder_name)

		#file_size_bytes = sys.stdin.buffer.read(4);
		#file_size = int.from_bytes(file_size_bytes, byteorder='little')

		file_content = sys.stdin.buffer.read()

		with open(os.path.join(folder_name, file_name), 'wb') as f:
			f.write(file_content)

if __name__ == '__main__':
		if len(sys.argv) != 2:
			print("Usage: python script.py folder_name")
			sys.exit(1)

		folder_name = sys.argv[1]
		file_name = os.environ.get("PATH_INFO", "uploaded_file")
		#file_content = sys.argv[3].encode('utf-8')

		#save_file(folder_name, file_name, file_content)
		save_file(folder_name, file_name)
		print('File saved successfully!')
