# webserver

## How to run?

```
make

./webserv conf-files/oneServer.conf
```


# Web Server To-Do List

- [ ] **Configuration File**
  - [x] Program should take a configuration file as an argument.
  - [ ] If not provided, use a default path for the configuration file.

- [x] **Execution Restrictions**
  - [x] Cannot use `execve` to run another web server.

- [ ] **Blocking and Client Handling**
  - [ ] Server must be non-blocking.
  - [ ] Ensure clients can be disconnected properly if necessary.
  - [ ] A request to the server should never hang indefinitely.

- [ ] **I/O Operations**
  - [x] Use only 1 `poll()` (or equivalent) for all I/O operations (including listening to clients).
  - [x] Ensure `poll()` (or equivalent) checks both read and write operations simultaneously.
  - [ ] Never perform a read or write operation without passing through `poll()` (or equivalent).
  - [ ] After a read or write operation, checking the value of `errno` is strictly prohibited.
  
- [ ] **Macros and Defines**
  - [ ] Allowed to use macros and defines like `FD_SET`, `FD_CLR`, `FD_ISSET`, `FD_ZERO`.

- [ ] **Web Browser Compatibility**
  - [ ] Ensure the server is compatible with the web browser of your choice.

- [ ] **Compliance and Comparisons**
  - [ ] Take NGINX as HTTP 1.1 compliant for comparisons related to headers and answer behaviors.
  - [ ] HTTP response status codes should be accurate.

- [ ] **Error Pages**
  - [ ] Server must have default error pages if none are provided by the user.

- [x] **Restrictions**
  - [x] You can’t use `fork` for anything other than CGI.

- [ ] **Functionality**
  - [ ] Must be able to serve a fully static website.
  - [ ] Clients should be able to upload files to the server.
  - [ ] Support at least the `GET`, `POST`, and `DELETE` HTTP methods.

- [ ] **Stress Testing**
  - [ ] Ensure the server remains available under heavy loads.

- [ ] **Port Listening**
  - [ ] The server should be able to listen on multiple ports as specified in the configuration file.

- [ ] **Configuration File Reading**
  - [ ] No need to use `poll()` (or equivalent) before reading the configuration file.


# Configuration File To-Do List

## **General Server Configuration**

- [ ] **Port and Host**
  - [ ] Ability to specify the port and host for each server.
  
- [ ] **Server Names**
  - [ ] Option to set up server names.
  - [ ] Define default server behavior (first server specified for a host:port is default).

- [ ] **Error Pages**
  - [ ] Ability to set up default error pages for the server.

- [ ] **Client Body Size**
  - [ ] Define a limit for client body size.

## **Route-specific Configuration**

- [ ] **HTTP Methods**
  - [ ] Specify a list of accepted HTTP methods for a route.

- [ ] **Redirection**
  - [ ] Set up HTTP redirection for specific routes.

- [ ] **Directory or File Definition**
  - [ ] Define directory or file from where content should be fetched.

- [ ] **Directory Listing**
  - [ ] Ability to turn on/off directory listing for routes.

- [ ] **Default File for Directory Requests**
  - [ ] Set a default file to be served if a request points to a directory.

- [ ] **CGI Execution**
  - [ ] Enable CGI execution based on file extensions (e.g., .php).
  - [ ] Ensure CGI compatibility with POST and GET methods.
  - [ ] Ability for routes to accept uploaded files.
  - [ ] Specify directory where uploaded files should be saved.
  - [ ] Configure server to call CGI with the requested file as the first argument.
  - [ ] Ensure CGI is executed in the correct directory for relative path file access.
  - [ ] Server must be compatible with at least one CGI (php-CGI, Python, etc.)

## **CGI Specific Considerations**

- [ ] Understand what a CGI is.
- [ ] For chunked requests, the server must unchunk it; the CGI expects EOF as the end of the body.
- [ ] If the CGI output doesn't provide content_length, use EOF to mark the end of returned data.
