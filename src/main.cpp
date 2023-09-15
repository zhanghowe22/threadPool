#include "threadPool.h"

using namespace std;

int main() 
{
    ThreadPool& instance = ThreadPool::getInstance(4);

    for(int i = 0; i < 10; i++) {
        instance.enqueue([i] {
            std::cout << "task : " << i << " is running" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "task : " << i << " is done" << std::endl;
        });
    }

    return 0;
}
