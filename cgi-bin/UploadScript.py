# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    UploadScript.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/15 14:17:14 by brolivei          #+#    #+#              #
#    Updated: 2024/07/14 10:30:29 by brolivei         ###   ########.fr        #
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

				/* Animation [WebServer] */
				@keyframes pulse {{
					0% {{
						transform: scale(1);
					}}
					50% {{
						transform: scale(1.1);
					}}
					100% {{
						transform: scale(1);
					}}
				}}

				/* header size and pulse [WebServer]*/
				h1 {{
					font-family: "Arial", monospace;
					font-size: 2.5em;
					font-weight: 900;
					/*animation: pulse 3s infinite;*/
				}}

				/* Container of the page */
				.container {{
					text-align: center;
					max-width: 850px;
					width: 100%;
					background-color: #94bbd0;
					padding: 20px;
					border-radius: 10px;
					box-shadow: 0px 0px 10px 0px rgba(0, 0, 0, 0.1);
				}}

				/* Buttons */
				.buttons {{
					font-size: 20px;
					/*background-color: #2b14bf;*/
					/*color: rgb(179, 18, 18);*/
					padding: 10px 20px;
					border: none;
					border-radius: 8px;
					cursor: pointer;
					margin-bottom: 20px;
					margin-left: 10px;
					margin-top: 5px;
				}}

				/* Credits 1 */
				.credits_1 {{
					text-align: center;
					padding: 15px;
					background-color: #5f94c9;
					margin-top: 20px; /* Add margin-top to create space between buttons and credits */
					border-radius: 10px; /* Add this line to round the corners */
				}}

				/* Credits 2 */
				.credits_2 {{
					max-width: 850px;
					margin: auto;
				}}

				/* hyperlink colors */
				a[href*="https://profile.intra.42.fr"] {{
					color: rgb(7, 7, 114);
				}}

				a[href*="https://profile.intra.42.fr"]:hover {{
					color: rgb(255, 255, 255);
				}}

				a[href*="https://github.com"] {{
					color: rgb(7, 7, 114);
				}}

				a[href*="https://github.com"]:hover {{
					color: rgb(255, 255, 255);
				}}

				.glow-on-hover {{
					border: none;
					outline: none;
					color: #fff;
					/*background: #57621f;*/
					cursor: pointer;
					position: relative;
					z-index: 0;
					border-radius: 10px;
				}}

				.glow-on-hover:before {{
					content: '';
					background: linear-gradient(45deg, #ff0000, #ff0000);
					position: absolute;
					top: -2px;
					left:-2px;
					background-size: 400%;
					z-index: -1;
					filter: blur(5px);
					width: calc(100% + 5px);
					height: calc(100% + 5px);
					animation: glowing 20s linear infinite;
					opacity: 0;
					transition: opacity .3s ease-in-out;
					border-radius: 10px;
				}}

				.glow-on-hover:active {{
					color: #000000
				}}

				.glow-on-hover:active:after {{
					background: #72bae0;
				}}

				.glow-on-hover:hover:before {{
					opacity: 1;
				}}

				.glow-on-hover:after {{
					z-index: -1;
					content: '';
					position: absolute;
					width: 100%;
					height: 100%;
					background: #090345;
					left: 0;
					top: 0;
					border-radius: 10px;
				}}

				.upload-button {{
					background-color: #8d0000;
					color: #ffffff;
					font-weight: bold
				}}
			</style>
		</head>
		<body>
			<div class="container">
				<h1>CGI RESPONSE\n</h1>
				<h2>The file was saved at {upload_dir} \n</h2>
				<button onclick="location.href='/'" type="button">Home</button>
			</div>
		</body>
		</html>
		""")
		# print(f"""
		# <!DOCTYPE html>
		# <html lang="en">
		# <head>
		# 	<meta charset="UTF-8">
		# 	<meta name="viewport" content="width=device-width, initial-scale=1.0">
		# 	<title>File Upload</title>

		# 	<style>
		# 		/* Body of the page */
		# 		body {{
		# 			font-family: Arial, sans-serif;
		# 			display: flex;
		# 			justify-content: center;
		# 			align-items: center;
		# 			height: 70vh;
		# 			margin: 0;
		# 			background-color: #01012b;
		# 		}}
		# 	</style>
		# </head>
		# <body>
		# 	<div class="container">
		# 		<h1>CGI RESPONSE\n</h1>
		# 		<h2>The file was saved at {upload_dir} \n</h2>
		# 		<button onclick="location.href='/'" type="button">Home</button>
		# 	</div>
		# </body>
		# </html>
		# """)

