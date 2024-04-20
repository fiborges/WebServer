# Webserv

Webserv is a simple HTTP server implemented in C++98. It allows handling multiple client connections concurrently and serves static files, as well as supports CGI scripts. The server is built using an event-driven architecture with the `poll()` function to efficiently manage multiple clients without blocking.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Architecture](#architecture)
- [Event Loop](#event-loop)
- [HTTP Request Parsing](#http-request-parsing)
- [HTTP Response Handling](#http-response-handling)
- [CGI Support](#cgi-support)
- [Contributing](#contributing)
- [License](#license)

## Features

- Handles multiple client connections concurrently using an event loop
- Serves static files (HTML, CSS, JavaScript, images, etc.)
- Supports CGI scripts for dynamic content generation
- Configurable through a configuration file
- Implements HTTP/1.1 protocol
- Supports GET, POST, and DELETE methods
- Handles HTTP headers and request/response bodies
- Provides logging functionality for monitoring server activity

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/your-username/webserv.git
   ```

2. Navigate to the project directory:
   ```
   cd webserv
   ```

3. Compile the server:
   ```
   make
   ```

## Usage

1. Start the server with the default configuration:
   ```
   ./webserv
   ```

   Alternatively, you can specify a custom configuration file:
   ```
   ./webserv path/to/config.conf
   ```

2. Open a web browser and visit `http://localhost:8080` to access the server.

3. To stop the server, press `Ctrl+C` in the terminal.

## Configuration

The server can be configured using a configuration file. The default configuration file is `config.conf`, but you can specify a different file when starting the server.

The configuration file allows you to set various options, such as:
- Server port and host
- Root directory for serving static files
- CGI script directory and file extensions
- Error page paths
- Logging settings

Here's an example configuration file:

```
server {
    listen 8080;
    host localhost;
    root /var/www/html;
    index index.html;
    error_page 404 /404.html;
    cgi_dir /var/www/cgi-bin;
    cgi_ext .php;
    log_file /var/log/webserv.log;
}
```

## Architecture

The Webserv project follows an event-driven architecture using the `poll()` function to handle multiple client connections concurrently. The main components of the server are:

- **Server Socket**: Listens for incoming client connections and accepts them.
- **Event Loop**: Continuously monitors the server socket and client sockets for events (e.g., new connections, data available for reading, socket ready for writing).
- **Client Sockets**: Represent the individual client connections. Each client socket is added to the event loop for monitoring.
- **HTTP Request Parsing**: Parses the incoming HTTP requests from clients, extracting the method, headers, and body.
- **HTTP Response Handling**: Generates appropriate HTTP responses based on the client's request and sends them back to the client.
- **CGI Support**: Executes CGI scripts when requested and returns the generated output to the client.

## Event Loop

The event loop is the core component of the server that allows handling multiple client connections concurrently. It uses the `poll()` function to monitor the server socket and client sockets for events.

The event loop performs the following steps:

1. Create a `pollfd` array to store the file descriptors and events to monitor.
2. Add the server socket to the `pollfd` array with the `POLLIN` event to monitor for incoming connections.
3. Enter an infinite loop:
   - Call `poll()` to wait for events on the monitored file descriptors.
   - Iterate through the `pollfd` array to check for events on each file descriptor.
   - If the server socket has a readable event (`POLLIN`), accept the new client connection and add the client socket to the `pollfd` array.
   - If a client socket has a readable event (`POLLIN`), read the incoming data, parse the HTTP request, and generate an appropriate HTTP response.
   - If a client socket has a writable event (`POLLOUT`), send the HTTP response to the client.
   - If a client socket has been closed, remove it from the `pollfd` array.

## HTTP Request Parsing

When a client sends an HTTP request, the server needs to parse the request to extract the relevant information. The HTTP request parsing process involves the following steps:

1. Read the incoming data from the client socket into a buffer.
2. Parse the request line to extract the HTTP method, request target (URL), and HTTP version.
3. Parse the request headers to extract additional information, such as `Host`, `User-Agent`, `Content-Type`, etc.
4. If the request has a body (e.g., POST request), read and parse the request body.
5. Validate the parsed request and handle any parsing errors gracefully.

## HTTP Response Handling

After parsing the HTTP request, the server generates an appropriate HTTP response and sends it back to the client. The HTTP response handling process involves the following steps:

1. Determine the appropriate response based on the request method, URL, and headers.
2. Set the response status code (e.g., 200 OK, 404 Not Found, 500 Internal Server Error).
3. Set the response headers, such as `Content-Type`, `Content-Length`, `Connection`, etc.
4. If the response includes a body (e.g., HTML file, image), read the file and include it in the response body.
5. Send the response headers and body to the client socket.
6. If the response indicates a connection close, remove the client socket from the `pollfd` array and close the socket.

## CGI Support

Webserv supports the execution of CGI scripts to generate dynamic content. When a client requests a resource that is mapped to a CGI script, the server executes the script and returns the generated output to the client.

The CGI support process involves the following steps:

1. Check if the requested resource is mapped to a CGI script based on the configuration.
2. Set the necessary environment variables for the CGI script, such as `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, etc.
3. Execute the CGI script using `fork()` and `exec()` functions.
4. Read the output generated by the CGI script from its stdout.
5. Send the CGI script's output as the response body to the client.