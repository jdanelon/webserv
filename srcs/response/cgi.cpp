#include <iostream> //basic c++
#include <cstring> //string
#include <sys/types.h> //pid_t
#include <sys/socket.h> //socketpair
#include <fcntl.h> //fcntl
#include <unistd.h> //fork, dup2
#include <cstdio> //perror
#include <vector> //vector
#include <sys/wait.h> //waitpid
#include <sys/time.h> // gettimeofday

static long long	timestamp( void )
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

static std::vector<std::string>	set_environment( void )
{
	std::vector<std::string>	env;

	// TO-DO: 1 - Change cgi script
	// TO-DO: 2 - Check which variables must be set for cgi script
	// TO-DO: 3 - Check which informations are needed to set variables

	env.resize(12);
	env[0] = "REQUEST_METHOD=GET";
	env[1] = "QUERY_STRING=\"\"";
	env[2] = "CONTENT_TYPE=text/html";
	env[3] = "CONTENT_LENGTH=12";
	env[4] = "HTTP_COOKIE=None";
	env[5] = "HTTP_USER_AGENT=None";
	env[6] = "DOCUMENT_ROOT=PATH";
	env[7] = "SCRIPT_FILENAME=PATH + cgi.py";
	env[8] = "SCRIPT_NAME=cgi.py";
	env[9] = "REDIRECT_STATUS=200";
	env[10] = "REQUEST_BODY=Hello World!";
	env[11] = "DISPLAY=:0";

	// setenv("CONTENT_LENGTH", "42", 1); // if msg has body present
	// setenv("CONTENT_TYPE", "", 1); // get from header
	// setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	// // setenv("PATH_INFO", request_uri, 1);
	// // setenv("PATH_TRANSLATED", path_to_cgi_script, 1);
	// setenv("QUERY_STRING", "", 1); // string after cgi script
	// // setenv("REMOTE_ADDR", network_address_of_the_client, 1);
	// // setenv("REMOTE_HOST", fully_qualified_domain_name_of_the_client, 1);
	// // setenv("REQUEST_METHOD", method, 1);
	// setenv("SCRIPT_NAME", "", 1); // url_of_the_script_being_executed
	// // setenv("SERVER_NAME", host_ip, 1);
	// // setenv("SERVER_PORT", host_port, 1);
	// setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	// setenv("SERVER_SOFTWARE", "webserv/1.0", 1);
	return (env);
}

static void	handle_child( int const child_fd, std::string bin, std::string script )
{
	// child will:
	// - link child fd to STDOUT
	// - what gets printed on child's STDOUT is sent to parent fd
	// dup2(parent_fd, STDOUT_FILENO);
	// close(parent_fd);
	dup2(child_fd, STDOUT_FILENO);
	close(child_fd);

	// - set array with execve args
	const char	**argv = new const char *[3];
	argv[0] = bin.c_str();
	argv[1] = script.c_str();
	argv[2] = NULL;

	// - set environ with CGI variables
	std::vector<std::string>	env = set_environment();
	const char	**envp = new const char *[13];
	envp[0] = env[0].c_str();
	envp[1] = env[1].c_str();
	envp[2] = env[2].c_str();
	envp[3] = env[3].c_str();
	envp[4] = env[4].c_str();
	envp[5] = env[5].c_str();
	envp[6] = env[6].c_str();
	envp[7] = env[7].c_str();
	envp[8] = env[8].c_str();
	envp[9] = env[9].c_str();
	envp[10] = env[10].c_str();
	envp[11] = env[11].c_str();
	envp[12] = NULL;

	// - run binary based on cgi script (execve)
	execve(argv[0], (char **)argv, (char **)envp);
}

static std::string	get_cgi_output( int fd )
{
	char		buf[1024];
	int			nbytes = read(fd, buf, 1024);
	std::string	cgi_output;

	while (nbytes != 0)
	{
		buf[nbytes] = '\0';
		cgi_output.append(buf);
		nbytes = read(fd, buf, 1024);
	}
	return (cgi_output);
}

static std::string	handle_parent( pid_t pid, int const parent_fd )
{
	long long	old_timestamp, new_timestamp;
	int			timeout = 30000;
	std::string	output;

	old_timestamp = timestamp();
	while (1)
	{
		pid_t result = waitpid(pid, NULL, WNOHANG);
		if (result == -1)
		{
			perror("waitpit");
			throw std::exception();
		}
		if (result != 0)
			break ;
		new_timestamp = timestamp();
		unsigned int runtime = new_timestamp - old_timestamp;
		if (runtime >= timeout)
		{
			kill(pid, SIGTERM);
			return (std::string());
		}
		usleep(1000);
	}
	output = get_cgi_output(parent_fd);
	close(parent_fd);
	return (output);
}

static std::string	handle_cgi( std::string bin, std::string script )
{
	pid_t				pid;
	int					fd[2];
	static const int	parent = 0, child = 1;
	std::string			cgi_output;

	socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
	fcntl(fd[parent], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fcntl(fd[child], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	// if (request.method == "POST")
	// 	write(fd[child], "Hello, World!\n", 15);
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		throw std::exception();
	}
	else if (pid == 0)
	{
		close(fd[parent]);
		handle_child(fd[child], bin, script);
	}
	else if (pid > 0)
	{
		close(fd[child]);
		cgi_output = handle_parent(pid, fd[parent]);
	}
	return (cgi_output);
}

int	main( void )
{
	// binary must be: absolute path + executable file
	std::string	bin("/home/jdanelon/anaconda3/bin/python3");
	// std::string	script("cgi.py");
	// std::string	script("new_cgi.py");
	// std::string	script("loop.py");
	std::string	script("64.py");

	std::string	output = handle_cgi(bin, script);
	if (output.empty())
	{
		std::cout << "CGI has timed out!" << std::endl;
	}
	else
		std::cout << "CGI output:\n" << output;
	return (0);
}
