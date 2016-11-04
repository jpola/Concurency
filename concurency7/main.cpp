#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <deque>
#include <condition_variable>
#include <experimental/filesystem>

using namespace std::experimental::filesystem;


/*
 * In this example we will play with the directory listing again.
 * Here we will implement Producer - Consumer pattern in multithreaded application. It requires use of condition_variable
 * Basically producer sends "notify" which updates the condition_variable. Consumer's default stat is wait. It is waiting
 * to do the job. The condition_variable can inform the consumer to wake up.
 * In mt apps we have to secure condition_variable to avoid races. We have to secure situation that
 * when condition_variable is changed the consumer will not miss the execution.
 * We will also implement the message queue to store tasks for consumer.
 *
 * see this video:
 * https://youtu.be/309Y-QlIvew?list=PL1835A90FC78FF8BE
 *
 */

/// We will use additonal shared variable to make sure that consumer do the job when notified. We have to use some locking protocol to
/// avoid races.
/// Here we will use the unique_lock instead the guard_lock. The unique_lock will be unlocked by condition_variable, while lock_guard is
/// unlocking the resource in destructor:
/// After StackOverflow:
/// The difference is that you can lock and unlock a std::unique_lock. std::lock_guard will be locked only once on construction and unlocked on destruction.
/// So for usecase B you definitely need a std::unique_lock for the condition variable. In case A it depends whether you need to relock the guard.
/// std::unique_lock has other features that allow it to e.g.: be constructed without locking the mutex immediately but to build the RAII wrapper (see here).
/// Lock guards can be used when you simply need a wrapper for a limited scope, e.g.: a member function

/// lock_guard and unique_lock are pretty much the same thing; lock_guard is a restricted version with a limited interface.
/// A lock_guard always holds a lock from its construction to its destruction. A unique_lock can be created without immediately locking,
/// can unlock at any point in its existence, and can transfer ownership of the lock from one instance to another.
/// So you always use lock_guard, unless you need the capabilities of unique_lock. A condition_variable needs a unique_lock

/// Important: unlock of the unique_lock and enter the wait state is done in one atomic operation!


/// Each thread will be a server. Server thread has a server loop inside. Loop start by waiting for the message queue.
/// Message queue provides items to be executed / processed and then the server is going to wait state again.

// In our case the server will be both a producer of directory paths and consumer of those paths.

constexpr int NUM_THREADS = 10;


template<typename T>
class MessageQueue
{
    std::deque<T> _queue;
    std::condition_variable _cond;
    std::mutex _mutex;

public:

    //Notify the reciever
    void send(T&& message)
    {
        //lock guard is releasing the lock in destr. So in this case in the end of this scope
        {
            std::lock_guard<std::mutex> lck(_mutex);
            _queue.push_front(std::move(message));
        }
        _cond.notify_one();

    }

    T recieve()
    {
        std::unique_lock<std::mutex> lck(_mutex);

        /// below corresponds to following piece of code;
        /// since it is very common the condition_variable
        /// is taking predicate as an argument
        ///
        /*
         * while (!queue.empty())
         * {
         *   cond.wait(lock) //release lock to give a chance for producer to do the changes
         *
         * }
         */

        /// Predicate is provided as lambda
        /// so server will wake up when queue is not empty
        ///
        /// _queue is part of the class so we have to capture pointer to this class
        _cond.wait(lck, [this] { return !_queue.empty();});

        T msg = std::move(_queue.back());
        _queue.pop_back();
        return msg;
    }


};


// queues are shared among threads
void listDirServer (MessageQueue<path> & dirQueue, MessageQueue<std::string>& fileQueue)
{
    for (;;)
    {
        path dir = dirQueue.recieve();
        for (directory_iterator it(dir); it != directory_iterator(); ++it)
        {
            if (is_directory(it->path()))
            {
                path p = std::move(it->path());
                dirQueue.send(std::move(p));
            }
            else
            {
                fileQueue.send(it->path().filename());

            }
        }
    }
}


void printServer (MessageQueue<std::string>& nameQueue)
{
    for(;;)
    {
        std::string name = nameQueue.recieve();
        std::cout << name << std::endl;
    }
}

void listTree (path&& rootDir)
{
    MessageQueue<path> dirQueue;
    MessageQueue<std::string> fileQueue;
    dirQueue.send(std::move(rootDir));

    std::vector<std::future<void>> futures;

    //Spawn #NUM_THREADS servers
    for (int i = 0; i < NUM_THREADS; ++i) {
        futures.push_back(
                    std::async(std::launch::async, &listDirServer,
                               std::ref(dirQueue),
                               std::ref(fileQueue)));
    }

    //Spawn one print server
    futures.push_back(std::async(std::launch::async, &printServer, std::ref(fileQueue)));

    try {
        while(!futures.empty())
        {
            auto ftr = std::move(futures.back());
            futures.pop_back();
            ftr.get();
        }
    } catch (std::exception& e) {

    }
}



int main(int argc, char *argv[])
{
    std::string root("/home/jpola/Projects/Concurency");
    listTree(path(root));


    //TODO: How to shutdown the servers;
    //How to decide that the job is done
    //HINT1: Dir Message queue is empty, however other servers can still produce the subdirectories to be listed so none of
    // the servers are active - implement counter for active servers in any point in time.
    //HINT2: How do you brake from inf loop. Send the server empty string! If empty string is recieved then brake out of the loop

}
