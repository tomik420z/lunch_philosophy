#include <iostream>
#include <vector>
#include <future>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "custom_mutex.h"
#include <mutex>
#include <algorithm>
#include <atomic>
#include <windows.h>
#include <condition_variable>


class condition_wait {
public:
    explicit condition_wait(volatile bool& l_fork, volatile bool& r_fork) : l_fork(l_fork), r_fork(r_fork) {}
    ~condition_wait() = default;

    bool operator()() const volatile {
        return l_fork && r_fork;
    }
private:
    volatile bool &l_fork;
    volatile bool &r_fork;

};

class philosophers_having_lunch {
public:
    philosophers_having_lunch(std::vector<std::string>&& _vec_names) 
        : vec_names(std::move(_vec_names)), 
          m_forks(),
          m_cv(),
          h(GetStdHandle(STD_OUTPUT_HANDLE))  {

        m_forks.reserve(vec_names.size());
        
        for(size_t i = 0; i < vec_names.size(); ++i) {
            m_forks.emplace_back(std::make_unique<std::atomic_bool>(true));
        }

        for(size_t i = 0; i < vec_names.size(); ++i) {
            vec_philosophers.emplace_back(
                std::async(philosophers_having_lunch::exec, this, i));
        }

        std::for_each(std::begin(vec_philosophers), std::end(vec_philosophers), [](const auto& philosopher) {
                                                                                        philosopher.wait();
                                                                                    });
        
    } 

    ~philosophers_having_lunch() = default;
private:

    void exec(int _i_th) {
        using namespace std::chrono_literals;
        using namespace std::chrono;

        for(size_t i = 0; i < 15; ++i) {
            take_possession_fork(_i_th);
            
            // поел - немного отдохни 
            int rand_millisecond = rand() % 20;
            std::this_thread::sleep_for(10ms + milliseconds(rand_millisecond));
        }
        m_locker.lock();
        SetConsoleTextAttribute(h, 06);
        std::cout << vec_names[_i_th]  << " left the table" << std::endl;
        SetConsoleTextAttribute(h, 07);
        m_locker.unlock();
    }

    void eating(const std::string& _name) {
        using namespace std::chrono_literals;

        m_locker.lock();
        SetConsoleTextAttribute(h, 02);
        std::cout << _name << " eating..." << std::endl;
        SetConsoleTextAttribute(h, 07);
        m_locker.unlock();

        std::this_thread::sleep_for(2'000ms);
        
        m_locker.lock();
        SetConsoleTextAttribute(h, 04);
        std::cout << _name << " stop eating" << std::endl;
        SetConsoleTextAttribute(h, 07);
        m_locker.unlock();
    }

    /// @brief получить номера вилок по номеру потока
    /// @param _idx - номер потока
    /// @return - индекс левой и правой вилки 
    std::pair<size_t, size_t> get_index_fork(size_t _idx) {
        return std::make_pair(_idx % m_forks.size(),  
                                        (_idx + 1) % m_forks.size());
    }
    /// @brief - процесс ожидания освобождения вилок 
    /// @param _idx  - номер  потока 
    void take_possession_fork(size_t _idx) {
        
        auto [left_ind, right_ind] = get_index_fork(_idx);

        auto & left_fork = *m_forks[left_ind];
        auto & right_fork = *m_forks[right_ind];

        std::unique_lock<std::mutex> lk(m_locker);
        m_cv.wait(lk, [&left_fork, &right_fork]() {
                        return left_fork && right_fork;
                     });
        lk.unlock();
        
        /// левая и правая вилка теперь занята 
        right_fork = false;
        left_fork = false;
        m_cv.notify_all();
    
        eating(vec_names[_idx]);

        right_fork = true;
        left_fork = true;
        m_cv.notify_all();
    }  

private:      
    std::vector<std::future<void>> vec_philosophers;
    
    std::vector<std::string> vec_names; 
    
    /// @brief условная переменная ожидания сигналов 
    std::condition_variable m_cv;
    /// @brief вилки на столе философов, true - если i-aя вилка свободна, false - иначе 
    std::vector<std::unique_ptr<std::atomic_bool>> m_forks;
    /// @brief мютекс для синхонизации печати и условной переменной 
    std::mutex m_locker;
    /// @brief 
    HANDLE h;
};

std::vector<std::string> init_names(size_t _n) {
    std::vector<std::string> vec_names;
    vec_names.reserve(_n);
    for(size_t i = 0; i < _n; ++i) {
        vec_names.emplace_back(std::string("guy_") + std::to_string(i + 1));
    }
    return vec_names;
} 



int main() {  
    std::srand(time(NULL)); 
    int count_guys; 
    std::cout << "Enter the number of philosophers: ";
    std::cin >> count_guys;
    philosophers_having_lunch alg(init_names(count_guys));
    
    return 0;
}