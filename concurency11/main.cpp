#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <future>
// Future and promise revisited: What if we brake our promise to deliver a value?

/// Future - class which is retriving data from the shared location
/// Promise - class which is setting the value from shared location

int factorial (std::future<int>& f)
{
    int result = 1;

    int N = f.get(); // if promise.set_value was not called the exception std::future_errc::broken_promise will be thrown;
    for (int i = N; i > 1; --i)
    {
        result *= i;
    }

    std::cout << "Result is: " << result << std::endl;
    return result;
}


int main (int argc, const char* argv[])
{
    int x;

    std::promise<int> p;
    std::future<int> f = p.get_future(); //share the location

    std::future<int> fu = std::async(std::launch::async, factorial, std::ref(f));

    // do something else ...
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    /// we wish to calculate factorial of 4
    ///p.set_value(4);
    ///x = fu.get(); /// If the p.set_value(4) is not called we will wait forever or broken_promise exception will be called;

    // Other solution is when I know that I can't send a value with promise - we are braking the promise
    // we can set the exception as follows;
    p.set_exception(std::make_exception_ptr(std::runtime_error("Error from promise - I can't send you a value")));
    x = fu.get();



    std::cout << "Result from async: " << x << std::endl;

	return 0;
}
