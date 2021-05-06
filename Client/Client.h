#pragma once

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "../Shared/Event.h"
#include "../Shared/GameState.h"
#include "../Shared/Camera.h"
#include <string>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include "Transform.h"

#include <glm/gtx/string_cast.hpp>


using boost::asio::ip::tcp;
using namespace std;


class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_context& io_context)
	{
		return pointer(new tcp_connection(io_context));
	}

	tcp::socket& getSocket()
	{
		return socket_;
	}

private:
	tcp_connection(boost::asio::io_context& io_context)
		: socket_(io_context)
	{
	}

	void handle_write(const boost::system::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{
	}

	tcp::socket socket_;
	std::string message_;
};


class Client {
public:
	Camera* camera;
	Transform* playerT;

	typedef boost::shared_ptr<tcp_connection> tcp_connection_ptr;
	tcp_connection_ptr connection;

	void callServer(Event& e);
	void processRead(
		boost::system::error_code error,
		size_t bytes_read
	);

	Client(boost::asio::io_context& ioContext);

	void start_client() {
		do_read();
	}

	void do_read();
	void handle_read(boost::system::error_code error, size_t bytes_read);

private:
	boost::asio::streambuf buf;
	boost::asio::io_context& io_context_;
};