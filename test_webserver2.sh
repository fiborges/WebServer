#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RESET='\033[0m'

x=5

function print_test_case {
    echo -e "\n"
    echo -e "========================================================================="
    echo -e "\n"
    echo -e "${BLUE}Test ${test_num}: ${1}${RESET}"
    echo -e "${CYAN}Expected Result: ${2}${RESET}"
    echo -e "\n"
    sleep $x
}

test_num=1

# Basic GET Requests
print_test_case "GET request to root directory" "Should return the root directory contents or index file."
curl -X GET http://localhost:8080/

((test_num++))

print_test_case "GET request to uploads directory" "Should return the contents of the uploads directory."
curl -X GET http://localhost:8080/uploads/

((test_num++))

# POST Requests
print_test_case "POST request to upload a file" "Should successfully upload a file."
curl -X POST -F "file=@./resources/website/aaa.txt" http://localhost:8080/uploads/
#curl -X POST -F "file=@/Users/filipa/Desktop/WebServer/resources/website/upload.html" http://localhost:8080/uploads/

((test_num++))

# DELETE Requests
print_test_case "DELETE request to remove a file" "Should delete the specified file."
curl -X DELETE http://localhost:8080/uploads/aaa.txt

((test_num++))

# CGI Script Execution
print_test_case "GET request to execute ExampleGET.py CGI script" "Should execute the Python CGI script and return its output."
curl -X GET http://localhost:8080/cgi-bin/ExampleGET.py

((test_num++))

print_test_case "POST request to execute ExampleGET.py CGI script" "Should execute the Python CGI script with POST data and return its output."
curl -X POST -d "name=John&age=30" http://localhost:8080/cgi-bin/ExampleGET.py

((test_num++))

# Error Pages
print_test_case "PUT request to root (Method Not Allowed)" "Should return 405 Method Not Allowed error page."
curl -X PUT http://localhost:8080/

((test_num++))

print_test_case "POST request to CGI script causing error" "Should return 500 Internal Server Error page."
curl -X POST -d "trigger=error" http://localhost:8080/cgi-bin/error_script.py

((test_num++))

# Autoindex
print_test_case "GET request to autoindexed directory" "Should return the directory listing."
curl -X GET http://localhost:8080/uploads/

((test_num++))

# Client Body Size Limit
print_test_case "POST request with large file exceeding client body size limit" "Should return an error indicating the file is too large."
dd if=/dev/zero of=largefile.txt bs=1024 count=200
curl -X POST -F "file=@largefile.txt" http://localhost:8080/uploads/

((test_num++))

# Custom Headers
print_test_case "GET request with custom header" "Should return the response with the custom header included."
curl -X GET -H "X-Custom-Header: value" http://localhost:8080/

((test_num++))

# Other HTTP Methods
print_test_case "HEAD request to root directory" "Should return headers only."
curl -I http://localhost:8080/

((test_num++))

print_test_case "OPTIONS request to root directory" "Should return allowed HTTP methods."
curl -X OPTIONS http://localhost:8080/

((test_num++))

# Testing Redirections
print_test_case "GET request to a redirected URL" "Should return the new location of the resource."
curl -L http://localhost:8080/redirect

((test_num++))

# 404 Not Found
print_test_case "GET request for a non-existent file" "Should return 404 Not Found error page."
curl -X GET http://localhost:8080/nonexistentfile.html

((test_num++))

# Multiple File Uploads
print_test_case "POST request to upload multiple files" "Should successfully upload multiple files."
curl -X POST -F "file1=@/Users/filipa/Desktop/WebServer/resources/website/upload1.html" -F "file2=@/Users/filipa/Desktop/WebServer/resources/website/upload2.html" http://localhost:8080/uploads/

((test_num++))

# CGI Script with Query Parameters
print_test_case "GET request to CGI script with query parameters" "Should execute the Python CGI script and return its output with query parameters."
curl -X GET "http://localhost:8080/cgi-bin/ExampleGET.py?name=Jane&age=25"

((test_num++))

# Directory Traversal Protection
print_test_case "GET request attempting directory traversal" "Should prevent directory traversal and return an error."
curl -X GET http://localhost:8080/../etc/passwd

((test_num++))

# Invalid File Upload
print_test_case "POST request to upload an invalid file type" "Should return an error indicating invalid file type."
echo "This is a test file with an invalid extension" > invalidfile.invalid
curl -X POST -F "file=@invalidfile.invalid" http://localhost:8080/uploads/

((test_num++))

# File Download
print_test_case "GET request to download a file" "Should successfully download the specified file."
curl -O http://localhost:8080/uploads/upload.html

((test_num++))

# Chunked Transfer Encoding
print_test_case "POST request with chunked transfer encoding" "Should handle chunked transfer encoding properly."
python3 chunked_request.py

((test_num++))

# Clean up large test file and invalid file
rm -f largefile.txt
rm -f invalidfile.invalid

echo -e "\n"
echo -e "${GREEN}All tests completed.${RESET}"
