#include "Location.hpp"

Location::Location( void )
{
	this->alias = "";
	this->client_max_body_size = -1;
	this->autoindex = -1;
	this->redirect = std::make_pair(0, "");
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
		this->alias = obj.alias;
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
	if (this->index.empty())
		this->index = srv.index;
	if (this->limit_except.empty())
	{
		this->limit_except.push_back("GET");
		this->limit_except.push_back("POST");
		this->limit_except.push_back("DELETE");
	}
	if (this->client_max_body_size == -1)
		this->client_max_body_size = srv.client_max_body_size;
	if (this->autoindex == -1)
		this->autoindex = srv.autoindex;
	if (this->cgi.empty())
		this->cgi = srv.cgi;
	if (this->redirect.first == 0 && this->redirect.second == "")
		this->redirect = srv.redirect;
	if (this->upload == -1)
		this->upload = srv.upload;
	if (this->upload_store.empty())
		this->upload_store = srv.upload_store;
}
