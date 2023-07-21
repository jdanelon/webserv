#include <iostream> //basic c++
#include <unistd.h> //fork, dup2
#include <sys/socket.h> //socketpair
#include <fcntl.h> //fcntl
#include <cstdio> //perror
#include <cstdlib> //setenv
#include <sys/wait.h> //waitpid

extern char	**environ;

int	main( void )
{
	// binary must be: absolute path + executable file
	std::string	bin("/home/jdanelon/anaconda3/bin/python3");
	std::string	script("cgi.py");

	pid_t				pid;
	int					fd[2], status;
	static const int	parent = 0, child = 1;

	socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
	fcntl(fd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fcntl(fd[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	// fork webserv
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		return (-1);
	}
	else if (pid == 0)
	{
		// child will:
		// - close parent fd
		close(fd[parent]);
		// - link child fd to STDOUT
		// - what gets printed on child's STDOUT is sent to parent fd 
		dup2(fd[child], STDOUT_FILENO);
		// - set environment variables that can be accessed from cgi script (setenv)
		setenv("jdanelon", "42", 1);

		// - run binary based on cgi script (execve)
		const char	**arr = new const char *[3];
		arr[0] = bin.c_str();
		arr[1] = script.c_str();
		arr[2] = NULL;
		execve(arr[0], (char **)arr, environ);
		// - close child fd
		close(fd[child]);
	}
	// parent will:
	// - wait for child to finish (waitpid)
	waitpid(pid, &status, 0);
	// - close child fd
	close(fd[child]);

	// access message body generated by script
	char		buf[1024];
	int			nbytes = read(fd[parent], buf, 1024);
	std::string	cgi_output;
	while (nbytes != 0)
	{
		buf[nbytes] = '\0';
		cgi_output.append(buf);
		nbytes = read(fd[parent], buf, 1024);
	}
	// cgi output saved as a long string
	std::cout << cgi_output;

	// close parent fd
	close(fd[parent]);
	return (0);
}
