#include <iostream>
#include <functional>
#include <signal.h>

void myHandler(int signalId)
{

    std::cout << "The signal ID is: " << signalId << std::endl;
}

void indirection(void (*callMe)(int))
{

    callMe(30);
}

void indirection2(std::function<void(int)> callMe)
{

    callMe(50);
}

int main()
{
    //void (*func)(int) = myHandler;
    //std::function<void(int)> func2 = myHandler;

    //func(10);
    //myHandler(20);
    //indirection(myHandler);
    //func2(40);
    //indirection2(myHandler);

    signal(SIGINT, myHandler);
    signal(SIGKILL, myHandler);
    signal(SIGFPE, myHandler);

    int a = 1;
    int b = 0;

    int r = a / b;

    std::cout << "Enter anything to quit...";
    int input;
    std::cin >> input;

    return 0;
}
