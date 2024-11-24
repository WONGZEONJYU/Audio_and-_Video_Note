#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "xhelper.hpp"

template <typename T>
class Singleton {
    X_DISABLE_COPY_MOVE(Singleton)
protected:
    explicit Singleton() = default;
    virtual ~Singleton() = default;

public:
    using Ptr = std::shared_ptr<T>;

    template <typename ...Args>
    static Ptr instance(Args&&... args){
        if (!sm_ins_){
            static std::mutex mux;
            std::unique_lock locker(mux);
            TRY_CATCH(CHECK_EXC(sm_ins_.reset(new T(std::forward<Args>(args)...))));
        }
        return sm_ins_;
    }

    // 重载函数，无需参数
    static Ptr instance(){
        return sm_ins_;
    }

    static void destroy() {
        sm_ins_.reset();
    }
private:
    static std::shared_ptr<T> sm_ins_;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::sm_ins_{};

#endif
