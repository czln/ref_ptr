#ifndef SELF_REF_PTR_H
#define SELF_REF_PTR_H
#include <memory>
#include <mutex>
#include <functional>

/**
 *  @brief  a simple smart pointer that can handle array buffer
 *  @bug    valgrind report: error calling PR_SET_PTRACER, vgdb might block
 *          env: wsl
 */
namespace ref {


    /**
     *  @brief  reference counter
     *  NOTE:   thread safe is not guaranteed! T *p maybe changed
     *          while using it
     */
    template <typename T, typename Deleter = std::function<void(T*)>>
    class _ref_cnt {
    private:
        std::mutex lock;
        Deleter deleter;
        int cnt;
        T *p;
    public:
        _ref_cnt() : cnt(0), p(nullptr), deleter([](T *ptr){
                                                delete ptr;}) { }
        explicit _ref_cnt(T *ptr, 
                            Deleter deleter=[](T *ptr){
                                                delete ptr;}) {
            cnt = 1;
            this->deleter = deleter;
            p = ptr;
        }
        ~_ref_cnt() { deleter(p);}

        // not allowed to copy
        _ref_cnt(const _ref_cnt& lhs) = delete;
        const _ref_cnt& operator=(const _ref_cnt& lhs) = delete;

        int add_ref();
        int dec_ref();
        
        int get_cnt() { return cnt;}
        T&  get_ref(int i=0) {return *(p+i);}
        T*  get_ptr(int i=0) const { return p+i; }
    };
    template <typename T, typename Deleter>
    int _ref_cnt<T, Deleter>::add_ref() {
        // lock.lock();
        int tmp = ++cnt;
        // lock.unlock();
        return tmp;
    }
    template <typename T, typename Deleter>
    int _ref_cnt<T, Deleter>::dec_ref() {
        // lock.lock();
        int tmp = --cnt;
        // lock.unlock();
        return tmp;
    }
    


    /**
     *  @brief  a smart pointer like std::shared_ptr but 
     *          able to handle array objects
     */
    template <typename T, typename Deleter=std::function<void(T*)>>
    class ref_ptr {
    public:
        template <typename fT>
        friend fT& operator *(const ref_ptr<fT>& val);
    private:
        _ref_cnt<T, Deleter> *pcnt;
    public:
        /** constructors
         *  allow convertion from @c T* to @c ref_ptr<T*>
         */
        ref_ptr() : pcnt(nullptr) { }
        ref_ptr(T *p) : pcnt(new _ref_cnt<T>(p)) {}

        // copy assignment
        ref_ptr(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
        }
        const ref_ptr& operator=(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
            return *this;
        }

        // move assignment
        ref_ptr(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                rhs = nullptr;
            }
        }
        const ref_ptr& operator=(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                rhs = nullptr;
            }
            return *this;
        }
        ~ref_ptr() {
            if (pcnt) {
                if (!pcnt->dec_ref()) 
                    delete pcnt;
            }
        }

        T*  operator ->() {return pcnt->get_ptr(); }
        T*  get() { return pcnt->get_ptr(); }
        int use_count() {return pcnt->get_cnt();}
        
    };
    
    // friend functions
    template <typename T>
    T& operator *(const ref_ptr<T>& val) {
        return val.pcnt->get_ref();
    }


    /** 
     *  @brief  specialized ref_ptr for array objects
     *  TODO:   check if it works for class deleter, i.g.
     *          if @p  std::function<void(T*)> deleter needs
     *          to be a template typename
     */
    template <typename T, typename Deleter>
    class ref_ptr<T[], Deleter> {
    public:
        // friend functions
        template <typename fT>
        friend fT& operator *(const ref_ptr<fT>& val);
    private:
        _ref_cnt<T, std::function<void(T*)>> *pcnt;
    public:
        /** constructors
         *  allow convertion from @c T* to @c ref_ptr<T*,Deleter>
         */
        ref_ptr() : pcnt(nullptr) { }
        ref_ptr (T *p, std::function<void(T*)> deleter=[](T *p){delete []p;}) :
            pcnt(new _ref_cnt<T, std::function<void(T*)>>(p, deleter)) {}

        // copy assignment
        ref_ptr(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
        }
        const ref_ptr& operator=(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
            return *this;
        }
        
        // move assignment
        ref_ptr(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                rhs.pcnt = nullptr;
            }
        }
        const ref_ptr& operator=(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                rhs.pcnt = nullptr;
            }
            return *this;
        }
        // deconstructor
        ~ref_ptr() {
            if (pcnt) {
                if (!pcnt->dec_ref()) 
                    delete pcnt;
            }
        }
    public:
        T&  operator [](int i) { return pcnt->get_ref(i); }
        T*  get(int i=0) { return pcnt->get_ptr(i); }
        int use_count() {return pcnt->get_cnt();}

    };
    /** TODO: @b operator+() */
    // friend functions
    // template <typename T>
    // T& operator *(const ref_ptr<T[]>& val) {
    //     return val.pcnt->get_ref();
    // }

    /** TODO: like std::make faster than constructors*/
    template <typename T, typename D = std::function<void(T*)>>
    ref_ptr<T, D> make_ref(T *p, D d=[](T* _p){delete _p;}) {
        return ref_ptr<T, D>(p);
    }
    /** TODO:*/
    // template <typename T, typename D = std::function<void(T*)>>
    // ref_ptr<T[]> make_ref(T *p, D d=[](T* _p){delete[] _p;}) {
    //     return ref_ptr<T, D>(p, d);
    // }

} // namespace ref
#endif //SELF_REF_PTR_H