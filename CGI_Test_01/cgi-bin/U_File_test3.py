# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    U_File_test3.py                                    :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/07 13:25:38 by brolivei          #+#    #+#              #
#    Updated: 2024/05/07 14:46:03 by brolivei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import sys
import os
import re
from io import BytesIO
from urllib.parse import parse_qs
from urllib.request import urlopen

# Extract the input from the command line arguments
input_data = sys.argv[1]

# Extract the file content and filename
match = re.search(r'Content-Disposition: form-data; name="file"; filename="(.+)"\r\n\r\n(.+)--', input_data, re.DOTALL)
filename, file_content = match.groups()

# Create the U_DATA directory if it doesn't exist
directory = 'U_DATA'
if not os.path.exists(directory):
    os.makedirs(directory)

# Write the file content to the file
file_path = os.path.join(directory, filename)
with open(file_path, 'w') as file:
    file.write(file_content)

print(f'File {file_path} has been created.')
