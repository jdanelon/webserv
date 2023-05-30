#include "utils.hpp"

int	ft_isspace( char c )
{
	if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
		return (1);
	return (0);
}

int	ft_atoi( char const *str )
{
	int	i;
	int	num;

	i = 0;
	num = 0;
	while (str[i])
	{
		num = (num * 10) + (str[i] - '0');
		i++;
	}
	return (num);
}
