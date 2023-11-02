#include "cgi.hpp"

static std::vector<std::string>	set_environment( std::string script, HttpRequest request, std::string root )
{
	std::vector<std::string>	env;

	env.resize(12);
	env[0] = "REQUEST_METHOD=" + request.method;
	env[1] = "SERVER_PROTOCOL=HTTP/1.1";
	std::string	path_info = request.path_info.empty() ? "\"\"" : request.path_info;
	env[2] = "PATH_INFO=" + path_info;
	std::string query_string = request.query_string.empty() ? "\"\"" : request.query_string.substr(1);
	env[3] = "QUERY_STRING=" + query_string;
	env[4] = "CONTENT_TYPE=text/html";
	env[5] = "CONTENT_LENGTH=" + ft_itoa(request.body.length());
	std::string	document_root = script.substr(0, script.find(root) + root.length());
	env[6] = "DOCUMENT_ROOT=" + document_root;
	env[7] = "SCRIPT_FILENAME=" + script;
	std::string	script_name = script.substr(document_root.length());
	env[8] = "SCRIPT_NAME=" + script_name;
	env[9] = "REDIRECT_STATUS=200";
	env[10] = "REQUEST_BODY=" + request.body;
	env[11] = "DISPLAY=:0";
	return (env);
}

static void	handle_child( int const parent_fd, int const child_fd, std::string bin,
						  std::string script, HttpRequest request, std::string root )
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
	std::vector<std::string>	env = set_environment(script, request, root);
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
	int			nbytes = read(fd, buf, 1023);
	std::string	cgi_output;

	while (nbytes != 0)
	{
		buf[nbytes] = '\0';
		cgi_output.append(buf);
		nbytes = read(fd, buf, 1023);
	}
	return (cgi_output);
}

static std::string	handle_parent( pid_t pid, int const parent_fd )
{
	long long		old_timestamp, new_timestamp;
	unsigned int	timeout = 30000;
	std::string		output;

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

std::string	handle_cgi( std::string bin, std::string script, HttpRequest request, std::string root )
{
	pid_t				pid;
	int					fd[2];
	static const int	parent = 0, child = 1;
	std::string			cgi_output;

	if (pipe(fd) == -1)
	{
		perror("pipe");
		throw std::exception();
	}
	if (request.method == "POST")
		write(fd[child], request.body.c_str(), request.body.length());
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		throw std::exception();
	}
	else if (pid == 0)
		handle_child(fd[parent], fd[child], bin, script, request, root);
	else if (pid > 0)
	{
		close(fd[child]);
		cgi_output = handle_parent(pid, fd[parent]);
	}
	return (cgi_output);
}
