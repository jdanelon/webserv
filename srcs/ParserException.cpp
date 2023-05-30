#include "ParserException.hpp"

std::string	ParserException::_msg = "";

ParserException::ParserException( void )
{
	return ;
}

ParserException::ParserException( std::string const &str )
{
	this->_msg = str;
	return ;
}

ParserException::ParserException( ParserException const &obj )
{
	ParserException::operator = (obj);
	return ;
}

ParserException &ParserException::operator = ( ParserException const &obj )
{
	if (this != &obj)
	{
		this->_msg = obj._msg;
	}
	return (*this);
}

ParserException::~ParserException( void ) throw()
{
	return ;
}

char const	*ParserException::what( void ) const throw()
{
	return (this->_msg.c_str());
}
