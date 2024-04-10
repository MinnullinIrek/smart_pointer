#include <atomic>
struct CounterBlock
{
  std::atomic<int> cnt = 0;
  std::atomic<int> wnt = 0;
};

template <typename T>
class wptr;

template <typename T>
class sptr
{
  friend class wptr<T>;
public:
  sptr(T* a = nullptr)  {
    init(a);
  }

  sptr(const sptr<T>& s) {
    release();
    val = s.val;
    cnt = s.cnt;
    cnt->cnt.fetch_add(1);
  }

  sptr(sptr<T>&& s) noexcept{
    cnt = s.cnt;
    //cnt->cnt.fetch_add(1);
    val = s.val;
    s.cnt = nullptr;
    s.val = nullptr;
  }

  ~sptr() {
    release();
  }
  
  T* operator *() {
    return val;
  }

  T* operator -> () {
    return val;
  }

  sptr<T> operator = (const sptr<T>& s) {
    
    release();
    cnt = s.cnt;
    val = s.val;

    return this;
  }

  T* get() {
    return val;
  }

  operator bool() {
    return val != nullptr;
  }

  int count() const {
    return cnt->cnt;
  }

  void reset(T* v = nullptr) {
    release();
    cnt = nullptr;
    val = nullptr;
    init(v);
  }



private:
  void release() {
    if (cnt) {
      cnt->cnt.fetch_add(-1);
      if (cnt->cnt == 0) {
        if (val != nullptr) {
          delete val;
          val = nullptr;
          if (cnt->wnt == 0) {
            delete cnt;
          }
        }
      }
      
    }

    
  }

  void init(T* v) {
    if (v) {
      val = v;
      cnt = new CounterBlock;
      cnt->cnt = 1;
    }
  }


private:

  CounterBlock* cnt;
  T* val = nullptr;
};

template <typename T>
class wptr
{
public:
  wptr(const sptr<T>& s) {
    cnt = s.cnt;
    val = s.val;

    cnt->wnt.fetch_add(1);
  }

  sptr<T> lock() {
    sptr<T> s;
    s.cnt = cnt;
    s.val = val;
    if(val)
      s.cnt->cnt.fetch_add(1);
    return s;
  }

private:
  CounterBlock* cnt;
  T* val = nullptr;
};
