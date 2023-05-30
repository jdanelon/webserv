#include "nnn.hpp"

nnn::nnn( void )
{
	return ;
}

nnn::nnn( nnn const &obj )
{
	nnn::operator = (obj);
	return ;
}

nnn &nnn::operator = ( nnn const &obj )
{
	if (this != &obj)
	{
		this->XXX = obj.XXX();
	}
	return (*this);
}

nnn::~nnn( void )
{
	return ;
}
