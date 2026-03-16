#include <libpq-fe.h>

#include <iostream>
#include <string>
#include <thread>

#include "CoreTLS.h"
#include "DBConnectionPool.h"
#include "SMTPConnection.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "Service.h"
#include "ThreadManager.h"
#include "Types.h"

using namespace std;

void SignUp(SessionRef session) {
    Protocol::C_SIGNUP pkt;
    string email, nickname, password;
    email = Utils::GetRandomStr(10) + '@' + Utils::GetRandomStr(3) + ".com";
    pkt.set_email(email);
    nickname = Utils::GetRandomStr(10);
    pkt.set_nickname(nickname);
    password = Utils::GetRandomStr(20);
    pkt.set_password(password);
    pkt.set_skip_email(true);
    session->Send(ServerPacketHandler::MakeWriteBuffer(pkt));
}
// this_thread::sleep_for(chrono::seconds(1));

void SignUpThread(ClientServiceRef service) {
    this_thread::sleep_for(chrono::seconds(1));
    cout << "Start SignUpThread" << endl;
    for (int i = 0; i < 10; i++) {  //랜덤하게 10개 세션 골라서 SignUp시킴
        SessionRef session = service->GetRandomSession();
        if (session) SignUp(session);
    }
}

void WorkerThread() {
    while (true) {
        JobRef job = nullptr;
        {
            unique_lock<mutex> lock(GThreadManager->_workerMutex);
            GThreadManager->_workerCv.wait(
                lock, []() { return !GJobQueue->Empty(); });
            job = GJobQueue->PopJob();
        }
        if (job) job->Execute();
    }
}

int main(int ac, char** av) {
    if (ac != 2) {
        cout << av[0] << " [port]" << endl;
        return 1;
    }
    ServerPacketHandler::Init();

    for (int i = 0; i < 10; i++) GThreadManager->Launch(WorkerThread);
    ClientServiceRef service = make_shared<ClientService>(
        NetAddress("127.0.0.1"/*서버의ip*/, atoi(av[1])),
        [](int clientSocket, sockaddr_in addr, ServiceRef service) {
            return make_shared<ServerSession>(clientSocket, addr, service);
        },
        5);
    GThreadManager->Launch([service]() { SignUpThread(service); });
    service->Start();

    GThreadManager->Join();
}
