#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>

/// In this example we presents the nice use of layzy initialization
/// Provide mechanizm for calling some functions only ONCE!! <---- GREAT!!!

class LogFile
{
    std::mutex _mu;
    std::once_flag _flag; // Inditactor if the function was called!
    std::ofstream _f;
public:
    LogFile()
    {
        // we don't need to open file if program does not write any log!
        /// _f.open("log.txt");
    }

    void shared_print (std::string id, int value)
    {
        // Naiive solution is following - sync threads, then check the file if was not opened.
        // but we lose the performance because every time the mu_open mutex is locked
        // to check if the file was not already opened!!!
        // without synchronization the file could be opened many times
        /*
        {
            std::unique_lock<std::mutex> locker2 (_mu_open2);
            if (!_f.open())
            {
            _f.open("log.txt");
            }
        }
        */


        std::call_once(_flag, [&]() { _f.open("log.txt");});
        std::unique_lock<std::mutex> locker(_mu, std::defer_lock);
        _f << "From " << id << ": " << value << std::endl;
    }

};

int main (int argc, const char* argv[])
{
	return 0;
}
