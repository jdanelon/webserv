#include "cgi.hpp"

static std::vector<std::string>	set_environment( std::string script, HttpRequest request )
{
	std::vector<std::string>	env;
	std::string					root(std::getenv("PWD"));
	// std::string					uri("/cgi-bin/hello.py?first_name=John&last_name=Danelon");
	// std::string					uri("/cgi-bin/hello.py");

	env.resize(10);
	env[0] = "REQUEST_METHOD=" + request.method;
	// env[0] = "REQUEST_METHOD=POST";
	size_t idx = request.uri.find("?");
	std::string query_string = idx != std::string::npos ? request.uri.substr(idx + 1) : "\"\"";
	env[1] = "QUERY_STRING=" + query_string;
	env[2] = "CONTENT_TYPE=text/html";
	env[3] = "CONTENT_LENGTH=" + ft_itoa(request.body.length());
	// env[3] = "CONTENT_LENGTH=34";
	env[4] = "DOCUMENT_ROOT=" + root;
	env[5] = "SCRIPT_FILENAME=" + root + "/cgi" + script;
	env[6] = "SCRIPT_NAME=/cgi/" + script;
	env[7] = "REDIRECT_STATUS=200";
	env[8] = "REQUEST_BODY=" + request.body;
	// env[8] = "REQUEST_BODY=\"\"";
	env[9] = "DISPLAY=:0";
	return (env);
}

static void	handle_child( int const parent_fd, int const child_fd, std::string bin,
						  std::string script, HttpRequest request )
{
	// child will:
	// - link child fd to STDOUT
	// - what gets printed on child's STDOUT is sent to parent fd
	dup2(parent_fd, STDIN_FILENO);
	close(parent_fd);
	dup2(child_fd, STDOUT_FILENO);
	close(child_fd);

	// - set array with execve args
	const char	**argv = new const char *[3];
	argv[0] = bin.c_str();
	argv[1] = script.c_str();
	argv[2] = NULL;

	// - set environ with CGI variables
	std::vector<std::string>	env = set_environment(script, request);
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

std::string	handle_cgi( std::string bin, std::string script, HttpRequest request )
{
	pid_t				pid;
	int					fd[2];
	static const int	parent = 0, child = 1;
	std::string			cgi_output;

	pipe(fd);
	if (request.method == "POST")
		write(fd[child], request.body.c_str(), request.body.length());
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		throw std::exception();
	}
	else if (pid == 0)
		handle_child(fd[parent], fd[child], bin, script, request);
	else if (pid > 0)
	{
		close(fd[child]);
		cgi_output = handle_parent(pid, fd[parent]);
	}
	return (cgi_output);
}
