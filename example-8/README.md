# Basics C++ - Example 8

# Table of Contents

* [Build Project](#build-project)
* [Install Project](#install-project)
* [Build and Install with Docker](#build-and-install-with-docker)
* [Usage](#usage)
  * [Client Example](#client-example)
  * [Server Example](#server-example)

# Build Project

Execute the `build.bat` script on Windows or `build.sh` script on Linux.

# Install Project

Execute the `build.sh` script on Linux. There is no support for Windows yet.

# Build and Install with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.

# Usage

Actions:
- `server`
- `client`

Parameters:
- `host=<IP-Address>`
- `port=<port number>`
- `key=<path to key file>`
- `cert=<path to cert file>`

The server will generate a self signed certifcate or you can pass your own certifcate.
You can generate your own certifcate e.g. with:
~~~
openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem
~~~

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
        0: build/project_cpp_binary
        1: client
Host: 127.0.0.1
Port: 5044
Key: 
Cert: 
Starting client...
Connect to server.
Server certificate:
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number: 1 (0x1)
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CA, O=GGolbik., CN=localhost
        Validity
            Not Before: Nov 13 20:12:01 2021 GMT
            Not After : Nov 13 20:12:01 2022 GMT
        Subject: C=CA, O=GGolbik., CN=localhost
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                RSA Public-Key: (2048 bit)
                Modulus:
                    00:97:14:6a:03:5f:f1:99:76:f9:2c:e2:98:0d:b9:
                    5b:ab:df:69:f0:76:25:90:87:29:8b:ac:3f:26:d7:
                    6d:95:79:45:7f:83:23:73:c8:1a:b2:a7:86:d6:65:
                    9c:22:ef:8e:2d:60:4c:cd:59:ce:3f:14:43:fe:78:
                    16:56:d7:c9:36:3f:89:f6:f6:5c:05:81:5f:ae:17:
                    05:41:68:74:6c:c6:4a:ab:55:e0:c3:a8:60:40:c4:
                    72:bc:4a:9d:15:b6:51:ca:5e:3b:29:47:75:b9:fa:
                    14:43:5b:22:04:ad:d2:ff:8e:fc:52:64:7f:ba:bb:
                    62:00:15:5b:7d:74:0b:0e:d3:95:94:3d:3d:4f:aa:
                    75:25:d7:75:97:b5:22:a2:85:b3:ef:46:40:fa:6c:
                    39:4b:cf:90:60:85:cd:c5:ca:bb:b3:1e:12:78:52:
                    d7:6c:d7:c0:d5:1c:e0:0b:48:68:2f:d2:2e:41:d2:
                    e9:9a:9c:9f:dd:2c:1f:37:ff:63:73:a3:37:63:2f:
                    a3:8f:76:61:ef:f8:9f:e8:24:7e:c8:ec:cc:d2:ef:
                    e1:09:c1:55:23:bc:19:79:ec:04:1a:d1:73:f7:5c:
                    9a:06:92:7b:07:3e:f9:be:5f:a3:f9:33:12:84:96:
                    fd:61:06:27:c9:0b:83:4a:ad:a0:11:57:83:44:8a:
                    00:d3
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
         83:ea:91:8b:c6:53:2b:31:a2:a2:16:51:2c:42:d3:71:f6:19:
         92:a0:c6:c2:5d:f1:d7:26:ee:34:72:1f:7f:e1:6c:0d:62:67:
         5f:42:50:2a:8b:03:b5:3a:6d:43:46:bd:7f:ce:d0:fc:6b:38:
         eb:dd:cf:87:37:b6:7e:12:dc:22:ad:53:9c:5d:94:39:84:c4:
         37:33:4f:a1:58:64:1a:9c:1c:fc:39:48:ee:af:3f:db:e0:16:
         ed:3b:dd:91:fb:1c:d7:64:c3:73:f6:c4:e0:f8:58:9e:9d:e8:
         f9:43:0e:e9:f0:ea:84:90:06:4a:3f:6e:1b:16:64:a9:8b:2a:
         6e:db:66:dc:de:86:02:a0:15:87:29:e2:9d:46:93:b8:9f:c3:
         44:2a:3a:d9:a4:7e:7c:32:74:0d:6b:a7:d4:6b:01:10:9f:e7:
         92:93:5c:1e:68:99:52:c3:0f:6a:ac:f3:62:38:54:54:76:e9:
         ad:ff:42:7b:9c:a8:cb:d5:de:6b:f6:45:e7:13:9e:bf:c3:04:
         a9:98:35:7e:46:51:2a:2a:ee:e4:d4:7a:d8:04:93:37:1f:76:
         7e:51:a9:65:80:b3:18:32:b8:a3:e1:66:c9:cc:4b:6d:c8:0f:
         4e:42:ba:1c:73:1f:8f:e8:71:25:40:9a:f6:9f:30:d2:76:7a:
         4d:2d:ed:f1
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
        0: build/project_cpp_binary
        1: server
Host: 127.0.0.1
Port: 5044
Key: 
Cert: 
Starting server...
Generate self signed certificate.
Using self signed certificate.
Server thread ID: 140242370656000
Started server.
>>> Type any key and press return to stop.
Listening on port 5044
Worker thread ID: 140242292045568
Worker thread ID: 140242292045568 Data: Hello World!
Worker thread ID: 140242292045568 Data: What's up!
~~~
