#pragma once

#include "Types.h"
#include "SslCtx.h"
#include "SslObject.h"
#include <string>
#include <mutex>
#include <queue>

typedef struct Mail
{
	std::string	emailTo;
	std::string	subject;
	std::string	message;
} Mail;

class SMTPManager
{
public:
	SMTPManager(std::string emailFrom);
	~SMTPManager();
	void					PushMail(std::shared_ptr<Mail>& mail);
	std::shared_ptr<Mail>	PopMail();

	SMTPConnectionRef	GetConnection();
private:
	SslCtx		_ctx;
	std::string	_DNSAddress;
	std::string	_emailFrom;
	std::string	_STMPServer;
	int			_serverPort;

	std::mutex				_m;
	std::queue<std::shared_ptr<Mail>>		_mailQueue;
};

class SMTPConnection
{
enum { READ_SIZE = 0x800 };
public:
	SMTPConnection(SSL_CTX* ctx, int fd, std::string DNSAddress,
			std::string emailFrom, std::string STMPServer);
	~SMTPConnection();

	void	Ehlo();
	void	AuthLogin();
	void	SendMail(const std::string& emailTo, const std::string& subject, const std::string& message);
	void	Quit();
private:
	BYTE		_readBuffer[READ_SIZE + 1];
	std::string	_writeBuffer;

	std::string	_DNSAddress;
	std::string	_emailFrom;
	std::string	_STMPServer;

	int			_socket;
	SslObject	_ssl;
};