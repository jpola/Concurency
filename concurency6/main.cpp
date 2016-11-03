#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <experimental/filesystem>

using namespace std::experimental::filesystem;


/*
 * In this example we will play with the directory listing.
 * Here we will see how to use locks to avoid races and implement
 * the Monitor pattern.
 */

/* The thread is the owner of the data if no other thread have an access to it at certain time.
 * Transfer of ownership is usualy achieved by move semantics.
 * In case of exclusive ownership there is no need for synchronization.
 *
 * What if we have shared data among many threads. We have build up a mechanizm
 * to avoid races (many threads accessing the same data at the same time).
 * We have to build synchronization mechanizm
 */


struct Result
{
    std::vector<std::string> files;
    std::vector<path> dirs;

    Result () {}
    //Create move semantics to be able to RVO
    //but since we defined move ctor we have to cvreate explicit default ctor
    Result (Result && r): files(std::move(r.files)), dirs(std::move(r.dirs))
    {

    }

    Result & operator=(Result&& r)
    {
        files = std::move(r.files);
        dirs = std::move(r.dirs);

        return *this;
    }
};


//create monitor pattern for result data;
/// Monitor pattern should aquire locks in each public functions
/// It makes it thread-safe
class MonitorResult
{
    Result _result;
    std::mutex _mutex; //used for synchronization
public:

    bool isDirsEmpty()
    {
        std::lock_guard<std::mutex> lck(_mutex);
        return _result.dirs.empty();
    }

    //encapsulation to access the files vector;
    void putFile(std::string&& file)
    {
        ///This piece of code is not very secure.
        /// If we have an exception from push_back the mutex will
        /// not be unlocked due to interuption.
//        _mutex.lock();
//        _result.files.push_back(file);
//        _mutex.unlock();

        ///The solution is to use lock_guard.
        /// It will release mutex when the lock_guard desctuctor is called!
        /// In this case when code is interrupted when stack will be unvinded _mutex will be released;

        //This is RAII object. Resource Aquisition Is Initialisation.
        std::lock_guard<std::mutex> lck(_mutex);
        _result.files.push_back(file);
    }

    void putDir(path&& pth)
    {
        std::lock_guard<std::mutex> lck(_mutex);
        _result.dirs.push_back(pth);

    }

    std::vector<path> getDirs(int n)
    {
        std::vector<path> dirs;
        std::lock_guard<std::mutex> lck(_mutex);
        for (int i = 0; i < n && !_result.dirs.empty(); ++i)
        {
            dirs.push_back(std::move(_result.dirs.back()));
            _result.dirs.pop_back();
        }
        return dirs;
    }

    Result getResult()
    {
        Result res = std::move(_result);
        return res;
    }
};

// Sharing result data
void listDir (path && dir, MonitorResult& result)
{

    for (directory_iterator it(dir); it != directory_iterator(); ++it)
    {
        if (is_directory(it->path()))
        {
            auto pth = it->path();
            result.putDir(std::move(pth));
        }
        else
        {
            result.putFile(it->path().filename());
        }
    }

}

void listAllFiles(std::string& root)
{

    //Create shared data
    MonitorResult result;
    result.putDir(path(root));


    //we don't want to create unbounded number of tasks
    while(!result.isDirsEmpty())
    {
        std::vector<path> dirsToDo = result.getDirs(16);

        std::vector<std::future<void>> futures;

        //limit the number of tasks to 16
        while(!dirsToDo.empty())
        {
            //pass the result (shared data) to async function
            auto ftr = std::async(std::launch::async, &listDir, std::move(dirsToDo.back()), std::ref(result));
            dirsToDo.pop_back();
            futures.push_back(std::move(ftr));
        }

        try
        {
            //here we are creating barrier
            while(!futures.empty())
            {
                auto ftr = std::move(futures.back());
                futures.pop_back();
                ftr.wait(); //previously it was ftr.get() to retrieve data from async now we just have to sync
            }
        }
        catch (std::system_error& e)
        {
            std::cout << "System error: " << e.code().message() << std::endl;
        }

        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
        }

        catch (...)
        {
            std::cout <<"Unknown Exception" << std::endl;
        }
    }

    auto r = result.getResult();
    std::for_each(r.files.begin(), r.files.end(), [](std::string & s)
    {
        std::cout << s << "\n";

    });

}

int main(int argc, char *argv[])
{
    std::string root("/home/jpola/Projects/Concurency");

    auto startTime = std::chrono::system_clock::now();

    //for (int i = 0; i < 25; i++)
        listAllFiles(root);

    auto endTime = std::chrono::system_clock::now();

    auto duration = (endTime - startTime)/25;
    auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    std::cout << "\nSearch performed in " << durationMs.count() << std::endl;




}
