#include <iostream>
#include <algorithm>
#include <list>
#include <cmath>
#include <chrono>
#include <thread>

void display_graph (std::list<double>& lst)
{
    std::for_each(lst.begin(), lst.end(), [](double& x)
    {
        int count = static_cast<int>(10* x + 10.5);
        for (int i = 0; i < count; ++i)
            std::cout.put('*');
        std::cout << std::endl;
    });

}

/* This example presents how to move data to the thread task
 *
 * We can do it in two ways
 * 1. Give an access to memory as shared data
 * 2. Move the data to the thread function
 *
 * In case of 1. we have to make sure that there is no race!
 * In case of 2. we are moving data so the thread have the unique set.
 *
 */


//In this case thread will take the list object by ref
//We are not securing that list will not be accessed by some other threads!
// so there can be races!
void toSinShared (std::list<double>& shared_list)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::for_each(shared_list.begin(), shared_list.end(), [](double& x)
    {
        x = sin(x);
    });
}

void toSinMoved (std::list<double>&& moved_list)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::for_each(moved_list.begin(), moved_list.end(), [](double& x)
    {
        x = sin(x);
    });

    //Because we moved data to the thread we have to print it here;
    // Can we return it by RVO?
    display_graph(moved_list);
}

std::list<double> toSinMovedRes(std::list<double>&& moved_list)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::for_each(moved_list.begin(), moved_list.end(), [](double& x)
    {
        x = sin(x);
    });

    // Can we return it by RVO?

    // for this we have to involve future, promise or async. See concurency3
    // Se this : https://www.youtube.com/watch?v=o0pCft99K74&index=4&list=PL1835A90FC78FF8BE
    return moved_list;
}

//This is ok. Shared memory is accessed in proper way.
// 1. Main thread creates shared memory
// 2. Just 1 thread is modifying the shared memory
// 3. Nothing else is modifying this memory at the same time;
// 4. After joining the main thread is reading.
// Everything is ok.

// If we first read and then join. program will compile but the result will be broken.
void example1()
{
    std::list<double> list;
    //access list from main thread
    const double pi = 3.141592;
    const double epsilon = 0.0000001;

    for (double x = 0.0; x < 2* pi + epsilon; x += pi/16.)
    {
        list.push_back(x) ;
    }

    // this thread will now work on the list
    std::thread th(&toSinShared, std::ref(list));
    // thread fhinishes
    th.join();

    //main thread access the shared data
    display_graph(list);
}

void example2()
{
    std::list<double> list;
    //access list from main thread
    const double pi = 3.141592;
    const double epsilon = 0.0000001;

    for (double x = 0.0; x < 2* pi + epsilon; x += pi/16.)
    {
        list.push_back(x) ;
    }

    std::thread th(&toSinMoved, std::move(list));
    th.join();
}

void example3()
{
    std::list<double> list;
    //access list from main thread
    const double pi = 3.141592;
    const double epsilon = 0.0000001;

    for (double x = 0.0; x < 2* pi + epsilon; x += pi/16.)
    {
        list.push_back(x) ;
    }

    std::thread th(&toSinMoved, std::move(list));
    th.join();

}

int main()
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
