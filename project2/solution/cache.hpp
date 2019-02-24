#ifndef quantlib_cache_hpp
#define quantlib_cache_hpp

#include <ql/functional.hpp>
#include<tr1/unordered_map>

namespace QuantLib {

    template <typename Key, typename Value>
    class Cache {
      public:
        Cache() {}
        explicit Cache(const QuantLib::ext::function<Value(Key)> f_) { f = f_;}
        ~Cache(){ clear();}

        Value operator()(Key key) const {
            typename std::tr1::unordered_map<Key, Value>::iterator it = memory_map.find(key);
            if (it != memory_map.end())
                return it->second;
            else
                return put(key);
        }

        void setf(const QuantLib::ext::function<Value(Key)> f_) { f = f_; }

        void clear() { memory_map.clear(); }
        void erase(Key key) { memory_map.erase(key); }

      private:
        QuantLib::ext::function<Value(Key)> f;
        mutable std::tr1::unordered_map<Key, Value> memory_map;

        Value put(Key key) const {
            Value value = f(key);
            memory_map[key] = value;
            return value;
        }
    };

}

#endif
