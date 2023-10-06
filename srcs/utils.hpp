#ifndef UTILS_HPP
# define UTILS_HPP

# include <stdlib.h>
# include <sys/time.h>

# include "../includes/main.hpp"

int			ft_isspace( char c );
int			ft_atoi( char const *str );
bool		ft_isalnum( char c );
char		ft_tolower( char c );
char		ft_toupper( char c );
bool		ft_empty( char const *str );
long long	timestamp( void );
std::string ft_itoa( int n );
std::string get_time_string( void );

#endif
