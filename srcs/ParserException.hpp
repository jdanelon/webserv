#ifndef PARSEREXCEPTION_HPP
# define PARSEREXCEPTION_HPP

# include <iostream>
# include <cstring>

class ParserException : public std::exception
{

	public:

		ParserException( void );
		explicit ParserException( std::string const &str );
		ParserException( ParserException const &obj );
		ParserException &operator = ( ParserException const &obj );
		virtual ~ParserException( void ) throw();

		char const	*what( void ) const throw();

	protected:

		static std::string	_msg;

};

#endif
