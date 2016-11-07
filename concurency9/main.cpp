#include <thread>
#include <iostream>
#include <string>


class Functor
{
public:
    void operator()()
    {
        std::cout << "Message from functor" << std::endl;
    }
};

class Functor2
{
public:
    void operator()(std::string &msg)
    {
        std::cout << "Message from functor2 " << msg << std::endl;
    }
};

int main(int argc, const char* argv[])
{

    /// Example of C++ most vexine parse:
    /// This by standard is resolved as declaration of function t which returns std::thread
    /// with single unnamed parameter which is a pointer to function returning Functor and it is
    /// taking no input
    //std::thread t(Functor());


    //solution 1: braces initialization
    std::thread t1{Functor{}};
    t1.join();

    //solution 2: additional ()
    std::thread t2((Functor()));
    t2.join();


    std::string msg("some dummy message");
    std::thread t3 {Functor2{}, std::ref(msg)};
    t3.join();

    ///Usually modern compilers can warn you about this (-Wvexing-parse)
    return 0;
}
