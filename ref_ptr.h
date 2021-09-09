#ifndef SELF_REF_PTR_H
#define SELF_REF_PTR_H
#include <memory>
#include <mutex>
#include <functional>

#include <type_traits>

/**
 *  @brief  a simple smart pointer that can handle array buffer
 */
namespace ref {


template <typename T>
struct type_identity {using type = T;};
template <typename T>
using type_identity_t = typename type_identity<T>::type;


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
        _ref_cnt& operator=(const _ref_cnt& lhs) = delete;

        int add_ref();
        int dec_ref();

        int get_cnt() { return cnt;}
        T&  get_ref(int i=0) {return *(p+i);}
        T*  get_ptr(int i=0) const { return p+i; }
        void release() { deleter(p); }
        void set_ptr(T *p_) { p = p_; }
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

        // copy operations
        ref_ptr(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
        }
        ref_ptr& operator=(const ref_ptr& lhs) {
            if (this != &lhs) {
                reset();
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
            return *this;
        }

        // move operations
        ref_ptr(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                pcnt->add_ref();
                rhs.reset();
            }
        }
        ref_ptr& operator=(ref_ptr&& rhs) {
            if (this != &rhs) {
                reset();
                pcnt = rhs.pcnt;
                pcnt->add_ref();
                rhs.reset();
            }
            return *this;
        }
        ~ref_ptr() {
            if (pcnt) {
                if (!pcnt->dec_ref())
                    delete pcnt;
            }
        }

        explicit operator bool() { return get(); }
        T*  operator ->() {return pcnt->get_ptr(); }
        T*  get() { if(pcnt) return pcnt->get_ptr(); return nullptr;}
        int use_count() {return pcnt->get_cnt();}
        void reset() { // to empty
            if (pcnt) {
                if (!pcnt->dec_ref())
                    delete pcnt;
                pcnt = nullptr;
            }
        }

        template<typename U>
        void reset(U* p) {
            if (pcnt) {
                if (!pcnt->dec_ref())
                    delete pcnt;
                pcnt = new _ref_cnt<U, type_identity_t< std::function<void(U*)> >>(p);
            }
        }

    };

    // friend functions
    template <typename T>
    T& operator *(const ref_ptr<T>& val) {
        return val.pcnt->get_ref();
    }


    /**
     *  @brief  specialized ref_ptr for array objects
     *  @NOTE:  there will be a memory leak if this is called like:
     *  bad \code
     *  ref_ptr<char[]> ptr(new char);
     *  \endcode
     *  good \code
     *  ref_ptr<char[]> ptr(new char[1]);
     *  \endcode
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
        /// \fixme: this would cause mismatched free just like std::shared_ptr
        ref_ptr (T *p, std::function<void(T*)> deleter=[](T *p){delete []p;}) :
                pcnt(new _ref_cnt<T, std::function<void(T*)>>(p, deleter)) { }

        // copy assignment
        ref_ptr(const ref_ptr& lhs) {
            if (this != &lhs) {
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
        }
        ref_ptr& operator=(const ref_ptr& lhs) {
            if (this != &lhs) {
                reset();
                pcnt = lhs.pcnt;
                pcnt->add_ref();
            }
            return *this;
        }

        // move assignment
        ref_ptr(ref_ptr&& rhs) {
            if (this != &rhs) {
                pcnt = rhs.pcnt;
                pcnt->add_ref();
                rhs.reset();
            }
        }
        ref_ptr& operator=(ref_ptr&& rhs) {
            if (this != &rhs) {
                reset();
                pcnt = rhs.pcnt;
                pcnt->add_ref();
                rhs.reset();
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
        explicit operator bool() { return get(); }
        T&  operator [](int i) { return pcnt->get_ref(i); }
        T*  get(int i=0) { if (pcnt) return pcnt->get_ptr(i);  return nullptr; }
        int use_count() {return pcnt->get_cnt();}
        void reset() { // to empty
            if (pcnt) {
                if (!pcnt->dec_ref())
                    delete pcnt;
                pcnt = nullptr;
            }
        }

        /// \tparam U shall be implicit convertable to T (ref_ptr's tparam)
        template<typename U>
        void reset(U* p) {
            if (pcnt) {
                if (!pcnt->dec_ref())
                    delete pcnt;
                pcnt = new _ref_cnt<U, type_identity_t<std::function<void(U*)> >>(p, [](U* p){delete[]p; });
            }
        }

    };
    /** TODO: @b operator+() */
    // friend functions
    // template <typename T>
    // T& operator *(const ref_ptr<T[]>& val) {
    //     return val.pcnt->get_ref();
    // }

#if __cplusplus > 201700
    template <typename T>
    inline constexpr bool is_array_v = std::is_array<T>::value;
#else
    template <typename T>
    constexpr bool is_array_v = std::is_array<T>::value;
#endif

    /** TODO: like std::make faster than constructors*/
    /// enable if \tparam T is not an array
    template <typename T, typename ...Args>
    std::enable_if_t<!is_array_v<T>, ref_ptr<T>> make_ref(Args... args) {
        return ref_ptr<T>(new T(args...));
    }
    
    
    template <typename T>
    std::enable_if_t< is_array_v<T>, ref_ptr<std::remove_all_extents_t<T>[]> >
    make_ref(size_t n) {
        using U = std::remove_all_extents_t<T>;
        return ref_ptr<U[]> (new U[n]);
    }
} // namespace ref
#endif //SELF_REF_PTR_H