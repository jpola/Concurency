#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <exception>
#include <stdexcept>


//IMPORTANT: https://youtu.be/o0pCft99K74?list=PL1835A90FC78FF8BE

/*
 * Promise and Future are giving a framework of communication channel between threads
 * the promise is an input of communication channel and future is an output
 * First we are creating promise templetized with a required type. When we are
 * creating promise a shared state is allocated. In this shared state we are storing the value will be stored;
 * from the promise we are creating future. (auto ftr = prms.get_future(); )
 * Future will share the same share state location as promise.
 * Shared state location is ref counted.
 *
 * We can move / pass somehow promise to the another thread in which it will be filled with requested values;
 *
 * Future is about to get the value from the promise: val = ftr.get(); if the promise is not yet set
 * it will wait for it :) ! NICE
 *
 * prms.set_value() - sets the shared state (location) from empty to ready;
 *
 * if shared state is ready we can get it by ftr.get()
 *
 * ftr.get() is invalidating the shared state location because we can take it by value
 * or MOVE IT from there!
 *
 */



void thFunc(std::promise<std::string>&& prms)
{
    std::string str("Hello from future move!");

    //It is very important that this string is passed by move! So we are not copying anything!
    prms.set_value(str);

   std::cout << "ptr addres " << (void *) str.data()<< std::endl;


}


void thFuncRef(std::promise<std::string>& prms)
{
    std::string str("Hello from referenced prms");
    prms.set_value(str);
}

//What if something goes wrong?
void thFuncWithExcept(std::promise<std::string>&& prms)
{
    try
    {
        std::string str("Hello from future with exceptions");
        throw std::runtime_error("Exception from future!");
        prms.set_value(str);
    }
    catch(...)
    {
        //current_exception takes the copy of current exception
        prms.set_exception(std::current_exception());
    }
}

// pass promise by move semantics
void example1()
{
    //Promise is an input
    std::promise<std::string> prms;

    // In this case we have to first create future because we are MOVING the promise to another thread so
    // in main thread it will be invalid;
    std::future<std::string> ftr = prms.get_future();

    std::thread th(&thFunc, std::move(prms));

    std::cout << "Hello from main!" << std::endl;

    std::string str = ftr.get(); //This also sync point. Waiting until thFunc will finish;

    std::cout << str << std::endl;
    std::cout << "ptr is the same addres " << (void *) str.data()<< std::endl;

    th.join(); //Important we have to terminate the spawned thread

}

void example2()
{
    //Promise is an input
    std::promise<std::string> prms;

    //In this case we have more freedom in creation of future because we are passing prms by ref.
    //So we are sharing it among main and new spawned threads;


    std::thread th(&thFuncRef, std::ref(prms));

    std::cout << "Hello from main!" << std::endl;

    // In example we can create it just before of get
    std::future<std::string> ftr = prms.get_future();
    std::string str = ftr.get(); //This also sync point. Waiting until thFunc will finish;

    std::cout << str << std::endl;

    th.join(); //Important we have to terminate the spawned thread

}

// handle exception with future and promise
void example3()
{
    //Promise is an input
    std::promise<std::string> prms;

    // In this case we have to first create future because we are MOVING the promise to another thread so
    // in main thread it will be invalid;
    std::future<std::string> ftr = prms.get_future();

    std::thread th(&thFuncWithExcept, std::move(prms));

    std::cout << "Hello from main!" << std::endl;

    // The exception if is present in promise it will be rethrown here
    try {
        std::string str = ftr.get(); //This also sync point. Waiting until thFunc will finish;
        std::cout << str << std::endl;
        std::cout << "ptr is the same addres " << (void *) str.data()<< std::endl;
    }
    catch(std::exception& e)
    {
       std::cout << e.what() << std::endl;
    }



    th.join(); //Important we have to terminate the spawned thread



}

std::string thFuncRet()
{
    std::string str ("Hello from Async!");
    std::cout << "ptr addres of string in async execution " << (void *) str.data()<< std::endl;
    return str;

}


//We can do it simpler with async. For that we can hide the code related to promise.
// it have exactly the same logic as before but it is simpler.
// Now we have to thing about that as task executed in async manner;
void example4()
{
    auto ftr = std::async(&thFuncRet);

    std::cout << "Hello from main!" << std::endl;
    std::string str = ftr.get();
    std::cout << "ptr addres of string in async execution ret by ftr " << (void *) str.data()<< std::endl;
    std::cout << "Also the value is moved!" << std::endl;
    std::cout << str << std::endl;
}


//Here we present the case when the exception is thrown in async task;
std::string thFuncRetExcept()
{

    std::string str ("Hello from Async exception case!");
    throw std::runtime_error("Exception from async task!");
    return str;

}

void example5()
{
    auto ftr = std::async(&thFuncRetExcept);

    std::cout << "Hello from main!" << std::endl;
    try
    {
        std::string str = ftr.get();
        std::cout << str << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}


//Here is how to handle the future and promise when dealing with void function
void voidFunction()
{
    std::cout << "Starting task." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Ending task" << std::endl;
}
//1. If the async flag is set (i.e. policy & std::launch::async != 0),
//   then async executes the function f on a new thread of execution
//   (with all thread-locals initialized) as if spawned by std::thread(f, args...),
//   except that if the function f returns a value or throws an exception,
//    it is stored in the shared state accessible through the std::future that async returns to the caller.

//2. If the deferred flag is set (i.e. policy & std::launch::deferred != 0),
//   then async converts args... the same way as by std::thread constructor,
//   but does not spawn a new thread of execution. Instead, lazy evaluation is performed:
//   the first call to a non-timed wait function on the std::future that async returned to the caller will cause f(args...)
//   to be executed in the current thread (which does not have to be the thread that originally called std::async).
//   The result or exception is placed in the shared state associated with the future and only then it is made ready.
//   All further accesses to the same std::future will return the result immediately.

//3. If both the std::launch::async and std::launch::deferred flags are set in policy,
//   it is up to the implementation whether to perform asynchronous execution or lazy evaluation.


void example6()
{
    //ONE THING: & in passing function does not matter! it is just for clarification!
    // there are two launch directives aync or deferred
    // Ad 1. run in async
    // std::future<void> ftr = std::async(std::launch::async, &voidFunction);

    // Ad 2. call wait()
    std::future<void> ftr = std::async(std::launch::deferred, &voidFunction);
    ftr.wait();

    // you can't call this: auto x = ftr.get(); because passed function is void
    std::cout << "Exiting form main" << std::endl;

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

    std::cout << " -- example 4 -- " << std::endl;
    example4();
    std::cout << " -- example 4 end -- " << std::endl;

    std::cout << " -- example 5 -- " << std::endl;
    example5();
    std::cout << " -- example 5 end -- " << std::endl;

    std::cout << " -- example 6 -- " << std::endl;
    example6();
    std::cout << " -- example 6 end -- " << std::endl;

    return 0;
}
