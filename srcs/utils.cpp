#include "utils.hpp"

int	ft_isspace( char c )
{
	if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
		return (1);
	return (0);
}

static int ft_strlen(char const *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

int	ft_atoi( char const *str )
{
	int	i;
	int	num;

	i = 0;
	num = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		num = (num * 10) + (str[i] - '0');
		i++;
	}
	if (i != ft_strlen(str))
		return (-1);
	return (num);
}

bool	ft_isalnum( char c )
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return (true);
	return (false);
}

char	ft_tolower( char c )
{
	if (c >= 'A' && c <= 'Z')
		return (c - 'A' + 'a');
	return (c);
}

char	ft_toupper( char c )
{
	if (c >= 'a' && c <= 'z')
		return (c - 'a' + 'A');
	return (c);
}

bool	ft_empty( char const *str )
{
	int	len;
	int	whitespace;

	len = ft_strlen(str);
	whitespace = 0;
	while (ft_isspace(str[whitespace]))
		whitespace++;
	if (len == whitespace)
		return (true);
	return (false);
}

long long	timestamp( void )
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

std::string	ft_itoa( int n )
{
	std::string	str;

	while (n >= 10)
	{
		str.insert(str.begin(), n % 10 + '0');
		n /= 10;
	}
	str.insert(str.begin(), n % 10 + '0');
	return (str);
}

std::string	get_time_string( void )
{
	time_t		now;
	struct tm	*timeinfo;
	char		time_str[30];

	now = std::time(NULL);
	timeinfo = std::localtime(&now);
	std::strftime(time_str, sizeof(time_str), "%a, %d %b %Y %T %Z", timeinfo);
	return std::string(time_str);
}

std::string constructPath(const std::string &rootPath, const std::string &uri) {
    std::string newRootPath = rootPath;
    std::string newUri = uri;

    // // Remove trailing slash from rootPath, if it exists
    // if (!newRootPath.empty() && newRootPath.back() == '/') {
    //     newRootPath.pop_back();
    // }

    // // Add leading slash to uri, if it doesn't exist
    // if (newUri.empty() || newUri.front() != '/') {
    //     newUri = "/" + newUri;
    // }

    return newRootPath + newUri;
}
