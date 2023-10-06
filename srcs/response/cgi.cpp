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
#include <cstdlib>

static long long	timestamp( void )
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

static std::vector<std::string>	set_environment( std::string script )
{
	std::vector<std::string>	env;
	std::string					root(std::getenv("PWD"));
	// std::string					uri("/cgi-bin/hello.py?first_name=John&last_name=Danelon");
	std::string					uri("/cgi-bin/hello.py");

	// TO-DO: Change cgi script

	env.resize(10);
	// env[0] = "REQUEST_METHOD=" + request.method;
	env[0] = "REQUEST_METHOD=POST";
	size_t idx = uri.find("?");
	std::string query_string = idx != std::string::npos ? uri.substr(idx + 1) : "\"\"";
	env[1] = "QUERY_STRING=" + query_string;
	env[2] = "CONTENT_TYPE=text/html";
	// env[3] = "CONTENT_LENGTH=" + ft_itoa(request.body.length());
	env[3] = "CONTENT_LENGTH=34";
	env[4] = "DOCUMENT_ROOT=" + root;
	env[5] = "SCRIPT_FILENAME=" + root + "/cgi" + script;
	env[6] = "SCRIPT_NAME=/cgi/" + script;
	env[7] = "REDIRECT_STATUS=200";
	// env[8] = "REQUEST_BODY=" + request.body;
	env[8] = "REQUEST_BODY=\"\"";
	env[9] = "DISPLAY=:0";
	return (env);
}

static void	handle_child( int const parent_fd, int const child_fd, std::string bin, std::string script )
{
	// child will:
	// - link child fd to STDOUT
	// - what gets printed on child's STDOUT is sent to parent fd
	dup2(parent_fd, STDIN_FILENO);
	close(parent_fd);
	dup2(child_fd, STDOUT_FILENO);
	close(child_fd);

	// write(STDIN_FILENO, "first_name=John&last_name=Danelon", 34);
	// - set array with execve args
	const char	**argv = new const char *[4];
	argv[0] = bin.c_str();
	argv[1] = script.c_str();
	// char *post_string = request.method == "POST" ? "first_name=John&last_name=Danelon" : NULL;
	argv[2] = NULL;
	// argv[2] = "first_name=John&last_name=Danelon";
	argv[3] = NULL;

	// - set environ with CGI variables
	std::vector<std::string>	env = set_environment(script);
	const char	**envp = new const char *[11];
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
	envp[10] = NULL;

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

	// socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
	// fcntl(fd[parent], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	// fcntl(fd[child], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	pipe(fd);
	// if (request.method == "POST")
	// write(fd[child], "first_name=John&last_name=Danelon", 34);
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		throw std::exception();
	}
	else if (pid == 0)
	{
		// close(fd[parent]);
		handle_child(fd[parent], fd[child], bin, script);
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
	std::string	bin("/usr/bin/python3");
	// std::string	script("hello.py");
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
