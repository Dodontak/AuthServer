#include "Utils.h"
#include <iomanip>
#include <vector>
#include <string>
#include <openssl/rand.h>
#include <jwt-cpp/jwt.h>

using namespace std;

void handle_error(const char* err_str, int rtn)
{
	cerr << err_str << endl;
	exit(rtn);
}

void SocketUtil::MakeSocketNonblock(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
		handle_error("fcntl F_GETFL error", 1);
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		handle_error("fcntl NONBLOCK error", 1);
}

int	SocketUtil::CreateSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

void	SocketUtil::CloseSocket(int socket)
{
	close(socket);
}

string	GetTempId(int len)
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

string CreateAccessToken(const string& user_id, const string& nickname)
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

bool VerifyAccessToken(const string& token, string& out_user_id)
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