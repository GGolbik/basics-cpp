#ifdef __linux__
// linux code goes here
// Implementation of Berkeley sockets

#include <arpa/inet.h>  // ::htons(...) ; ::htonl(...), ::inet_addr(...)
#include <fcntl.h>      // ::fcntl(...)
#include <netinet/in.h>  // INADDR_ANY ; INADDR_NONE ; sockaddr_in ; IPPROTO_TCP ;
#include <sys/select.h>  // ::select(...)
#include <sys/socket.h>  // ::socket(...) ; AF_INET ; SOCK_STREAM
#include <sys/time.h>    // struct timeval
#include <unistd.h>      // ::close(int), ::read(int, void*, size_t)

#include <cerrno>    // errno
#include <cstring>   // ::strerror_r(...)
#include <iostream>  // std::cout(...) ; std::cerr(...)
#include <string>    // std::string
#include <vector>    // std::vector

#include "Server.h"
#include "Worker.h"

namespace ggolbik {
namespace cpp {
namespace tls {

Server::Server(unsigned short port) : Server(port, "") {}

Server::Server(unsigned short port, std::string interfaceAddress)
    : port{port},
      interfaceAddress{interfaceAddress},
      enabled{false},
      running{false},
      listenSocket{-1},
      keyFileName{"key.pem"},
      certFileName{"cert.pem"} {}

Server::~Server() { this->close(); }

/**
 * errno is thread safe. On Linux, the global errno variable is thread-specific.
 * POSIX requires that errno be threadsafe. If the value of errno should be
 * preserved across a library call, it must be saved.
 *
 * strerror_r is thread safe.
 *
 * errno is defined in <cerrno> and is an integer value.
 *
 * The method call
 *   char *::strerror_r(int errnum, char *buf, size_t buflen);
 * is defined in <cstring>
 */
static void printError() {
  size_t length = 1024;
  char buffer[length];
  std::cerr << "(" << errno << ")" << ::strerror_r(errno, buffer, length)
            << std::endl;
}

/**
 * Creates the IP socket address.
 *
 * ::inet_addr() interprets a string that represents numbers in the Internet
 * standard dotted decimal notation and returns a corresponding Internet
 * address. If ::inet_addr() is successful, it returns the address in network
 * byte order. Otherwise, it returns INADDR_NONE ( 0xFFFFFFFF ), and sets errno
 * to indicate the type of error.
 *
 * ::htons() converts the unsigned short integer hostshort from host byte order
 * to network byte order.
 *
 * ::htonl() converts the unsigned integer hostlong from host byte order to
 * network byte order. Required to convert the macros INADDR_ANY,
 * INADDR_LOOPBACK, INADDR_ANY.
 *
 * The method call
 *   unsigned long inet_addr(const char *cp);
 * as well as the method calls
 *   uint16_t htons(uint16_t hostshort);
 *   uint32_t htonl(uint32_t hostlong);
 * are defined in header <arpa/inet.h>
 *
 * The <sys/socket.h> header defines the following macros, with distinct integer
 * values:
 * - AF_INET
 * - AF_INET6
 *
 * The <netinet/in.h> header defines the following macros:
 * - INADDR_ANY (0.0.0.0)
 * - INADDR_LOOPBACK (127.0.0.1)
 * - INADDR_BROADCAST (255.255.255.255)
 * - INADDR_NONE
 * as well as the sockaddr_in type:
 *   struct sockaddr_in {
 *     sa_family_t    sin_family; // address family: AF_INET
 *     in_port_t      sin_port;   // port in network byte order
 *     struct in_addr sin_addr;   // internet address
 *   };
 *
 * Notice: errno is thread safe. On Linux, the global errno variable is
 * thread-specific. POSIX requires that errno be threadsafe.
 *
 * @param port the port number
 * @param interfaceAddress the IP address of the interface.
 * @param socketAddress the socket address object.
 * @return true if socket address has been created. false if the method failed
 * to create the socket address. On error you should check the errno value.
 */
static bool createSocketAddress(unsigned short port,
                                const std::string& interfaceAddress,
                                sockaddr_in& socketAddress) {
  // define the IPv4 address family.
  socketAddress.sin_family = AF_INET;

  // set port
  socketAddress.sin_port = ::htons(port);

  // check interface address. use any address if not specified.
  if (interfaceAddress.empty()) {
    // INADDR_ANY will bind the socket to all available interfaces.
    socketAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
  } else {
    // convert and set IP address
    socketAddress.sin_addr.s_addr = ::inet_addr(interfaceAddress.c_str());
  }

  // check if address is valid
  if (socketAddress.sin_addr.s_addr == ::htonl(INADDR_NONE)) {
    // failed to set address
    return false;
  }  // else address is defined

  return true;
}

/**
 * Creates a TCP socket for the given address.
 *
 * A socket is a generalized interprocess communication channel.
 * Like a pipe, a socket is represented as a file descriptor.
 * Unlike pipes sockets support communication between unrelated processes, and
 * even between processes running on different machines that communicate over a
 * network. Sockets are the primary means of communicating with other machines;
 * telnet, rlogin, ftp, talk and the other familiar network programs use
 * sockets.
 *
 * When you create a socket, you must specify the style of communication you
 * want to use and the type of protocol that should implement it. The
 * communication style of a socket defines the user-level semantics of sending
 * and receiving data on the socket.
 *
 * There are two types of Internet Sockets
 *   SOCK_STREAM : Stream Sockets - Stream sockets are reliable two-way
 * connected communication streams. SOCK_DGRAM : Datagram Sockets - Datagram
 * sockets are sometimes called "connectionless sockets" There are a few other
 * options like SOCK_SEQPACKET : Provides a sequenced, reliable, two-way
 * connection-based data transmission path for datagrams of fixed maximum
 * length; a consumer is required to read an entire packet with each input
 * system call. SOCK_RAW : Provides raw network protocol access. SOCK_RDM :
 * Provides a reliable datagram layer that does not guarantee ordering.
 *
 * You must also choose a namespace for naming the socket.
 * a namespace is sometimes called a protocol family.
 * - AF_INET
 *
 * Finally you must choose the protocol to carry out the communication.
 * The protocol determines what low-level mechanism is used to transmit and
 * receive data. Each protocol is valid for a particular namespace and
 * communication style.
 *
 * For each combination of style and namespace there is a default protocol,
 * which you can request by specifying 0 as the protocol number. So you would
 * see sometime
 *   ::socket(socketAddress.sin_family, SOCK_STREAM, 0);
 * to create a TCP socket.
 *
 * The method call
 *   int socket(int domain, int type, int protocol);
 * is defined in header <sys/socket.h>
 *
 * In <netinet/in.h> you find the sockaddr_in type:
 *   struct sockaddr_in {
 *     sa_family_t    sin_family; // address family: AF_INET
 *     in_port_t      sin_port;   // port in network byte order
 *     struct in_addr sin_addr;   // internet address
 *   };
 *
 * The <netinet/in.h> header defines the following macros:
 * - IPPROTO_TCP
 * The <sys/socket.h> header defines the following macros, with distinct integer
 * values:
 * - SOCK_STREAM
 * - AF_INET
 *
 * @param socketAddress The address with the specified address family /
 * namespace.
 * @return On success, a file descriptor for the new socket is returned. On
 * error, -1 is returned, and errno is set appropriately.
 */
static int createSocket(sockaddr_in& socketAddress) {
  // SOCK_STREAM is used to specify a stream socket.
  // IPPROTO_TCP is used to specify the TCP protocol.
  return ::socket(socketAddress.sin_family, SOCK_STREAM, IPPROTO_TCP);
}

/**
 * Closes a file descriptor, so that it no longer refers to any file and may be
 * reused.
 *
 * When you have finished using a socket, you can simply close its file
 * descriptor with ::close().
 *
 * If there is still data waiting to be transmitted over the connection,
 * normally close tries to complete this transmission. You can control this
 * behavior using the SO_LINGER socket option to specify a timeout period.
 *
 * The method call
 *   int close(int fd);
 * is defined in header <unistd.h>
 *
 * You can also shut down only reception or transmission on a connection by
 * calling shutdown, which is declared in <sys/socket.h>.
 *
 * @return true on success. On error, false is returned, and errno is set
 * appropriately.
 */
bool Server::closeSocket() {
  // lock mutex to set listen socket appropriately
  std::unique_lock<std::mutex> lock(this->mutexCloseSocket);

  // check if socket is valid
  if (this->listenSocket == -1) {
    // invalid socket
    // socket seems to be already closed.
    return true;
  }

  // close socket
  int closed = ::close(this->listenSocket) != -1;

  // set socket to an invalid value.
  this->listenSocket = -1;

  // return whether socket has been closed successfully.
  return closed;
}

static bool closeClientSocket(int socket) {
  if (socket == -1) {
    // invalid socket
    // socket already closed
    return true;
  }

  if (::close(socket) != 0) {
    return false;
  } else {
    return true;
  }
}

/**
 * Allows socket descriptor to be reuseable (Forecefully attaching socket to the
 * port).
 *
 * SOL_SOCKET to set options at the sockets API level
 *
 * SO_REUSEADDR: Specifies that the rules used in validating addresses supplied
 * to bind() should allow reuse of local addresses, if this is supported by the
 * protocol. This socket option tells the kernel that even if this port is busy
 * (in the TIME_WAIT state), go ahead and reuse it anyway. If it is busy, but
 * with another state, you will still get an address already in use error. It is
 * useful if your server has been shut down, and then restarted right away while
 * sockets are still active on its port. You should be aware that if any
 * unexpected data comes in, it may confuse your server, but while this is
 * possible, it is not likely.
 *
 * SO_REUSEPORT: SO_REUSEADDR socket option already allows multiple UDP sockets
 * to be bound to, and accept datagrams on, the same UDP port. However, by
 * contrast with SO_REUSEPORT, SO_REUSEADDR does not prevent port hijacking and
 * does not distribute datagrams evenly across the receiving threads.
 *
 * You can find a way more detailed explanation of SO_REUSEADDR and SO_REUSEPORT
 * at
 * -
 * https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ
 * -
 * https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
 * - https://lwn.net/Articles/542629/
 *
 * The method call
 *   int setsockopt(int sockfd, int level, int optname, const void *optval,
 * socklen_t optlen); is defined in header <sys/socket.h> as well as the macros
 * - SOL_SOCKET
 * - SO_REUSEADDR
 * - SO_REUSEPORT
 */
static bool setSocketOptions(int socket) {
  // For setsockopt(), the argument should be nonzero to enable a boolean
  // option, or zero if the option is to be disabled.
  int enable = 1;
  // On success, zero is returned. On error, -1 is returned, and errno is set
  // appropriately.
  if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable,
                   sizeof(int)) != 0) {
    return false;
  }
  return true;
}

/**
 * Sets the socket / file descriptor in non-blocking mode.
 *
 * By default, TCP sockets are in "blocking" mode. For example, when you call
 * ::read() to read from a stream, control isn't returned to your program until
 * at least one byte of data is read from the remote site. This process of
 * waiting for data to appear is referred to as "blocking". The same is true for
 * the write() API, the connect() API, etc. When you run them, the connection
 * "blocks" until the operation is complete.
 *
 * Its possible to set a descriptor so that it is placed in "non-blocking" mode.
 * When placed in non-blocking mode, you never wait for an operation to
 * complete. This is an invaluable tool if you need to switch between many
 * different connected sockets, and want to ensure that none of them cause the
 * program to "lock up."
 *
 * If you call ::read() in non-blocking mode, it will return any data that the
 * system has in it's read buffer for that socket. But, it won't wait for that
 * data. If the read buffer is empty, the system will return from ::read()
 * immediately saying "Operation Would Block!". To be more precise ::read()
 * returns -1 to indicate that an error occurred. And errno is set to EAGAIN or
 * EWOULDBLOCK. errno as well as the macros EAGAIN and EWOULDBLOCK are defined
 * in header <cerrno>
 *
 * The "non-blocking" mode is set by changing one of the socket's "flags". The
 * flags are a series of bits, each one representing a different capability of
 * the socket. So, to turn on non-blocking mode requires three steps:
 * 1. Call the ::fcntl() API to retrieve the socket descriptor's current flag
 * settings into a local variable.
 * 2. In our local variable, set the O_NONBLOCK (non-blocking) flag on. (being
 * careful, of course, not to tamper with the other flags)
 * 3. Call the ::fcntl() API to set the flags for the descriptor to the value in
 * our local variable.
 *
 * The method call
 *   int ::fcntl(int fd, int cmd, ...arg);
 * and the macro O_NONBLOCK are defined in header <fcntl.h>
 * On error, ::fctnl() returns -1 and errno is set appropriately.
 *
 * Notice: errno is thread safe. On Linux, the global errno variable is
 * thread-specific. POSIX requires that errno be threadsafe.
 *
 * @param socket the file descriptor to set in non-blocking mode.
 * @return true if mode could be set. false if the method failed to set the
 * mode. On error you should check the errno value.
 */
static bool setSocketModeNonBlocking(int socket) {
  // get the current flags
  int flags = ::fcntl(socket, F_GETFL, 0);
  if (flags == -1) {
    // failed to get flags
    return false;
  }  // else get flags was successful.

  // set flag for non-blocking mode
  flags |= O_NONBLOCK;
  if (::fcntl(socket, F_SETFL, flags) == -1) {
    // failed to set flags
    return false;
  }  // else socket / file descriptor has been set in non-blocking mode.

  return true;
}

/**
 * Enables connection requests on the socket.
 *
 * The number of pending connection requests on a server socket is finite.
 * If connection requests arrive from clients faster than the server can act
 * upon them, the queue can fill up and additional requests are refused with an
 * ECONNREFUSED error. You can specify the maximum length of this queue as an
 * argument to the listen function. The backlog argument defines the maximum
 * length to which the queue of pending connections for sockfd may grow. If a
 * connection request arrives when the queue is full, the client may receive an
 * error with an indication of ECONNREFUSED or, if the underlying protocol
 * supports retransmission, the request may be ignored so that a later reattempt
 * at connection succeeds.
 *
 * The listen function is not allowed for sockets using connectionless
 * communication styles.
 *
 * The method call
 *   int listen(int sockfd, int backlog);
 * is defined in header <sys/socket.h>
 */
static bool listenOnSocket(int socket) {
  // After the socket is bound to an IP address and port on the system, the
  // server must then listen on that IP address and port for incoming connection
  // requests.
  unsigned int backlog = 32;  // max incoming connections
  if (::listen(socket, backlog) != 0) {
    return false;
  }
  return true;
}

/**
 * A socket newly created with the socket function has no address.
 * Other processes can find it for communication only if you give it an address.
 * We call this binding the address to the socket, and the way to do it is with
 * the bind function.
 */
static bool bindSocket(int socket, sockaddr_in& address) {
  // Setup the TCP listening socket
  // bind the socket
  if (::bind(socket, (sockaddr*)&address, sizeof(address)) != 0) {
    return false;
  }
  return true;
}

/**
 * Once connection requests are enabled on a server socket, the select function
 * reports when the socket has a connection ready to be accepted.
 */
static int select(int socket) {
  // requires includes
  // #include <sys/select.h> for select()
  // #include <sys/time.h> for struct timeval
  // See also https://linux.die.net/man/2/select

  // select() uses a timeout that is a struct timeval (with seconds and
  // microseconds) The timeout argument specifies the minimum interval that
  // select() should block waiting for a file descriptor to become ready.
  // timeout must be set each time select is called because select updates the
  // timeout argument to indicate how much time was left. if timeout==NULL then
  // wait forever if timeout == fixed_amount_time then wait until specified time
  // if timeout == 0 return immediately.
  // A struct timeval represents a time as a number of seconds (tv_sec) plus a
  // number of microseconds (tv_usec) between 0 and 999,999. Thus, to represent
  // 10 milliseconds, you would use 10,000 microseconds.
  timeval timeout = {};
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;  // 100000 := 100ms

  // The fd_set data type represents file descriptor sets for the select
  // function. It is actually a bit array. The sets are modified by select() in
  // place to indicate which file descriptors actually changed status. Three
  // independent sets of file descriptors are watched. Those listed in readfds
  // will be watched to see if characters become available for reading (more
  // precisely, to see if a read will not block; in particular, a file
  // descriptor is also ready on end-of-file)
  fd_set readfds;
  // those in writefds will be watched to see if a write will not block
  /*fd_set writefds;*/
  // those in exceptfds will be watched for exceptions
  /*fd_set exceptfds;*/

  // The number nfds is the highest-numbered file descriptor in any of the three
  // sets, plus 1. Here we use just 1 socket. If you don't know the highest
  // number of sockets beforehand use FD_SETSIZE which is macro defined by
  // system that specifies the max number of file descriptors of an fd_set
  int nfds = FD_SETSIZE;

  // Initialize the file descriptor set.
  // FD_ZERO initializes the file descriptor set set to be the empty set.
  FD_ZERO(&readfds);
  // FD_SET adds file descriptor to the file descriptor set.
  FD_SET(socket, &readfds);
  // FD_SET(0, &readfds) would listen on stdin

  // On success, select() returns the number of file descriptors contained in
  // the three returned descriptor sets (that is, the total number of bits that
  // are set in readfds, writefds, exceptfds) which may be zero if the timeout
  // expires before anything interesting happens. On error, -1 is returned, and
  // errno is set appropriately; the sets and timeout become undefined, so do
  // not rely on their contents after an error.
  int result = ::select(nfds, &readfds, NULL /*&writefds*/, NULL /*&exceptfds*/,
                        &timeout);

  if (result == 0) {
    // timeout occurred
    return result;
  } else if (result < 0) {
    // error occurred
    return result;
  }  // else select returned number of file descriptors

  // FD_ISSET returns a nonzero value (true) if filedes/socket is a member of
  // the file descriptor set, and zero (false) otherwise.
  if (!FD_ISSET(socket, &readfds)) {
    // The socket is not a member of the readfds set
    return -1;
  }  // else socket is ready for reading data

  // you can now call accept() on the socket.
  return result;
}

/**
 * A poll implementation.
 *
 * #include <poll.h>
 * int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 * struct pollfd {
 *   int fd;           // file descriptor
 *   short events;     // requested events
 *   short revents;    // returned events
 * };
 *
 * The field revents is an output parameter
 */
/*
static int poll(int socket)
{
  pollfd fds[1];
  pollfd fd = {};
  fd.fd = socket;
  // Set POLLIN to return if there is data to read.
  fd.events = POLLIN;
  fds[0] = fd;

  nfds_t nfds = FD_SETSIZE;

  // milliseconds
  int timeout = 100;

  int result = ::poll(fds, nfds, timeout);

  if (result == 0)
  {
    // timeout occurred
    return result;
  }
  else if (result < 0)
  {
    // error occurred
    return result;
  } // else select returned number of file descriptors

  // you can now call accept() on the socket.
  return result;
}
*/

/**
 * Accept each incoming connection.
 * The accept() function shall extract the first connection on the queue of
 * pending connections, create a new socket with the same socket type protocol
 * and address family as the specified socket, and allocate a new file
 * descriptor for that socket.
 *
 * A socket that has been established as a server can accept connection requests
 * from multiple clients. The server’s original socket does not become part of
 * the connection. Instead, accept makes a new socket which participates in the
 * connection. accept returns the descriptor for this socket. The server’s
 * original socket remains available for listening for further connection
 * requests.
 *
 * The argument addr is a pointer to a sockaddr structure.
 * This structure is filled in with the address of the peer socket, as known to
 * the communications layer.
 *
 * The method call
 *   int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
 * is defined in <sys/socket.h>
 *
 * @return On success, a nonnegative integer that is a file descriptor for the
 * accepted socket. On error, -1 is returned, errno is set appropriately
 */
static int accept(int socket, sockaddr_in& peerAddress) {
  int addrlen = sizeof(peerAddress);

  // If address is not a null pointer, the address of the peer for the accepted
  // connection shall be stored in the sockaddr structure pointed to by address,
  // and the length of this address shall be stored in the object pointed to by
  // address_len.
  return ::accept(socket, (sockaddr*)&peerAddress, (socklen_t*)&addrlen);
}

bool Server::open(const std::string& password) {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  // check if server is already listening
  if (this->enabled) {
    std::cerr << "Server is already listening." << std::endl;
    return false;
  }

  // create TLS context
  this->tlsContextPtr =
      OpenSslWrapper::TlsContextPtr(OpenSslWrapper::createTlsContextServer());
  if (!tlsContextPtr) {
    std::cerr << "Failed to create TLS context." << std::endl;
    return false;
  }
  // configure TLS context
  if (!OpenSslWrapper::configureTlsContext(
          this->tlsContextPtr.get(), this->keyFileName, this->certFileName)) {
    std::cerr << "Failed to configure TLS context." << std::endl;
    return false;
  }

  // create a socket address
  sockaddr_in address;
  if (!createSocketAddress(this->port, this->interfaceAddress, address)) {
    std::cerr << "Failed to create socket address." << std::endl;
    printError();
    return false;
  }

  // Create a SOCKET for the server to listen for client
  this->listenSocket = createSocket(address);
  // Check for errors to ensure that the socket is a valid socket.
  if (this->listenSocket == -1) {
    std::cerr << "Create socket failed." << std::endl;
    printError();
    return false;
  }

  // Allow socket descriptor to be reuseable (Forecefully attaching socket to
  // the port)
  if (!setSocketOptions(this->listenSocket)) {
    std::cerr << "Set socket options failed." << std::endl;
    printError();
    if (!this->closeSocket()) {
      printError();
    }
    return false;
  }

  // set socket to be nonblocking.
  if (!setSocketModeNonBlocking(this->listenSocket)) {
    std::cerr << "Set socket to be non-blocking failed." << std::endl;
    printError();
    if (!this->closeSocket()) {
      printError();
    }
    return false;
  }

  // bind the socket
  if (!bindSocket(this->listenSocket, address)) {
    std::cerr << "Bind failed." << std::endl;
    printError();
    if (!this->closeSocket()) {
      printError();
    }
    return false;
  }

  // start listening
  if (!listenOnSocket(this->listenSocket)) {
    std::cerr << "Listen failed." << std::endl;
    printError();
    if (!this->closeSocket()) {
      printError();
    }
    return false;
  }

  // Update status
  this->enabled = true;
  this->running = true;

  // start accept thread
  this->serverThread = std::thread(&Server::run, this);
  std::cout << "Server thread ID: " << this->serverThread.get_id() << std::endl;

  return true;
}

bool Server::isOpen() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  // return whether server has been stopped or is still running
  return this->enabled || this->running;
}

