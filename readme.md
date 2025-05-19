# File Transfer Server (C)

This is a multi-threaded TCP server in C that accepts file uploads from multiple clients, simulates network errors (like packet loss and corruption), verifies file integrity using a simple checksum, and sends back the file in chunks.

## Features
- Concurrent file transfers using POSIX threads
- Simulated packet loss and corruption
- Custom checksum verification (simplified)
- Chunked file retransmission with client feedback
- Supports up to 10 clients

## How to Compile
```bash
gcc file_transfer.c -o file_transfer -lpthread
```

## How to Run
```bash
./file_transfer
```
Server listens on port 65432.

## Sample Output
```
Server listening on port 65432
Thread 1: New connection from 127.0.0.1:51432
Thread 1: Receiving file of size 2048 bytes
Thread 1: Received 2048 bytes of file data
Thread 1: Calculated checksum: 123456789
Thread 1: Number of chunks: 4
Simulating packet loss
Simulating data corruption
Thread 1: Transfer result: SUCCESS
Thread 1: Connection closed
```
Note: This is the server side only. A client program must be implemented separately to test the full flow.