#include <iostream>
#include <string>
#include "Service.h"
#include "Types.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include <thread>

using namespace std;

int main(int ac, char** av)
{
	if (ac != 2)
		handle_error((string(av[0]) + " [port]").c_str(), 1);

	for (int i = 0; i < 5; i++)
	{
		GThreadManager->Launch();
	}
	ServiceRef	service = make_shared<Service>(
		"127.0.0.1",
		stoi(av[1]),
		"server.crt",
		"server.key"
	);
	service->Start();
	GThreadManager->Join();
}
