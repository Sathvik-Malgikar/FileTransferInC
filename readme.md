# File Transfer Server in C

This is a simple file transfer server implemented in C using socket programming, multi-threading, and synchronization mechanisms.

## Code Description

### `server.c`

The `server.c` file contains the C source code for the file transfer server.  Here's a breakdown of the code:

**Includes**

* `stdio.h`: Standard input/output functions.

* `stdlib.h`: Standard library functions (e.g., memory allocation).

* `string.h`: String manipulation functions.

* `unistd.h`: Provides access to the POSIX operating system API (e.g., `read`, `write`, `close`).

* `arpa/inet.h`:  Functions for manipulating IP addresses.

* `pthread.h`: Functions for creating and managing threads.

* `semaphore.h`: Functions for creating and managing semaphores.

* `errno.h`: Defines macros for reporting errors.

**Defines**

* `SERVER_PORT`: The port number the server listens on (65432).

* `MAX_BUFFER_SIZE`: The maximum size of the buffer used for sending/receiving data (1024 bytes).

* `MAX_CLIENTS`: Maximum number of clients the server can handle

* `CHUNK_SIZE`: Size of file chunks (512 bytes)

**Typedefs**

* `client_t`: A structure to hold client information, including the socket file descriptor, the client's address, and the client's ID.

**Global Variables**

* `clients`: An array of `client_t` structures to store information about connected clients.

* `client_count`: The number of currently connected clients.

* `client_mutex`: A mutex to protect access to the `clients` array and `client_count`.

* `client_sem`: A semaphore to limit the number of concurrent connections.

**Functions**

* `calculate_checksum()`:  Calculates a very simplified checksum of the data. **Note:** This is *not* cryptographically secure.  Real applications should use a proper checksum library like OpenSSL.

* `send_data()`: Sends data to a client, with the ability to simulate packet loss or data corruption.

* `receive_data()`: Receives data from a client.

* `handle_client_connection()`:  This is the function that each thread executes to handle a connected client.  It receives the file, calculates the checksum, sends the file in chunks, and handles retransmissions.

* `main()`:  The main function:

  * Creates a socket.

  * Binds the socket to the server address and port.

  * Listens for incoming connections.

  * Accepts client connections in a loop.

  * Creates a new thread for each client connection.

## Compilation

To compile the program, use the following command:

gcc server.c -o server -lpthread
* `gcc`: The GNU C Compiler.

* `server.c`: The name of the source file.

* `-o server`:  Specifies the name of the output executable file (in this case, `server`).

* `-lpthread`: Links the POSIX Threads library.

## How to Run

To run the server, execute the compiled binary:

./server
## Testing

To test, you will need a client program.  You can use a tool like `netcat` (nc) to send a file to the server.  Here's a basic example:

1. **Start the server** in one terminal:

./server
2. **Create a test file** in another terminal:

echo "This is a test file." > test.txt
3. **Send the file using `netcat`** in the same terminal as the test file.  First, send the file size, then the file content:

filesize=$(stat -c %s "test.txt")echo -n "$filesize" | nc 127.0.0.1 65432  # Send the file sizesleep 1 #wait for ackcat test.txt | nc 127.0.0.1 65432 #send file data
* Replace `127.0.0.1` with the actual server IP address if needed.

* The server will print the file size, the calculated checksum, and the chunks sent.

## Sample Output

Here's a sample output of the server:

Server listening on port 65432Thread 1: New connection from 127.0.0.1:54321Thread 1: Receiving file of size 21 bytesThread 1: Received 21 bytes of file dataThread 1: Calculated checksum: 2925499788Thread 1: Number of chunks: 1Thread 1: Sending chunk 0Thread 1: Transfer result: SUCCESSThread 1: Connection closed
## Explanation of Sample Output

* The server starts and listens on port 65432.

* A client connects from port 54321 on the same machine (127.0.0.1).

* The server receives a file of 21 bytes.

* The server calculates a checksum (2925499788 in this example).

* The server splits the file into 1 chunk.

* The server sends the chunk to the client.

* The server sends the transfer result to the client.

