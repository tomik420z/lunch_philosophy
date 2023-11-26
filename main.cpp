#include <iostream>
#include <vector>
#include <future>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "custom_mutex.h"
#include <mutex>

class philosophers_having_lunch {
public:
    philosophers_having_lunch(std::vector<std::string>&& _vec_names) 
        : vec_names(std::move(_vec_names))  {
        
        vec_forks.reserve(vec_names.size() - 1);

        for(size_t i = 0; i < vec_names.size() - 1; ++i) {
            vec_forks.emplace_back(std::make_unique<custom_mutex>());
        }

        m_finish = clock() + 40'000;

        for(size_t i = 0; i < vec_names.size(); ++i) {
            vec_philosophers.emplace_back(
                std::async(philosophers_having_lunch::exec, this, i));
        }

        

        for(size_t i = 0; i < vec_philosophers.size(); ++i) {
            vec_philosophers[i].wait();
        }
        
    } 

    ~philosophers_having_lunch() = default;
private:

    void exec(int _i_th) {
        clock_t start = clock();
        using namespace std::chrono_literals;
        while(start < m_finish) {
            if (take_possession_fork(_i_th)) {
                std::this_thread::sleep_for(10s);
            } else {
                std::this_thread::sleep_for(10ms);
            }
        }
    }

    void eating(const std::string& _name) {
        using namespace std::chrono_literals;
        mtx_output.lock();
        std::cout << _name << " eating..." << std::endl;
        mtx_output.unlock();

        std::this_thread::sleep_for(5'000ms);
        
        mtx_output.lock();
        std::cout << _name << " stop eating" << std::endl;
        mtx_output.unlock();
    }

    
    std::pair<size_t, size_t> get_index_fork(size_t _idx) {
        return std::make_pair(_idx % vec_forks.size(),  
                                        (_idx + 1) % vec_forks.size());
    }
  
    bool take_possession_fork(size_t _idx) {
        auto [lFork, rFork] = get_index_fork(_idx);
        if (!vec_forks[lFork]->is_locked() && !vec_forks[rFork]->is_locked()) {
            {
                std::mutex mtx;
                std::lock_guard<std::mutex> guard(mtx);
                vec_forks[lFork]->lock();
                vec_forks[rFork]->lock();
            }
            eating(vec_names[_idx]);
            {
                
                std::mutex mtx;
                std::lock_guard<std::mutex> guard(mtx);
                vec_forks[lFork]->unlock();
                vec_forks[rFork]->unlock();
            }
            return true;
        } else {
            return false;
        }
    
    }  

private:  
    clock_t m_finish;
    std::vector<std::unique_ptr<custom_mutex>> vec_forks;
    std::vector<std::future<void>> vec_philosophers;
    std::vector<std::string> vec_names; 

    
    std::mutex mtx_output;
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
    int count_guys; 
    std::cout << "Enter the number of philosophers: ";
    std::cin >> count_guys;
    philosophers_having_lunch alg(init_names(count_guys));

    return 0;
}