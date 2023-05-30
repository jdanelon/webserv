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
