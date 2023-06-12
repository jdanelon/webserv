#include "Location.hpp"

Location::Location( void )
{
	this->root = "";
	this->client_max_body_size = -1;
	this->autoindex = -1;
	this->upload = -1;
	this->upload_store = "";
	return ;
}

Location::Location( Location const &obj )
{
	Location::operator = (obj);
	return ;
}

Location &Location::operator = ( Location const &obj )
{
	if (this != &obj)
	{
		this->root = obj.root;
		this->index = obj.index;
		this->limit_except = obj.limit_except;
		this->client_max_body_size = obj.client_max_body_size;
		this->autoindex = obj.autoindex;
		this->cgi = obj.cgi;
		this->redirect = obj.redirect;
		this->upload = obj.upload;
		this->upload_store = obj.upload_store;
	}
	return (*this);
}

Location::~Location( void )
{
	return ;
}

void	Location::fill_with_defaults( Server const &srv )
{
	if (this->root.empty())
		this->root = srv.root;
	if (this->index.empty())
		this->index = srv.index;
	// if (!this->limit_except.size())
	// 	this->limit_except = ;
	if (this->client_max_body_size == -1)
		this->client_max_body_size = srv.client_max_body_size;
	if (this->autoindex == -1)
		this->autoindex = srv.autoindex;
	if (this->cgi.empty())
		this->cgi = srv.cgi;
	if (this->redirect.first == 0 && this->redirect.second == "")
		this->redirect = srv.redirect;
	// if (this->upload == -1)
	// 	this->upload = ;
	if (this->upload_store.empty())
		this->upload_store = "";
	return ;
}
