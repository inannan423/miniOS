#include <iostream>
#include "os.h"
#include <thread>

using namespace std;

int main() {
    os _os;
    _os.initTemps();
    _os.initFileSystem();
    thread t1(&os::run, std::ref(_os));
    // thread 执行 os 的构造函数
    thread t2(&os::kernel, std::ref(_os));

    t1.join();
    t2.join();
    return 0;
}
