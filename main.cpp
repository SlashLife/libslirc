#include <iostream>
#include <thread>

int main() {
    std::thread thr([]{ std::cout << "Yup"; });
    thr.join();
    return 0;
}
