#include <iostream>
#include "Server.h"

using namespace std;

int main()
{
    Server server(8080);

    server.start();

    cout << "Hello World" << endl;

    return 0;
}
