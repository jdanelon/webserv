# webserver

## How to run?

```
make

./webserv conf-files/oneServer.conf
```


# Web Server To-Do List

- [x] **Configuration File**
  - [x] Program should take a configuration file as an argument.
  - [x] If not provided, use a default path for the configuration file.

- [x] **Execution Restrictions**
  - [x] Cannot use `execve` to run another web server.

- [x] **Blocking and Client Handling**
  - [x] Server must be non-blocking.
  - [x] Ensure clients can be disconnected properly if necessary.
  - [x] A request to the server should never hang indefinitely.

- [x] **I/O Operations**
  - [x] Use only 1 `poll()` (or equivalent) for all I/O operations (including listening to clients).
  - [x] Ensure `poll()` (or equivalent) checks both read and write operations simultaneously.
  - [x] Never perform a read or write operation without passing through `poll()` (or equivalent).
  - [x] After a read or write operation, checking the value of `errno` is strictly prohibited.
  
- [x] **Macros and Defines**
  - [x] Allowed to use macros and defines like `FD_SET`, `FD_CLR`, `FD_ISSET`, `FD_ZERO`.

- [ ] **Web Browser Compatibility**
  - [ ] Ensure the server is compatible with the web browser of your choice.

- [ ] **Compliance and Comparisons**
  - [ ] Take NGINX as HTTP 1.1 compliant for comparisons related to headers and answer behaviors.
  - [ ] HTTP response status codes should be accurate.

- [x] **Error Pages**
  - [x] Server must have default error pages if none are provided by the user.

- [x] **Restrictions**
  - [x] You can’t use `fork` for anything other than CGI.

- [ ] **Functionality**
  - [x] Must be able to serve a fully static website.
  - [ ] Clients should be able to upload files to the server.
  - [ ] Support at least the `GET`, `POST`, and `DELETE` HTTP methods.

- [ ] **Stress Testing**
  - [ ] Ensure the server remains available under heavy loads.

- [x] **Port Listening**
  - [x] The server should be able to listen on multiple ports as specified in the configuration file.

- [x] **Configuration File Reading**
  - [x] No need to use `poll()` (or equivalent) before reading the configuration file.


# Configuration File To-Do List

## **General Server Configuration**

- [x] **Port and Host**
  - [x] Ability to specify the port and host for each server.

- [x] **Server Names**
  - [x] Option to set up server names.
  - [x] Define default server behavior (first server specified for a host:port is default).

- [x] **Error Pages**
  - [x] Ability to set up default error pages for the server.

- [x] **Client Body Size**
  - [x] Define a limit for client body size.

## **Route-specific Configuration**

- [x] **HTTP Methods**
  - [x] Specify a list of accepted HTTP methods for a route.

- [x] **Redirection**
  - [x] Set up HTTP redirection for specific routes.

- [x] **Directory or File Definition**
  - [x] Define directory or file from where content should be fetched.

- [x] **Directory Listing**
  - [x] Ability to turn on/off directory listing for routes.

- [x] **Default File for Directory Requests**
  - [x] Set a default file to be served if a request points to a directory.

- [ ] **CGI Execution**
  - [x] Enable CGI execution based on file extensions (e.g., .php).
  - [ ] Ensure CGI compatibility with POST and GET methods.
  - [ ] Ability for routes to accept uploaded files.
  - [ ] Specify directory where uploaded files should be saved.
  - [x] Configure server to call CGI with the requested file as the first argument.
  - [x] Ensure CGI is executed in the correct directory for relative path file access.
  - [x] Server must be compatible with at least one CGI (php-CGI, Python, etc.)

## **CGI Specific Considerations**

- [x] Understand what a CGI is.
- [x] For chunked requests, the server must unchunk it; the CGI expects EOF as the end of the body.
- [x] If the CGI output doesn't provide content_length, use EOF to mark the end of returned data.

## Last Sprint:

- [ ] Implement Content lenght required error
- [ ] Implement Upload files

- [ ] Adicionar Tests:
  - [ ] Test do timeout con telnet
  - [ ] Test do default error page
  - [ ] Test custom error page
  - [ ] Test para DELETE //curl -X DELETE localhost:3490/delete/ringo.txt , para pasta e 403
  - [ ] Test server multiple ports
  - [ ] Test Limit client body size.
  - [ ] Define a list of accepted HTTP methods for the route.