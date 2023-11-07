# Webserver

## How to run?

```
make

./webserv conf-files/oneServerTest.conf
```
## How to test?

```
make testing
```

# Non-blocking I/O

Our web server implements a non-blocking IO system using poll, which allows us to efficiently handle multiple connections with a single thread. This approach is akin to time-division multiplexing, where the server attends to each active connection in small time slices, rather than dedicating a continuous thread or process per connection.

## Poll Mechanism
We utilize the poll system call to monitor multiple file descriptors (sockets in our case) to see if any of them are ready for IO operations. This is a scalable method for handling numerous simultaneous connections, as poll returns a list of sockets that are ready for reading or writing without blocking the server's execution.

## Service Strategy
When servicing a connection, instead of sending or receiving large chunks of data at once, we process data in smaller fragments. For example, when a client is downloading a large file, the server does not attempt to send the entire file in one go. It instead sends a portion of the file and then yields control back to the poll, which then checks for other sockets that may need servicing.

## Advantages of Time Division Multiplexing
This multiplexed approach has several advantages:

Scalability: It can handle a large number of connections simultaneously without the need for multiple threads, thus saving on context-switching overhead and memory usage.
Fairness: Each connection gets a fair amount of attention, preventing any single connection from monopolizing the server's resources.
Responsiveness: The server remains responsive to new incoming requests, even while processing ongoing ones, because it does not block on long-running IO operations.
