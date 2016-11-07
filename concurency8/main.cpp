#include <iostream>
#include <thread>

// Let's build a thread wrapper class to have scoped execution


class scoped_thread
{
    std::thread _t;

public:

    scoped_thread() = delete ;
    scoped_thread(const scoped_thread  &) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;

    scoped_thread(std::thread t) : _t(std::move(t))
    {
    }

    template<typename _Callable, typename... _Args>
      explicit
      scoped_thread(_Callable&& __f, _Args&&... __args): _t(__f, __args...)
      {}


    void detach()
    {
        _t.detach();
    }

    bool joinable()
    {
        return _t.joinable();
    }

    ~scoped_thread()
    {

        if (_t.joinable())
            _t.join();
    }
};


void sc_thr_func()
{
    std::cout << "This is message from scoped thread\n";
}


void thr_func()
{
    std::cout << "This is message from 'normal' thread function\n";
}

void thr_func_wa(int a)
{
    std::cout << "Thread function with argument " << a << "\n";
}

void thr_func_wref(int& a)
{
    std::cout << "Thread function with ref arg " << a << "\n";

}

int main(int argc, const char* argv[])
{
    {
        scoped_thread t((std::thread(sc_thr_func)));
    }

    {
        std::thread th(thr_func);
        scoped_thread t(std::move(th));

    }
    {
        scoped_thread t(sc_thr_func);
    }

    {
        scoped_thread t(thr_func_wa, 10);
    }

    {
        int x = 3;
        scoped_thread t(thr_func_wref, std::ref(x));
    }

    return 0;
}
