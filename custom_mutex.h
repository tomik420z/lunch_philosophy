#include <mutex>

//мьютекс с дополнительной инофримацией о блокировании 
class custom_mutex {
public:
    custom_mutex(const custom_mutex&) = delete;
    custom_mutex& operator=(const custom_mutex&) = delete;
    custom_mutex(custom_mutex&&) = delete;
    custom_mutex& operator=(custom_mutex&&) = delete;

    custom_mutex() : m_mtx(), m_is_locked(false) {}
    
    void lock() {
        m_is_locked = true;
        m_mtx.lock(); 
    }

    void unlock() {
        m_is_locked = false;
        m_mtx.unlock();
    }

    bool try_lock() {
        m_is_locked =  m_mtx.try_lock();
        return m_is_locked;
    } 

    bool is_locked() const noexcept { 
        return m_is_locked; 
    }  

    ~custom_mutex() {} 
private:
    std::mutex m_mtx;
    bool  m_is_locked;
};
