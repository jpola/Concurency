#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <future>
// Future and promise revisited: What if we brake our promise to deliver a value?

/// Future - class which is retriving data from the shared location
/// Promise - class which is setting the value from shared location

// Broadcast a value by shared_future - object which is only retriving a value from shared location

int factorial (std::shared_future<int> f)
{
    int result = 1;

    //with the shared_future we can call get() method many times
    int N = f.get(); // if promise.set_value was not called we will wait forever or the exception std::future_errc::broken_promise will be thrown;
    for (int i = N; i > 1; --i)
    {
        result *= i;
    }

    std::cout << "Result is: " << result << std::endl;
    return result;
}


int main (int argc, const char* argv[])
{

    std::promise<int> p;
    std::future<int> f =p.get_future(); //share location
    std::shared_future<int> sf = f.share(); //make shared future;

    // we want to run many function basing on the same value;
    std::future<int> f1 = std::async(std::launch::async, factorial, sf);
    std::future<int> f2 = std::async(std::launch::async, factorial, sf);
    std::future<int> f3 = std::async(std::launch::async, factorial, sf);
    std::future<int> f4 = std::async(std::launch::async, factorial, sf);
    // ...

    // do something else ...
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    //set the promissed value
    p.set_value(4);


    std::cout << "Result from async 1: " << f1.get() << std::endl;
    std::cout << "Result from async 2: " << f2.get() << std::endl;
    std::cout << "Result from async 3: " << f3.get() << std::endl;
    std::cout << "Result from async 4: " << f4.get() << std::endl;

	return 0;
}
