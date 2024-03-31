#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct SharedData {
  std::mutex mutexLock{};

  std::string output;
};

void Thread1(SharedData *dat) {
  std::lock_guard<std::mutex> guard(dat->mutexLock);
  for (int i = 0; i < 900000; i++) {
    dat->output += std::to_string(i);
  }
}

int main() {
  SharedData dat{};
  dat.output = "";

  std::vector<std::thread> threads;

  for (unsigned int i = 0; i < std::thread::hardware_concurrency() * 7; i++) {
    threads.emplace_back(Thread1, &dat);
  }

  for (auto &thread : threads) {
    thread.join();
  }
}