* The client disconnects.
# File Transfer Server in C

This is a simple file transfer server implemented in C using socket programming, multi-threading, and synchronization mechanisms.

## Code Description

### `server.c`

The `server.c` file contains the C source code for the file transfer server.  Here's a breakdown of the code:

**Includes**

* `stdio.h`: Standard input/output functions.

* `stdlib.h`: Standard library functions (e.g., memory allocation).

* `string.h`: String manipulation functions.

* `unistd.h`: Provides access to the POSIX operating system API (e.g., `read`, `write`, `close`).

* `arpa/inet.h`:  Functions for manipulating IP addresses.

* `pthread.h`: Functions for creating and managing threads.

* `semaphore.h`: Functions for creating and managing semaphores.

* `errno.h`: Defines macros for reporting errors.

**Defines**

* `SERVER_PORT`: The port number the server listens on (65432).

* `MAX_BUFFER_SIZE`: The maximum size of the buffer used for sending/receiving data (1024 bytes).

* `MAX_CLIENTS`: Maximum number of clients the server can handle

* `CHUNK_SIZE`: Size of file chunks (512 bytes)

**Typedefs**

* `client_t`: A structure to hold client information, including the socket file descriptor, the client's address, and the client's ID.

**Global Variables**

* `clients`: An array of `client_t` structures to store information about connected clients.

* `client_count`: The number of currently connected clients.

* `client_mutex`: A mutex to protect access to the `clients` array and `client_count`.

* `client_sem`: A semaphore to limit the number of concurrent connections.

**Functions**

* `calculate_checksum()`:  Calculates a very simplified checksum of the data. **Note:** This is *not* cryptographically secure.  Real applications should use a proper checksum library like OpenSSL.

* `send_data()`: Sends data to a client, with the ability to simulate packet loss or data corruption.

* `receive_data()`: Receives data from a client.

* `handle_client_connection()`:  This is the function that each thread executes to handle a connected client.  It receives the file, calculates the checksum, sends the file in chunks, and handles retransmissions.

* `main()`:  The main function:

  * Creates a socket.

  * Binds the socket to the server address and port.

  * Listens for incoming connections.

  * Accepts client connections in a loop.

  * Creates a new thread for each client connection.

## Compilation

To compile the program, use the following command:

gcc server.c -o server -lpthread
* `gcc`: The GNU C Compiler.

* `server.c`: The name of the source file.

* `-o server`:  Specifies the name of the output executable file (in this case, `server`).

* `-lpthread`: Links the POSIX Threads library.

## How to Run

To run the server, execute the compiled binary:

./server
## Testing

To test, you will need a client program.  You can use a tool like `netcat` (nc) to send a file to the server.  Here's a basic example:

1. **Start the server** in one terminal:

./server
2. **Create a test file** in another terminal:

echo "This is a test file." > test.txt
3. **Send the file using `netcat`** in the same terminal as the test file.  First, send the file size, then the file content:

filesize=$(stat -c %s "test.txt")echo -n "$filesize" | nc 127.0.0.1 65432  # Send the file sizesleep 1 #wait for ackcat test.txt | nc 127.0.0.1 65432 #send file data
* Replace `127.0.0.1` with the actual server IP address if needed.

* The server will print the file size, the calculated checksum, and the chunks sent.

## Sample Output

Here's a sample output of the server:

Server listening on port 65432Thread 1: New connection from 127.0.0.1:54321Thread 1: Receiving file of size 21 bytesThread 1: Received 21 bytes of file dataThread 1: Calculated checksum: 2925499788Thread 1: Number of chunks: 1Thread 1: Sending chunk 0Thread 1: Transfer result: SUCCESSThread 1: Connection closed
## Explanation of Sample Output

* The server starts and listens on port 65432.

* A client connects from port 54321 on the same machine (127.0.0.1).

* The server receives a file of 21 bytes.

* The server calculates a checksum (2925499788 in this example).

* The server splits the file into 1 chunk.

* The server sends the chunk to the client.

* The server sends the transfer result to the client.

* The client disconnects.
