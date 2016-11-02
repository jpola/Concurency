#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <algorithm>

//This header requires to link with stdc++fs // experimental filesystem
#include <experimental/filesystem>
/*
 * This is related to task / thread parallelism. How to deal with many tasks
 */

void example1()
{
    std::cout << "Main thread id : " << std::this_thread::get_id() << std::endl;

    std::vector<std::future<void>> futures;

    for (int i = 0; i < 10; ++i)
    {
        //Once again in my system the prefered option is deferred maybe because Debug.
        //In deffered option the code will be executed in serial mode when wait will be called.
        //If i foreced to execute in async the code is running with different threads
        auto fut = std::async(std::launch::async, []
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << std::this_thread::get_id() << "\n";
        });

        //future does not have copy constructor so we have to move it
        futures.push_back(std::move(fut));
    }

    std::for_each(futures.begin(), futures.end(), [](std::future<void>& fut)
    {
        fut.wait();
    });


    std::cout << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
}


// This is good example of hiding disk I/O latency we will list many directories concurrently
//using namespace boost::filesystem;
typedef std::vector<std::string> string_vector;

string_vector listDirectory(std::string&& dir)
{
    string_vector listing;
    std::string dirStr("\n> ");
    dirStr += dir;
    dirStr += ":\n\t ";
    listing.push_back(dirStr);


    std::vector<std::future<string_vector>> futures;
    for (std::experimental::filesystem::directory_iterator it(dir);
         it != std::experimental::filesystem::directory_iterator(); ++it)
    {
       //if this is a directory
       if (std::experimental::filesystem::is_directory(it->path()))
       {

           auto ftr = std::async(std::launch::async, &listDirectory, it->path());
           futures.push_back(std::move(ftr));
       }
       else
       {
           listing.push_back(it->path().filename());
       }
    }

    std::for_each(futures.begin(), futures.end(), [&listing](std::future<string_vector>& f)
    {
        string_vector lst = f.get();
        // instead of copy we are moving;
        std::copy(std::make_move_iterator(std::begin(lst)),
                  std::make_move_iterator(std::end(lst)),
                  std::back_inserter(listing));
    });

    return listing;
}


void example2()
{

    std::experimental::filesystem::path root("/home/jpola/Projects");

    auto ftr = std::async(std::launch::async, &listDirectory, root);

    try
    {
        string_vector listing = ftr.get();
        std::for_each(listing.begin(), listing.end(), [](std::string & s)
        {
            std::cout << s << " ";

        });
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    catch (...)
    {
        std::cout << "Unknown exception";
    }
}


int main(int argc, char *argv[])
{

    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    std::cout << " -- example 1 -- " << std::endl;
    //example1();
    std::cout << " -- example 1 end\n -- " << std::endl;

    std::cout << " -- example 2 -- " << std::endl;
    example2();
    std::cout << " -- example 2 end\n -- " << std::endl;
}
