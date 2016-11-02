#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <experimental/filesystem>

using namespace std::experimental::filesystem;


/*
 * In this example we will list the directories again but
 * this time we will limit the number of tasks which are
 * spawn during the execution. In concurency4 we were
 * spawning each task for each subdirectory and again for any within it.
 * That will flood the processor. We would like to limit it i.e. up to 4 threads
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
};


Result listDir (path && dir)
{
    Result result;
    for (directory_iterator it(dir); it != directory_iterator(); ++it)
    {
        if (is_directory(it->path()))
        {
            result.dirs.push_back(it->path());
        }
        else
        {
            result.files.push_back(it->path().filename());
        }
    }

    return result; //RVO;
}

int main(int argc, char *argv[])
{

    std::string root("/home/jpola/Projects");
    std::vector<path> dirsToDo;
    dirsToDo.push_back(root);

    //accumulator for list of files;
    std::vector<std::string> files;

    //we don't want to create unbounded number of tasks
    while(!dirsToDo.empty())
    {
        std::vector<std::future<Result>> futures;

        //limit the number of tasks to 4
        for (int i = 0; i < 4 && !dirsToDo.empty(); ++i)
        {
            /* We are poping down the directories to do from back to beginning
            *  pop_back Removes last element.
            *  This is a typical stack operation. It shrinks the %vector by one.
            *
            *  Firstly we have moved the last element
            *  so later we have to remove it from the list (pop_back)
            */
            auto ftr = std::async(&listDir, std::move(dirsToDo.back()));
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
                Result result = ftr.get(); //Get the result;
                //back inserter will do push backs - which means it will add the elements to the end of the vector
                std::copy(result.files.begin(), result.files.end(), std::back_inserter(files));
                //add (sub)directories to be parsed
                std::copy(result.dirs.begin(), result.dirs.end(), std::back_inserter(dirsToDo));

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

    std::for_each(files.begin(), files.end(), [](std::string & s)
    {
        std::cout << s << "\n";

    });

}
