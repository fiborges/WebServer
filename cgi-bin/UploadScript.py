# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    UploadScript.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/06/27 15:36:06 by brolivei         ###   ########.fr        #
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
		if len(sys.argv) != 1:
			print("Usage: python script.py folder_name")
			sys.exit(1)

		upload_dir = os.environ.get('UPLOAD_DIR', '')

		if not upload_dir:
			print("Status: 400 Bad Request")
			print("Content-Type: text/plain")
			print()
			print("Error: PATH_INFO not set")
			sys.exit(1)

		folder_name = upload_dir.lstrip('/')

		file_name = os.environ.get("FILE_NAME", "uploaded_file")
		#file_content = sys.argv[3].encode('utf-8')

		#save_file(folder_name, file_name, file_content)
		save_file(folder_name, file_name)

		print("Status: 200 OK")
		print("Content-Type: text/plain")
		print()
		print("File saved successfully!")

