# Basics C++ - Example 6

# Table of Contents

* [Build Project](#build-project)
* [Install Project](#install-project)
* [Build and Install with Docker](#build-and-install-with-docker)
* [Usage](#usage)
  * [Client Example](#client-example)
  * [Server Example](#server-example)

# POSIX Threads

The `-pthread` option must be added to the `CMAKE_CXX_FLAGS`:
~~~
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread" )
~~~

# Build Project

Execute the `build.bat` script on Windows or `build.sh` script on Linux.

# Install Project

Execute the `install.sh` script on Linux. There is no support for Windows yet.

# Build and Install with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.

# Usage

Actions:
- `server`
- `client`

Parameters:
- `host=<IP-Address>`
- `port=<port number>`

The server waits for connections, reads the data from the stream and sends the data back to the client.
The server creates for each connection a worker thread.

The client connects to a server.
The user has to enter a message and send the data by pressing return.
Afterwards the client reads the input from server.

## Client Example

*Note: There is no implementation for Windows yet.*
~~~
project_cpp_binary client host=127.0.0.1 port=5044
~~~

~~~
Input Arguments:
        0: ./build/project_cpp_binary
        1: client
Host: 127.0.0.1
Port: 5044
Starting client...
Connect to server.
Client is connected to server.
>>> Enter 'quit', 'q' or 'exit' to stop program.
>>> Enter a message to send and press return.
Hello World!
> Data has been sent.
>>> Enter a message to send and press return.
What's up!
> Data has been sent.
Response: Hello World!
>>> Enter a message to send and press return.
~~~

## Server Example

~~~
project_cpp_binary server host=127.0.0.1 port=5044
~~~

~~~
Input Arguments:
        0: ./build/project_cpp_binary
        1: server
Host: 127.0.0.1
Port: 5044
Starting server...
Server thread ID: 140177203349248
Started server.
>>> Type any key and press return to stop.
Listening on port 5044
Worker thread ID: 140177194956544
Data: Hello World!
Data: What's up!
~~~


