# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test4.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/06/03 13:57:27 by fde-carv         ###   ########.fr        #
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
    if not file_name or file_name.endswith('/'):
        print("Invalid file name")
        sys.exit(1)

    save_file(folder_name, file_name)
    print('File saved successfully!')