void Server::close() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  if (this->enabled) {
    // signal stop
    this->enabled = false;

    std::cout << "Join server thread" << std::endl;

    // wait until thread stops
    if (this->serverThread.joinable()) {
      this->serverThread.join();
    }

    // should already be closed by server thread
    if (!this->closeSocket()) {
      std::cerr << "Failed to close socket." << std::endl;
      printError();
    }

    // dispose TLS context
    this->tlsContextPtr.reset();

    this->running = false;
  }
}

/**
 * requires includes
 * #include <sys/socket.h> for accept
 * #include <vector> for std::vector
 */
void Server::run() {
  std::cout << "Listening on port " << this->port << std::endl;

  std::vector<std::shared_ptr<Worker>> workers = {};

  while (this->enabled) {
    // select returns 0 if timeout or -1 if error
    int rc = select(this->listenSocket);
    if (rc < 0) {
      // select failed
      printError();
      std::cerr << "Select failed." << std::endl;
      break;
    } else if (rc == 0) {
      // timeout occurred
      continue;
    } else if (!this->enabled) {
      // server shall stop listening
      break;
    }  // else socket is ready to read from

    // listen socket must be non blocking even if we use select().
    // The pending connection might be closed before accept is called.

    // Accept a client socket
    sockaddr_in peerAddress;
    rc = accept(this->listenSocket, peerAddress);
    if (rc == -1) {
      // an error occurred or the connection has been closed before accept
      // could be executed
      std::cerr << "Failed to accept." << std::endl;
      printError();
      // continue. The select block will handle the termination if the socket
      // in not valid .
      continue;
    } else if (!this->enabled) {
      // server shall stop listening
      break;
    }  // else rc is the socket;

    int clientSocket = rc;

    // set non blocking mode
    if (!setSocketModeNonBlocking(clientSocket)) {
      std::cerr << "Failed to set client socket in non blocking mode"
                << std::endl;
      printError();
      continue;
    }

    // check TLS
    OpenSslWrapper::TlsPtr tlsPtr = OpenSslWrapper::TlsPtr(
        OpenSslWrapper::acceptTls(this->tlsContextPtr.get(), clientSocket));
    if (!tlsPtr) {
      // failed to accept connection
      closeClientSocket(clientSocket);
      continue;
    }

    // pass the accepted client socket to a worker thread
    std::shared_ptr<Worker> worker(new Worker(clientSocket, tlsPtr.release()));
    // put worker in list to be able to stop all workers
    workers.push_back(worker);
    // start worker to read
    worker->start();
  }

  // stop all workers
  for (std::shared_ptr<Worker> worker : workers) {
    if (worker) {
      worker->close();
    }
  }

  // close socket
  if (!this->closeSocket()) {
    std::cerr << "Failed to close socket." << std::endl;
    printError();
  }

  this->running = false;
  std::cout << "Stopped listening on port " << this->port << std::endl;
}

bool Server::setKeyFileName(const std::string& fileName) {
  if (this->isOpen()) {
    return false;
  }
  this->keyFileName = fileName;
  return true;
}

const std::string& Server::getKeyFileName() { return this->keyFileName; }

bool Server::setCertFileName(const std::string& fileName) {
  if (this->isOpen()) {
    return false;
  }
  this->certFileName = fileName;
  return true;
}

const std::string& Server::getCertFileName() { return this->certFileName; }

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
#endif
