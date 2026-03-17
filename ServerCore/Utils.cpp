#include "Utils.h"
#include <iomanip>
#include <vector>
#include <string>
#include <openssl/rand.h>
#include <jwt-cpp/jwt.h>
#include <sys/socket.h>
#include <regex>

using namespace std;

/*============================================================================*\
|                                                                              |
|                               SocketUtils                                    |
|                                                                              |
\*============================================================================*/

bool SocketUtils::MakeSocketNonblock(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
        return false;
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;
    return true;
}

bool SocketUtils::Bind(int socket, NetAddress addr)
{
    if (0 != bind(socket, (struct sockaddr *)&addr.GetAddr(), sizeof(addr.GetAddr())))
        return false;
    return true;
}

bool SocketUtils::SetReuseAddress(int socket, bool flag)
{
    int optval = (flag ? 1 : 0);
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (unsigned char *)&optval, sizeof(optval)) == -1)
        return false;
	return true;
}

int	SocketUtils::CreateSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

void	SocketUtils::CloseSocket(int socket)
{
	close(socket);
}

/*============================================================================*\
|                                                                              |
|                                   Utils                                      |
|                                                                              |
\*============================================================================*/

void Utils::ErrorExit(const char* err_str)
{
	cerr << err_str << endl;
	exit(1);
}

//len 만큼의 랜덤바이트 배열 만들고, 각 바이트 16진수로 변환. len*2길이로 나옴.
string	Utils::GetRandomStr(int len)
{
	vector<unsigned char>	buffer(len);
	if (RAND_bytes(buffer.data(), buffer.size()) != 1)
	{
		cout << "error" << endl;
	}
	stringstream ss;
    for (unsigned char byte : buffer) {
        ss << hex << setw(2) << setfill('0') << (int)byte;
    }
	return ss.str();
}

string Utils::CreateAccessToken(const string& user_id, const string& nickname)
{
    // JWT 시그니처용 비밀키는 환경변수에 저장해둠.
    const string SECRET_KEY = getenv("JWT_SECRET_KEY");

    auto token = jwt::create()
        .set_issuer("auth_server")          // 발급자
        .set_type("JWT")
        .set_payload_claim("user_id",  jwt::claim(user_id))
        .set_payload_claim("nickname", jwt::claim(nickname))
        .set_issued_at(chrono::system_clock::now())
        .set_expires_at(chrono::system_clock::now() 
                        + chrono::hours(1))    // 1시간
        .sign(jwt::algorithm::hs256{SECRET_KEY});   // 비밀키로 서명

    return token;
}

bool Utils::VerifyAccessToken(const string& token, string& out_user_id)
{
	// JWT 시그니처용 비밀키는 환경변수에 저장해둠.
    const string SECRET_KEY = getenv("JWT_SECRET_KEY");
    
    try
    {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .with_issuer("auth_server");    // 발급자 확인
        
        auto decoded = jwt::decode(token);
        verifier.verify(decoded);           // 서명 + 만료시간 자동 검증
        
        out_user_id = decoded.get_payload_claim("user_id").as_string();
        return true;
    }
    catch (const exception& e)
    {
		cout << "exeption" << endl;
        // 서명 불일치, 만료, 형식 오류 전부 여기로 떨어짐
        return false;
    }
}

bool    Utils::VerifyEmail(const string& email)
{
    if (email.length() > 100)
        return false;
    const regex pattern(R"([a-zA-Z0-9]+@[a-zA-Z0-9]+(\.[a-zA-Z]{2,}){1,2})");
    return regex_match(email, pattern);
}

bool    Utils::VerifyNickname(const string& nickname)
{
    const regex pattern(R"([a-zA-Z0-9]{2,20})");
    return regex_match(nickname, pattern);
}