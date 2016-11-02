#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <cassert>


int example1()
{
    //create vector of threads
    std::vector<std::thread> workers;

    for (int i = 0; i < 10; ++i)
    {
        //std::thread require a function in constructor to be executed
        //here we just passing it inline as lambda function

        //Threads begin execution immediately upon construction of the associated thread object (pending any OS scheduling delays)
        workers.push_back(std::thread( []()
        {
            std::cout << "Hi from thread!" << std::endl;
        }
        ));
    }

    std::cout << "Hello from example 1" << std::endl;

    // for each worker call a function;
    // here we again using lambda which takes reference to thread as argument.
    // for each element of workers we are joining them to main thread.
    // the function which is executed have to take the element by ref if we want to modify the element!
    // If that is not present we would have terminate "called without an active exception" error;
    std::for_each(workers.begin(), workers.end(), [](std::thread& th)
    {
        th.join(); //Important! otherwise error
    });
    return 0;
}


int example2()
{
    //create vector of threads
    std::vector<std::thread> workers;

    for (int i = 0; i < 10; ++i)
    {
        //std::thread require a function in constructor to be executed
        //here we just passing it inline as lambda function

        //here in addition we are letting the lambda to capture the external value
        //it bacome an closure - lambda which is accessing the external variables

        // if we put [i] this lambda captures that variable by value!
        // we may pass it by ref!


        workers.push_back(std::thread( [i]()
        {
            // if we have << << there are separate printouts. I would rather create that in one out
            std::cout << "Hi from thread " << i << "!" << std::endl;

        }
        ));
    }

    std::cout << "Hello from example 2" << std::endl;

    // for each worker call a function;
    // here we again using lambda which takes reference to thread as argument.
    // for each element of workers we are joining them to main thread.
    // the function which is executed have to take the element by ref if we want to modify the element!
    // If that is not present we would have terminate "called without an active exception" error;
    std::for_each(workers.begin(), workers.end(), [](std::thread& th)
    {
        th.join();
    });
    return 0;
}

//IMPORTANT: Don’t pass addresses of variables from local stack to thread’s callback function.
// If we pass i by ref there will be an error because value under address of i can change
// i.e. two threads can read the same value.
void thFunc(int& i )
{
    std::cout << "Hi from thread " << i << "!" << std::endl;
}

int example3()
{
    // We cand call move on threads

    std::vector<std::thread> workers;

    for (int i = 0; i < 10; ++i)
    {
        //we are passing function. and give an argument (at the bottom there is variadic template)
        auto th = std::thread(thFunc, std::ref(i));

        //the th already started. but we want to manage it from workers vector
        workers.push_back(std::move(th));

        //There is nice function emplace_back to create object in proper place of the vector. This will avoid using copy / move
        //workers.emplace_back(std::thread(thFunc, i));

        /*
         *  Checks if the thread object identifies an active thread of execution.
         *  Specifically, returns true if get_id() != std::thread::id(). So a default constructed thread is not joinable.
         *  A thread that has finished executing code, but has not yet been joined is still considered an active thread of execution and is therefore joinable.
         */
        assert(!th.joinable());
    }

    std::cout << "Hello from example 3" << std::endl;

    for (auto& t : workers) {
        t.join();
    }

    return 0;
}

int main(int argc, char *argv[])
{
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    std::cout << " -- example 1 -- " << std::endl;
    example1();
    std::cout << " -- example 1 end -- " << std::endl;


    std::cout << " -- example 2 -- " << std::endl;
    example2();
    std::cout << " -- example 2 end -- " << std::endl;

    std::cout << " -- example 3 -- " << std::endl;
    example3();
    std::cout << " -- example 3 end -- " << std::endl;


}
