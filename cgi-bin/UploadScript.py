# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    UploadScript.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/07/01 10:56:31 by brolivei         ###   ########.fr        #
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
			print("Error: UPLOAD_DIR not set")
			sys.exit(1)

		#folder_name = upload_dir.lstrip('/')

		file_name = os.environ.get("FILE_NAME", "uploaded_file")
		#file_content = sys.argv[3].encode('utf-8')

		#save_file(folder_name, file_name, file_content)
		save_file(upload_dir, file_name)

		print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")
		print(f"""
		<!DOCTYPE html>
		<html lang="en">
		<head>
			<meta charset="UTF-8">
			<meta name="viewport" content="width=device-width, initial-scale=1.0">
    		<title>File Upload</title>

			<style>

				/* Body of the page */
				body {{
					font-family: Arial, sans-serif;
					display: flex;
					justify-content: center;
					align-items: center;
					height: 70vh;
					margin: 0;
					background-color: #01012b;
				}}
			</style>

			<body>
				<div class="container">
					<h1>CGI RESPONSE</h1>
					<a href="/index.html"><b>Home</b></button></a>
				</div>
			</body>
		""")

