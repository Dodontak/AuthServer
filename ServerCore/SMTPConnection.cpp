#include "SMTPConnection.h"
#include "Utils.h"
#include "unistd.h"
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

string Base64Encode(const string& input)
{
    BIO* bio = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    bio = BIO_push(bio, bmem);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // 줄바꿈 없이
    BIO_write(bio, input.c_str(), (int)input.size());
    BIO_flush(bio);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio, &bptr);

    string result(bptr->data, bptr->length);
    BIO_free_all(bio);
    return result;
}

/*============================================================================*\
|                                                                              |
|                              SMTPManager                                     |
|                                                                              |
\*============================================================================*/

SMTPManager::SMTPManager() : _ctx(false), _DNSAddress("google.co.kr"),
	_STMPServer("smtp.gmail.com"), _serverPort(587)
{
}

SMTPManager::~SMTPManager() {}

void	SMTPManager::Init(string emailFrom)
{
	_emailFrom = emailFrom;
}

SMTPConnectionRef	SMTPManager::GetConnection()
{
	int	serverSock = SocketUtils::CreateSocket();
	if (serverSock < 0)
	{
		cerr << "SMTPManager GerConnection CeateSocket\n";
		return nullptr;
	}
	struct sockaddr_in	mailServerAddr;
	memset(&mailServerAddr, 0, sizeof(struct sockaddr_in));

	hostent*	host = gethostbyname(_STMPServer.c_str());
	if (host == nullptr)
	{
		cerr << "SMTPManager GerConnection gethostbyname error\n";
		SocketUtils::CloseSocket(serverSock);
		return nullptr;
	}
	memcpy(&(mailServerAddr.sin_addr), host->h_addr_list[0], host->h_length);
	mailServerAddr.sin_family = host->h_addrtype;
	mailServerAddr.sin_port = htons(_serverPort);

	int	result = connect(serverSock, (sockaddr*)&mailServerAddr, sizeof(mailServerAddr));
	if (result == -1)
	{
		cerr << "SMTPManager GerConnection connect error\n";
		SocketUtils::CloseSocket(serverSock);
		return nullptr;
	}
	SMTPConnectionRef conn = make_shared<SMTPConnection>(_ctx.GetCtx(), serverSock, _DNSAddress,
		_emailFrom, _STMPServer);
	if (conn == nullptr)
		SocketUtils::CloseSocket(serverSock);
	return conn;
}

bool	SMTPManager::Empty()
{
	return _mailQueue.empty();
}

void	SMTPManager::PushMail(shared_ptr<Mail> mail)
{
	lock_guard<mutex>	lock(_m);
	_mailQueue.push(mail);
}

shared_ptr<Mail>	SMTPManager::PopMail()
{
	lock_guard<mutex>	lock(_m);
	if (_mailQueue.empty())
		return nullptr;
	shared_ptr<Mail>	ret = _mailQueue.front();
	_mailQueue.pop();
	return ret;
}

/*============================================================================*\
|                                                                              |
|                             SMTPConnection                                   |
|                                                                              |
\*============================================================================*/

SMTPConnection::SMTPConnection(SSL_CTX* ctx, int fd, string DNSAddress, string emailFrom, string STMPServer)
	: _ssl(ctx, fd), _socket(fd), _DNSAddress(DNSAddress), _emailFrom(emailFrom), _STMPServer(STMPServer)
{
	_writeBuffer.reserve(READ_SIZE);
}

SMTPConnection::~SMTPConnection()
{
	SocketUtils::CloseSocket(_socket);
}

void	SMTPConnection::SendMail(shared_ptr<Mail> mail)
{
	Ehlo();
	AuthLogin();
	SendMail(mail->emailTo, mail->subject, mail->message);
	Quit();
}

void	SMTPConnection::Ehlo()
{
	size_t	readLen = 0;
	size_t	writeLen = 0;
/*----------------------------------------------------------------------------*\
|                                  ehlo                                        |
\*----------------------------------------------------------------------------*/
	readLen = read(_socket, _readBuffer, READ_SIZE);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "ehlo " + _DNSAddress + "\r\n";
	// cout << _writeBuffer;
	write(_socket, (BYTE*)_writeBuffer.data(), _writeBuffer.length());
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                 START TLS                                    |
\*----------------------------------------------------------------------------*/
	readLen = read(_socket, _readBuffer, READ_SIZE);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "STARTTLS\r\n";
	// cout << _writeBuffer;
	write(_socket, (BYTE*)_writeBuffer.data(), _writeBuffer.length());
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                  TLS ehlo                                    |
\*----------------------------------------------------------------------------*/
	readLen = read(_socket, _readBuffer, READ_SIZE);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_ssl.Connect();
	_writeBuffer = "ehlo " + _DNSAddress + "\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();
}

void	SMTPConnection::AuthLogin()
{
	size_t	readLen = 0;
	size_t	writeLen = 0;
/*----------------------------------------------------------------------------*\
|                                AUTH LOGIN                                    |
\*----------------------------------------------------------------------------*/
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "AUTH LOGIN\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                               encodedEmail                                   |
\*----------------------------------------------------------------------------*/
	string	encodedEmail = Base64Encode(_emailFrom);
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = encodedEmail + "\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                 encodedPw                                    |
\*----------------------------------------------------------------------------*/
	string	encodedPw = Base64Encode(getenv("SMTP_APP_PASSWORD"));
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = encodedPw + "\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();
}

void	SMTPConnection::SendMail(const string& emailTo, const string& subject, const string& message)
{
	size_t	readLen;
	size_t	writeLen;
/*----------------------------------------------------------------------------*\
|                                    mail                                      |
\*----------------------------------------------------------------------------*/
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "mail from:<" + _emailFrom + ">\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                    rcpt                                      |
\*----------------------------------------------------------------------------*/
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "rcpt to:<" + emailTo + ">\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                    data                                      |
\*----------------------------------------------------------------------------*/
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "data\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();

/*----------------------------------------------------------------------------*\
|                                  subject                                     |
\*----------------------------------------------------------------------------*/
	string msgId = to_string(time(nullptr)) + "-" + _emailFrom;
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "To: " + emailTo + "\r\n" +
		"From: " + _emailFrom + "\r\n" + 
		"Subject: " + subject + "\r\n" +
		"Message-ID: " + msgId + "\r\n\r\n" +
		message + "\r\n.\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();
}

void	SMTPConnection::Quit()
{
	size_t	readLen = 0;
	size_t	writeLen = 0;
/*----------------------------------------------------------------------------*\
|                                    quit                                      |
\*----------------------------------------------------------------------------*/
	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
	_writeBuffer = "quit\r\n";
	// cout << _writeBuffer;
	_ssl.Write((BYTE*)_writeBuffer.data(), _writeBuffer.length(), &writeLen);
	_writeBuffer.clear();


	_ssl.Read(_readBuffer, READ_SIZE, &readLen);
	_readBuffer[readLen] = 0;
	// cout << _readBuffer << endl;
}