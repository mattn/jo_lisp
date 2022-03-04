// Written by Jon Olick
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#ifndef JO_STDCPP
#define JO_STDCPP

// Not in any way intended to be fastest implementation, just simple bare minimum we need to compile tinyexr

#include <string.h>
//#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <limits.h>

#ifdef _MSC_VER
#include <direct.h>
#define jo_strdup _strdup
#define jo_chdir _chdir
#pragma warning(push)
#pragma warning(disable : 4345)
#else
#define jo_strdup strdup
#define jo_chdir chdir
#endif

#ifdef _MSC_VER
static int jo_setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#else
#define jo_setenv setenv
#endif

static bool jo_file_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if(f) {
        fclose(f);
        return true;
    }
    return false;
}

#ifndef _MSC_VER
#include <dirent.h>
static bool jo_dir_exists(const char *path)
{
    DIR *d = opendir(path);
    if(d) {
        closedir(d);
        return true;
    }
    return false;
}
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static bool jo_dir_exists(const char *path)
{
    DWORD attrib = GetFileAttributes(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}
#undef min
#undef max
#endif

// 
// Simple C++std replacements...
//

#define JO_M_PI 3.14159265358979323846
#define JO_M_PI_2 1.57079632679489661923
#define JO_M_PI_4 0.785398163397448309616
#define JO_M_1_PI 0.318309886183790671538
#define JO_M_2_PI 0.636619772367581343076
#define JO_M_2_SQRTPI 1.12837916709551257390
#define JO_M_SQRT2 1.41421356237309504880
#define JO_M_SQRT1_2 0.707106781186547524401
#define JO_M_LOG2E 1.44269504088896340736
#define JO_M_LOG10E 0.434294481903251827651
#define JO_M_LN2 0.693147180559945309417
#define JO_M_LN10 2.30258509299404568402
#define JO_M_E 2.7182818284590452354


template<typename T> struct jo_numeric_limits;

template<> struct jo_numeric_limits<int> {
    static int max() { return INT_MAX; }
    static int min() { return INT_MIN; }
};

#define jo_endl ("\n")
#define jo_string_npos ((size_t)(-1))

// jo_pair is a simple pair of values
template<typename T1, typename T2>
struct jo_pair {
    T1 first;
    T2 second;
};

struct jo_string {
    char *str;
    
    jo_string() { str = jo_strdup(""); }
    jo_string(const char *ss) { str = jo_strdup(ss); }
    jo_string(char c) { str = jo_strdup(" "); str[0] = c; }
    jo_string(const jo_string *other) { str = jo_strdup(other->str); }
    jo_string(const jo_string &other) { str = jo_strdup(other.str); }
    jo_string(const char *a, size_t size) {
        str = (char*)malloc(size+1);
        memcpy(str, a, size);
        str[size] = 0;
    }
    jo_string(const char *a, const char *b) {
        ptrdiff_t size = b - a;
        str = (char*)malloc(size+1);
        memcpy(str, a, size);
        str[size] = 0;
    }

    ~jo_string() {
        free(str);
        str = 0;
    }

    const char *c_str() const { return str; };
    int compare(const jo_string &other) { return strcmp(str, other.str); }
    bool empty() const { return str[0] == 0; }
    size_t size() const { return strlen(str); }
    size_t length() const { return strlen(str); }

    jo_string &operator=(const char *s) {
        char *tmp = jo_strdup(s);
        free(str);
        str = tmp;
        return *this;
    }

    jo_string &operator=(const jo_string &s) {
        char *tmp = jo_strdup(s.str);
        free(str);
        str = tmp;
        return *this;
    }

    jo_string &operator+=(const char *s) {
        size_t l0 = strlen(str);
        size_t l1 = strlen(s);
        char *new_str = (char*)realloc(str, l0 + l1 + 1);
        if(!new_str) {
            // malloc failed!
            return *this;
        }
        str = new_str;
        memcpy(str+l0, s, l1);
        str[l0+l1] = 0;
        return *this;
    }
    jo_string &operator+=(const jo_string &s) { *this += s.c_str(); return *this; }

    jo_string &operator+=(char c) {
        size_t l0 = strlen(str);
        char *new_str = (char*)realloc(str, l0 + 2);
        if(!new_str) {
            // malloc failed!
            return *this;
        }
        str = new_str;
        str[l0] = c;
        str[l0+1] = 0;
        return *this;
    }

    size_t find_last_of(char c) {
        char *tmp = strrchr(str, c);
        if(!tmp) return jo_string_npos;
        return (size_t)(tmp - str);
    }

    jo_string &erase(size_t n) {
        str[n] = 0;
        return *this;
    }

    jo_string substr(size_t pos = 0, size_t len = jo_string_npos) {
        if(len == jo_string_npos) {
            len = size() - pos;
        }
        return jo_string(str + pos, len);
    }

    size_t find(char c, size_t pos = 0) const {
        const char *tmp = strchr(str+pos, c);
        if(!tmp) return jo_string_npos;
        return (size_t)(tmp - str);
    }
    size_t find(const char *s, size_t pos = 0) const {
        const char *tmp = strstr(str+pos, s);
        if(!tmp) return jo_string_npos;
        return (size_t)(tmp - str);
    }
    size_t find(const jo_string &s, size_t pos = 0) const { return find(s.c_str(), pos); }

    static jo_string format(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(0, 0, fmt, args);
        va_end(args);
        if(len < 0) {
            // error
            return jo_string();
        }
        char *tmp = (char*)malloc(len+1);
        if(!tmp) {
            // malloc failed!
            return jo_string();
        }
        va_start(args, fmt);
        vsnprintf(tmp, len+1, fmt, args);
        va_end(args);
        return jo_string(tmp);
    }




};

static inline jo_string operator+(const jo_string &lhs, const jo_string &rhs) { jo_string ret(lhs); ret += rhs; return ret; }
static inline jo_string operator+(const jo_string &lhs, const char *rhs) { jo_string ret(lhs); ret += rhs; return ret; }
static inline jo_string operator+(const char *lhs, const jo_string &rhs) { jo_string ret(lhs); ret += rhs; return ret; }
static inline jo_string operator+(const jo_string &lhs, char rhs) { jo_string ret(lhs); ret += rhs; return ret; }
static inline jo_string operator+(char lhs, const jo_string &rhs) { jo_string ret(lhs); ret += rhs; return ret; }
static inline bool operator==(const jo_string &lhs, const jo_string &rhs) { return !strcmp(lhs.c_str(), rhs.c_str()); }
static inline bool operator==(const char *lhs, const jo_string &rhs) { return !strcmp(lhs, rhs.c_str()); }
static inline bool operator==(const jo_string &lhs, const char *rhs) { return !strcmp(lhs.c_str(), rhs); }
static inline bool operator!=(const jo_string &lhs, const jo_string &rhs) { return !!strcmp(lhs.c_str(), rhs.c_str()); }
static inline bool operator!=(const char *lhs, const jo_string &rhs) { return !!strcmp(lhs, rhs.c_str()); }
static inline bool operator!=(const jo_string &lhs, const char *rhs) { return !!strcmp(lhs.c_str(), rhs); }
static inline bool operator<(const jo_string &lhs, const jo_string &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) < 0; }
static inline bool operator<(const char *lhs, const jo_string &rhs) { return strcmp(lhs, rhs.c_str()) < 0; }
static inline bool operator<(const jo_string &lhs, const char *rhs) { return strcmp(lhs.c_str(), rhs) < 0; }
static inline bool operator<=(const jo_string &lhs, const jo_string &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) <= 0; }
static inline bool operator<=(const char *lhs, const jo_string &rhs) { return strcmp(lhs, rhs.c_str()) <= 0; }
static inline bool operator<=(const jo_string &lhs, const char *rhs) { return strcmp(lhs.c_str(), rhs) <= 0; }
static inline bool operator>(const jo_string &lhs, const jo_string &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) > 0; }
static inline bool operator>(const char *lhs, const jo_string &rhs) { return strcmp(lhs, rhs.c_str()) > 0; }
static inline bool operator>(const jo_string &lhs, const char *rhs) { return strcmp(lhs.c_str(), rhs) > 0; }
static inline bool operator>=(const jo_string &lhs, const jo_string &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) >= 0; }
static inline bool operator>=(const char *lhs, const jo_string &rhs) { return strcmp(lhs, rhs.c_str()) >= 0; }
static inline bool operator>=(const jo_string &lhs, const char *rhs) { return strcmp(lhs.c_str(), rhs) >= 0; }

struct jo_stringstream {
    jo_string s;

    jo_string &str() { return s; }
    const jo_string &str() const { return s; }

    jo_stringstream &operator<<(int val) {
        char tmp[33];
#ifdef _MSC_VER
        sprintf_s(tmp, "%i", val);
#else
        sprintf(tmp, "%i", val);
#endif
        s += tmp;
        return *this;
    }

    jo_stringstream &operator<<(const char *val) {
        s += val;
        return *this;
    }
};

template<typename T> static inline T jo_min(T a, T b) { return a < b ? a : b; }
template<typename T> static inline T jo_max(T a, T b) { return a > b ? a : b; }

#if !defined(__PLACEMENT_NEW_INLINE) && !defined(_MSC_VER)
inline void *operator new(size_t, void *p) { return p; }
#endif

template<typename T>
struct jo_vector {
    T *ptr;
    size_t ptr_size;
    size_t ptr_capacity;

    jo_vector() {
        ptr = 0;
        ptr_size = 0;
        ptr_capacity = 0;
    }

    jo_vector(size_t n) {
        ptr = 0;
        ptr_size = 0;
        ptr_capacity = 0;
        
        resize(n);
    }

    ~jo_vector() {
        resize(0);
    }

    size_t size() const { return ptr_size; }

    T *data() { return ptr; }
    const T *data() const { return ptr; }

    T *begin() { return ptr; }
    const T *begin() const { return ptr; }

    T *end() { return ptr + ptr_size; }
    const T *end() const { return ptr + ptr_size; }

    T *rbegin() { return ptr + ptr_size - 1; }
    const T *rbegin() const { return ptr + ptr_size - 1; }

    T *rend() { return ptr - 1; }
    const T *rend() const { return ptr - 1; }

    T &at(size_t i) { return ptr[i]; }
    const T &at(size_t i) const { return ptr[i]; }

    T &operator[](size_t i) { return ptr[i]; }
    const T &operator[](size_t i) const { return ptr[i]; }

    void resize(size_t n) {
        if(n < ptr_size) {
            // call dtors on stuff your destructing before moving memory...
            for(size_t i = n; i < ptr_size; ++i) {
                ptr[i].~T();
            }
        }

        if(n > ptr_capacity) {
            T *newptr = (T*)malloc(n*sizeof(T));
            if(!newptr) {
                // malloc failed!
                return;
            }
            if(ptr) {
                memcpy(newptr, ptr, ptr_size*sizeof(T));
                free(ptr);
            }
            ptr = newptr;
            ptr_capacity = n;
        }

        if(n > ptr_size) {
            // in-place new on new data after moving memory to new location
            for(size_t i = ptr_size; i < n; ++i) {
                new(ptr+i) T(); // default c-tor
            }
        }
        
        if ( n == 0 )
        {
          if ( ptr ) free( ptr );
          ptr = 0;
          ptr_size = 0;
          ptr_capacity = 0;
        }

        ptr_size = n;
    }
    void clear() { resize(0); }

    void insert(const T *where, const T *what, size_t how_many) {
        if(how_many == 0) {
            return;
        }

        size_t n = ptr_size + how_many;
        ptrdiff_t where_at = where - ptr;

        // resize if necessary
        if(n > ptr_capacity) {
            size_t new_capacity = n + n/2; // grow by 50%
            T *newptr = (T*)malloc(new_capacity*sizeof(T));
            if(!newptr) {
                // malloc failed!
                return;
            }
            if(ptr) {
                memcpy(newptr, ptr, ptr_size*sizeof(T));
                free(ptr);
            }
            ptr = newptr;
            ptr_capacity = new_capacity;
        }

        // simple case... add to end of array.
        if(where == ptr+ptr_size || where == 0) {
            for(size_t i = ptr_size; i < n; ++i) {
                new(ptr+i) T(what[i - ptr_size]);
            }
            ptr_size = n;
            return;
        }

        // insert begin/middle means we need to move the data past where to the right, and insert how_many there...
        memmove(ptr + where_at + how_many, ptr + where_at, sizeof(T)*(ptr_size - where_at));
        for(size_t i = where_at; i < where_at + how_many; ++i) {
            new(ptr+i) T(what[i - where_at]);
        }
        ptr_size = n;
    }

    void insert(const T *where, const T *what_begin, const T *what_end) {
        insert(where, what_begin, (size_t)(what_end - what_begin));
    }

    void push_back(const T& val) { insert(end(), &val, 1); }
    void push_front(const T& val) { insert(begin(), &val, 1); }
    T pop_back() { T ret = ptr[ptr_size-1]; resize(ptr_size-1); return ret; }
    T &back() { return ptr[ptr_size-1]; }
    const T &back() const { return ptr[ptr_size-1]; }

    T &front() { return ptr[0]; }
    const T &front() const { return ptr[0]; }
};

template<typename T, typename TT>
void jo_sift_down(T *begin, T *end, size_t root, TT cmp) {
    ptrdiff_t n = end - begin;
    ptrdiff_t parent = root;
    ptrdiff_t child = 2 * parent + 1;
    while (child < n) {
        if (child + 1 < n && cmp(begin[child], begin[child + 1])) {
            child++;
        }
        if (!cmp(begin[child], begin[parent])) {
            T tmp = begin[child];
            begin[child] = begin[parent];
            begin[parent] = tmp;

            parent = child;
            child = 2 * parent + 1;
        } else {
            break;
        }
    }
}

template<typename T, typename TT>
void jo_sift_up(T *begin, ptrdiff_t child, TT cmp) {
   	ptrdiff_t parent = (child - 1) >> 1;
	while (child > 0) {
		if(!cmp(begin[child], begin[parent])) {
			T tmp = begin[child];
			begin[child] = begin[parent];
			begin[parent] = tmp;

			child = parent;
			parent = (child - 1) >> 1;
		} else {
			break;
		}
	}
}

template<typename T, typename TT>
void jo_make_heap(T *begin, T *end, TT cmp) {
    if(begin >= end) {
        return;
    }
    ptrdiff_t n = end - begin;
    ptrdiff_t root = (n - 2) >> 1;
    for(; root >= 0; --root) {
        jo_sift_down(begin, end, root, cmp);
    }
}

template<typename T, typename TT>
void jo_pop_heap(T *begin, T *end, TT cmp) {
    if(begin >= end) {
        return;
    }
    T tmp = begin[0];
    begin[0] = end[-1];
    end[-1] = tmp;
    jo_sift_down(begin, end-1, 0, cmp);
}

template<typename T, typename TT>
void jo_push_heap(T *begin, T *end, TT cmp) {
    ptrdiff_t n = end - begin;
    if(n <= 1) {
        return; // nothing to do...
    }
    jo_sift_up(begin, n - 1, cmp);
}

template<typename T, typename TT>
void jo_sort_heap(T *begin, T *end, TT cmp) {
    jo_make_heap(begin, end, cmp);
    ptrdiff_t n = end - begin;
    for (ptrdiff_t i = n; i > 0; --i) {
        begin[0] = begin[i-1];
        jo_sift_down(begin, begin + i - 1, 0, cmp);
    }
}

template<typename T>
const T *jo_find(const T *begin, const T *end, const T &needle) {
    for(const T *ptr = begin; ptr != end; ++ptr) {
        if(*ptr == needle) return ptr;
    }
    return end;
}

template<typename T>
T *jo_find(T *begin, T *end, const T &needle) {
    for(T *ptr = begin; ptr != end; ++ptr) {
        if(*ptr == needle) return ptr;
    }
    return end;
}

template<typename T>
const T *jo_find_if(const T *begin, const T *end, bool (*pred)(const T&)) {
    for(const T *ptr = begin; ptr != end; ++ptr) {
        if(pred(*ptr)) return ptr;
    }
    return end;
}

template<typename T>
T *jo_find_if(T *begin, T *end, bool (*pred)(const T&)) {
    for(T *ptr = begin; ptr != end; ++ptr) {
        if(pred(*ptr)) return ptr;
    }
    return end;
}

template<typename T>
T *jo_lower_bound(T *begin, T *end, T &needle) {
    ptrdiff_t n = end - begin;
    ptrdiff_t first = 0;
    ptrdiff_t last = n;
    while (first < last) {
        ptrdiff_t mid = (first + last) / 2;
        if (needle < begin[mid]) {
            last = mid;
        } else {
            first = mid + 1;
        }
    }
    return begin + first;
}

template<typename T>
T *jo_upper_bound(T *begin, T *end, T &needle) {
    ptrdiff_t n = end - begin;
    ptrdiff_t first = 0;
    ptrdiff_t last = n;
    while (first < last) {
        ptrdiff_t mid = (first + last) / 2;
        if (needle <= begin[mid]) {
            last = mid;
        } else {
            first = mid + 1;
        }
    }
    return begin + first;
}

template<typename T>
class jo_set {
    T *ptr;
    size_t ptr_size;
    size_t ptr_capacity;
    bool (*cmp)(const T&, const T&);
public:
    jo_set(bool (*cmp)(const T&, const T&)) : ptr(0), ptr_size(0), ptr_capacity(0), cmp(cmp) {}
    jo_set(const jo_set<T> &other) : ptr(0), ptr_size(0), ptr_capacity(0), cmp(other.cmp) {
        *this = other;
    }
    jo_set<T> &operator=(const jo_set<T> &other) {
        if(this == &other) return *this;
        if(ptr_capacity < other.ptr_size) {
            ptr_capacity = other.ptr_size;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        ptr_size = other.ptr_size;
        memcpy(ptr, other.ptr, sizeof(T)*ptr_size);
        return *this;
    }
    ~jo_set() {
        if(ptr) free(ptr);
    }

    void insert(const T &val) {
        if(ptr_size == ptr_capacity) {
            ptr_capacity = ptr_capacity ? ptr_capacity*2 : 4;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        ptr[ptr_size++] = val;
        jo_sift_up(ptr, ptr_size-1, cmp);
    }

    void emplace(T &&val) {
        if(ptr_size == ptr_capacity) {
            ptr_capacity = ptr_capacity ? ptr_capacity*2 : 4;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        ptr[ptr_size++] = val;
        jo_sift_up(ptr, ptr_size-1, cmp);
    }

    void erase(const T &val) {
        T *ptr = jo_find(this->ptr, this->ptr + ptr_size, val);
        if(ptr == this->ptr + ptr_size) return;
        ptr_size--;
        if(ptr != this->ptr + ptr_size) {
            T tmp = *ptr;
            *ptr = *(this->ptr + ptr_size);
            *(this->ptr + ptr_size) = tmp;
            jo_sift_down(this->ptr, this->ptr + ptr_size, (size_t)(ptr - this->ptr), cmp);
            jo_sift_up(this->ptr, ptr_size, cmp);
        }
    }

    void erase_if(bool (*pred)(const T&)) {
        T *ptr = jo_find_if(this->ptr, this->ptr + ptr_size, pred);
        if(ptr == this->ptr + ptr_size) return;
        ptr_size--;
        if(ptr != this->ptr + ptr_size) {
            T tmp = *ptr;
            *ptr = *(this->ptr + ptr_size);
            *(this->ptr + ptr_size) = tmp;
            jo_sift_down(this->ptr, this->ptr + ptr_size, (size_t)(ptr - this->ptr), cmp);
            jo_sift_up(this->ptr, ptr_size, cmp);
        }
    }

    void clear() { ptr_size = 0; }
    size_t size() const { return ptr_size; }
    size_t max_size() const { return ptr_capacity; }

    const T &operator[](size_t i) const { return ptr[i]; }
    T &operator[](size_t i) { return ptr[i]; }
    const T &at(size_t i) const { return ptr[i]; }
    T &at(size_t i) { return ptr[i]; }

    const T &front() const { return ptr[0]; }
    T &front() { return ptr[0]; }
    const T &back() const { return ptr[ptr_size-1]; }
    T &back() { return ptr[ptr_size-1]; }

    void sort() { jo_sort_heap(ptr, ptr + ptr_size, cmp); }

    void push(const T &val) { insert(val); }

    void pop_back() { erase(ptr[--ptr_size]); }
    void pop_front() { erase(ptr[0]); }

    void swap(jo_set<T> &other) {
        jo_swap(ptr, other.ptr);
        jo_swap(ptr_size, other.ptr_size);
        jo_swap(ptr_capacity, other.ptr_capacity);
        jo_swap(cmp, other.cmp);
    }

    void reserve(size_t n) {
        if(ptr_capacity < n) {
            ptr_capacity = n;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
    }

    void shrink_to_fit() {
        if(ptr_capacity > ptr_size) {
            ptr_capacity = ptr_size;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
    }

    void resize(size_t n) {
        if(ptr_capacity < n) {
            ptr_capacity = n;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        ptr_size = n;
    }

    void resize(size_t n, const T &val) {
        if(ptr_capacity < n) {
            ptr_capacity = n;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        if(ptr_size < n) {
            memset(ptr + ptr_size, 0, sizeof(T)*(n - ptr_size));
        }
        ptr_size = n;
    }

    void resize(size_t n, const T &val, bool (*cmp)(const T&, const T&)) {
        if(ptr_capacity < n) {
            ptr_capacity = n;
            ptr = (T*)realloc(ptr, sizeof(T)*ptr_capacity);
        }
        if(ptr_size < n) {
            memset(ptr + ptr_size, 0, sizeof(T)*(n - ptr_size));
        }
        ptr_size = n;
        jo_sort_heap(ptr, ptr + ptr_size, cmp);
    }

    T *lower_bound(const T &val) { return jo_lower_bound(ptr, ptr + ptr_size, val, cmp);}
    const T *lower_bound(const T &val) const { return jo_lower_bound(ptr, ptr + ptr_size, val, cmp);}

    T *upper_bound(const T &val) { return jo_upper_bound(ptr, ptr + ptr_size, val, cmp); }
    const T *upper_bound(const T &val) const { return jo_upper_bound(ptr, ptr + ptr_size, val, cmp); }
};

// jo_exception class
class jo_exception {
public:
    const char *msg;
    jo_exception(const char *msg) : msg(msg) {}
};

// std map like implementation
template<typename Key, typename Value> 
class jo_map {
    struct Node {
        Key key;
        Value value;
        Node *left, *right;
        Node(const Key &key, const Value &value) : key(key), value(value), left(nullptr), right(nullptr) {}
    };
    Node *root;
    size_t root_size;
    bool (*cmp)(const Key&, const Key&);
    void insert(Node *&root, const Key &key, const Value &value) {
        if(!root) {
            root = new Node(key, value);
            root_size++;
            return;
        }
        if(cmp(key, root->key)) {
            insert(root->left, key, value);
        } else if(cmp(root->key, key)) {
            insert(root->right, key, value);
        } else {
            root->value = value;
        }
    }

    void erase(Node *&root, const Key &key) {
        if(!root) return;
        if(cmp(key, root->key)) {
            erase(root->left, key);
        } else if(cmp(root->key, key)) {
            erase(root->right, key);
        } else {
            if(!root->left) {
                Node *tmp = root;
                root = root->right;
                delete tmp;
            } else if(!root->right) {
                Node *tmp = root;
                root = root->left;
                delete tmp;
            } else {
                Node *tmp = jo_min(root->right);
                root->key = tmp->key;
                root->value = tmp->value;
                erase(root->right, tmp->key);
            }
            root_size--;
        }
    }
    Node *find(Node *root, const Key &key) const {
        if(!root) return nullptr;
        if(cmp(key, root->key)) {
            return find(root->left, key);
        } else if(cmp(root->key, key)) {
            return find(root->right, key);
        } else {
            return root;
        }
    }

    void clear(Node *root) {
        if(!root) return;
        clear(root->left);
        clear(root->right);
        delete root;
    }

    void copy(Node *&root, Node *other) {
        if(!other) {
            root = nullptr;
            return;
        }
        root = new Node(other->key, other->value);
        copy(root->left, other->left);
        copy(root->right, other->right);
    }

    void swap(Node *&root, Node *&other) {
        Node *tmp = root;
        root = other;
        other = tmp;
    }
public:
    jo_map() : root(nullptr), root_size(0), cmp(nullptr) {}
    jo_map(const jo_map<Key, Value> &other) : root(nullptr), root_size(0), cmp(nullptr) {
        copy(root, other.root);
    }
    jo_map(jo_map<Key, Value> &&other) : root(nullptr), root_size(0), cmp(nullptr) {
        swap(root, other.root);
    }
    jo_map<Key, Value> &operator=(const jo_map<Key, Value> &other) {
        if(this == &other) return *this;
        clear(root);
        copy(root, other.root);
        return *this;
    }
    jo_map<Key, Value> &operator=(jo_map<Key, Value> &&other) {
        if(this == &other) return *this;
        clear(root);
        swap(root, other.root);
        return *this;
    }
    ~jo_map() {
        clear(root);
    }

    size_t size() const { return root_size; }
    bool empty() const { return !root_size; }

    void insert(const Key &key, const Value &value) {
        insert(root, key, value);
    }
    void erase(const Key &key) {
        erase(root, key);
    }
    Value &operator[](const Key &key) {
        Node *node = find(root, key);
        if(!node) {
            insert(key, Value());
            node = find(root, key);
        }
        return node->value;
    }
    const Value &operator[](const Key &key) const {
        Node *node = find(root, key);
        if(!node) {
            throw jo_exception("Key not found");
        }
        return node->value;
    }
    bool contains(const Key &key) const {
        return find(root, key) != nullptr;
    }
    Value &at(const Key &key) {
        Node *node = find(root, key);
        if(!node) {
            throw jo_exception("Key not found");
        }
        return node->value;
    }

    void clear() {
        clear(root);
        root = nullptr;
        root_size = 0;
    }

    void swap(jo_map<Key, Value> &other) {
        swap(root, other.root);
        swap(root_size, other.root_size);
        swap(cmp, other.cmp);
    }

    class iterator {
        Node *node;
        jo_map<Key, Value> *map;
        iterator(Node *node, jo_map<Key, Value> *map) : node(node), map(map) {}
    public:
        iterator() : node(nullptr), map(nullptr) {}
        iterator(const iterator &other) : node(other.node), map(other.map) {}
        iterator &operator=(const iterator &other) {
            node = other.node;
            map = other.map;
            return *this;
        }
        iterator &operator++() {
            if(node->right) {
                node = jo_min(node->right);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->right == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator &operator--() {
            if(node->left) {
                node = jo_max(node->left);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->left == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        bool operator==(const iterator &other) const {
            return node == other.node;
        }
        bool operator!=(const iterator &other) const {
            return node != other.node;
        }
        Key &key() { return node->key; }
        Value &value() { return node->value; }
        const Key &key() const { return node->key; }
        const Value &value() const { return node->value; }
    };

    iterator begin() {
        if(!root) return iterator();
        return iterator(jo_min(root), this);
    }

    iterator end() {
        return iterator();
    }

    iterator find(const Key &key) {
        return iterator(find(root, key), this);
    }

    class reverse_iterator {
        Node *node;
        jo_map<Key, Value> *map;
        reverse_iterator(Node *node, jo_map<Key, Value> *map) : node(node), map(map) {}

    public:
        reverse_iterator() : node(nullptr), map(nullptr) {}
        reverse_iterator(const reverse_iterator &other) : node(other.node), map(other.map) {}
        reverse_iterator &operator=(const reverse_iterator &other) {
            node = other.node;
            map = other.map;
            return *this;
        }
        reverse_iterator &operator++() {
            if(node->left) {
                node = jo_max(node->left);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->left == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        reverse_iterator operator++(int) {
            reverse_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        reverse_iterator &operator--() {
            if(node->right) {
                node = jo_min(node->right);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->right == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        reverse_iterator operator--(int) {
            reverse_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        bool operator==(const reverse_iterator &other) const {
            return node == other.node;
        }
        bool operator!=(const reverse_iterator &other) const {
            return node != other.node;
        }
        Key &key() { return node->key; }
        Value &value() { return node->value; }
        const Key &key() const { return node->key; }
        const Value &value() const { return node->value; }
    };

    reverse_iterator rbegin() {
        if(!root) return reverse_iterator();
        return reverse_iterator(jo_max(root), this);
    }

    reverse_iterator rend() {
        return reverse_iterator();
    }

    reverse_iterator rfind(const Key &key) {
        return reverse_iterator(find(root, key), this);
    }
};

template<typename Key, typename Value>
class jo_hash_map {
    struct Node {
        Key key;
        Value value;
        Node *left, *right;
        Node *parent;
        int height;
        Node(const Key &key, const Value &value) : key(key), value(value), left(nullptr), right(nullptr), parent(nullptr), height(1) {}
    };

    Node *root;
    int root_size;
    int (*hash)(const Key &key);
    bool (*equal)(const Key &a, const Key &b);

public:

    jo_hash_map() : root(nullptr), root_size(0), hash(nullptr), equal(nullptr) {}

    jo_hash_map(int (*hash)(const Key &key), bool (*equal)(const Key &a, const Key &b)) : root(nullptr), root_size(0), hash(hash), equal(equal) {}

    jo_hash_map(const jo_hash_map &other) : root(nullptr), root_size(0), hash(other.hash), equal(other.equal) {
        for(auto it = other.begin(); it != other.end(); ++it) {
            insert(new Node(it->key, it->value));
        }
    }

    jo_hash_map(jo_hash_map &&other) : root(other.root), root_size(other.root_size), hash(other.hash), equal(other.equal) {
        other.root = nullptr;
        other.root_size = 0;
    }

    jo_hash_map &operator=(const jo_hash_map &other) {
        jo_hash_map tmp(other);
        swap(tmp);
        return *this;
    }

    jo_hash_map &operator=(jo_hash_map &&other) {
        swap(other);
        return *this;
    }

    ~jo_hash_map() {
        clear();
    }

    void swap(jo_hash_map &other) {
        jo_swap(root, other.root);
        jo_swap(root_size, other.root_size);
        jo_swap(hash, other.hash);
        jo_swap(equal, other.equal);
    }

    void clear() {
        for(auto it = begin(); it != end(); ++it) {
            delete it.node;
        }
        root = nullptr;
        root_size = 0;
    }

    int count(const Key &key) const {
        return find(root, key) ? 1 : 0;
    }

    Value &operator[](const Key &key) {
        Node *node = find(root, key);
        if(node) return node->value;
        insert(new Node(key, Value()));
        return root->value;
    }

    const Value &operator[](const Key &key) const {
        Node *node = find(root, key);
        if(node) return node->value;
        throw jo_exception("jo_hash_map::operator[]");
    }

    bool contains(const Key &key) const {
        return find(root, key) != nullptr;
    }

    bool empty() const {
        return root_size == 0;
    }

    int size() const {
        return root_size;
    }

    class iterator {
        Node *node;
        jo_hash_map *map;
        friend class jo_hash_map;
        iterator(Node *node, jo_hash_map *map) : node(node), map(map) {}
    public:
        iterator() : node(nullptr), map(nullptr) {}
        iterator &operator++() {
            if(node->right) {
                node = jo_min(node->right);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->right == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator &operator--() {
            if(node->left) {
                node = jo_max(node->left);
            } else {
                Node *tmp = node;
                node = node->parent;
                while(node && node->left == tmp) {
                    tmp = node;
                    node = node->parent;
                }
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        bool operator==(const iterator &other) const {
            return node == other.node;
        }
        bool operator!=(const iterator &other) const {
            return node != other.node;
        }
        const Key &key() const { return node->key; }
        Value &value() const { return node->value; }
    };

    iterator begin() {
        if(!root) return iterator();
        return iterator(jo_min(root), this);
    }

    iterator end() {
        return iterator();
    }

    iterator find(const Key &key) {
        Node *node = find(root, key);
        if(node) return iterator(node, this);
        return end();
    }

    void insert(const Key &key, const Value &value) {
        insert(new Node(key, value));
    }

    void insert(const jo_pair<Key, Value> &pair) {
        insert(new Node(pair.first, pair.second));
    }

    void insert(Node *node) {
        if(!root) {
            root = node;
            root_size++;
            return;
        }
        Node *tmp = root;
        while(tmp) {
            if(hash(node->key) < hash(tmp->key)) {
                if(tmp->left) {
                    tmp = tmp->left;
                } else {
                    tmp->left = node;
                    node->parent = tmp;
                    root_size++;
                    insert_fixup(node);
                    return;
                }
            } else if(hash(node->key) > hash(tmp->key)) {
                if(tmp->right) {
                    tmp = tmp->right;
                } else {
                    tmp->right = node;
                    node->parent = tmp;
                    root_size++;
                    insert_fixup(node);
                    return;
                }
            } else {
                if(equal(node->key, tmp->key)) {
                    tmp->value = node->value;
                    delete node;
                    return;
                }
                if(tmp->left) {
                    tmp = tmp->left;
                } else {
                    tmp->left = node;
                    node->parent = tmp;
                    root_size++;
                    insert_fixup(node);
                    return;
                }
            }
        }
    }

    void erase(const Key &key) {
        Node *node = find(root, key);
        if(!node) return;
        erase(node);
    }

    void erase(iterator it) {
        erase(it.node);
    }

    void erase(Node *node) {
        if(!node) return;
        if(node->left && node->right) {
            Node *tmp = jo_max(node->left);
            node->key = tmp->key;
            node->value = tmp->value;
            node = tmp;
        }
        Node *child = node->left ? node->left :
                        node->right ? node->right : nullptr;
        if(child) {
            child->parent = node->parent;
            if(node->parent) {
                if(node->parent->left == node) {
                    node->parent->left = child;
                } else {
                    node->parent->right = child;
                }
            } else {
                root = child;
            }
        } else {
            if(node->parent) {
                if(node->parent->left == node) {
                    node->parent->left = nullptr;
                } else {
                    node->parent->right = nullptr;
                }
            } else {
                root = nullptr;
            }
        }
        delete node;
        root_size--;
    }
};

// std sort implementation using quicksort
template<typename T>
struct jo_sort {
    static void sort(T *array, int size) {
        if(size <= 1) return;
        int pivot = size / 2;
        jo_swap(array[0], array[pivot]);
        int i = 1;
        for(int j = 1; j < size; j++) {
            if(array[j] < array[0]) {
                jo_swap(array[i], array[j]);
                i++;
            }
        }
        jo_swap(array[0], array[i - 1]);
        sort(array, i - 1);
        sort(array + i, size - i);
    }
};

// std stable sort implementation using merge sort
template<typename T>
struct jo_stable_sort {
    static void merge(T *array, int size, int start, int mid, int end) {
        T *tmp = new T[end - start];
        int i = start, j = mid, k = 0;
        while(i < mid && j < end) {
            if(array[i] < array[j]) {
                tmp[k++] = array[i++];
            } else {
                tmp[k++] = array[j++];
            }
        }
    }

    static void sort(T *array, int size, int start, int end) {
        if(end - start <= 1) return;
        int mid = (start + end) / 2;
        sort(array, size, start, mid);
        sort(array, size, mid, end);
        merge(array, size, start, mid, end);
    }

    static void sort(T *array, int size) {
        sort(array, size, 0, size);
    }

    static void merge(T *array, int size, int start, int end) {
        if(end - start <= 1) return;
        int mid = (start + end) / 2;
        merge(array, size, start, mid, end);
        merge(array, size, mid, end);
    }

    static void merge(T *array, int size) {
        merge(array, size, 0, size);
    }
};


#ifdef _MSC_VER
#include <mutex>
#define jo_mutex std::mutex
#else
#include <pthread.h>
class jo_mutex {
    pthread_mutex_t mutex;
public:
    jo_mutex() { pthread_mutex_init(&mutex, nullptr); }
    ~jo_mutex() { pthread_mutex_destroy(&mutex); }
    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }
};
#endif

class jo_lock_guard {
    jo_mutex& mutex;
public:
    jo_lock_guard(jo_mutex& mutex) : mutex(mutex) { mutex.lock(); }
    ~jo_lock_guard() { mutex.unlock(); }
};

template<typename T> 
struct jo_shared_ptr {
    T* ptr;
    int* ref_count;
    
    jo_shared_ptr() : ptr(nullptr), ref_count(nullptr) {}
    jo_shared_ptr(T* ptr) : ptr(ptr), ref_count(new int(1)) {}
    jo_shared_ptr(const jo_shared_ptr& other) : ptr(other.ptr), ref_count(other.ref_count) {
        if(ref_count) (*ref_count)++;
    }
    jo_shared_ptr(jo_shared_ptr&& other) : ptr(other.ptr), ref_count(other.ref_count) {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }
    
    jo_shared_ptr& operator=(const jo_shared_ptr& other) {
        if (this != &other) {
            if(ref_count) {
                if(--(*ref_count) == 0) {
                    delete ptr;
                    delete ref_count;
                }
            }
            ptr = other.ptr;
            ref_count = other.ref_count;
            if(ref_count) (*ref_count)++;
        }
        return *this;
    }
    
    jo_shared_ptr& operator=(jo_shared_ptr&& other) {
        if (this != &other) {
            if(ref_count) {
                if(--(*ref_count) == 0) {
                    delete ptr;
                    delete ref_count;
                }
            }
            ptr = other.ptr;
            ref_count = other.ref_count;
            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }
    
    ~jo_shared_ptr() {
        if(ref_count) {
            --(*ref_count);
            if (*ref_count == 0) {
                delete ptr;
                delete ref_count;
            }
        }
    }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
    const T& operator*() const { return *ptr; }
    const T* operator->() const { return ptr; }

    bool operator==(const jo_shared_ptr& other) const { return ptr == other.ptr; }
    bool operator!=(const jo_shared_ptr& other) const { return ptr != other.ptr; }

    bool operator!() const { return ptr == nullptr; }
    operator bool() const { return ptr != nullptr; }
};

// jo_unqiue_ptr
template<typename T>
struct jo_unique_ptr {
    T* ptr;
    jo_unique_ptr(T* ptr) : ptr(ptr) {}
    jo_unique_ptr(const jo_unique_ptr& other) = delete;
    jo_unique_ptr(jo_unique_ptr&& other) : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    jo_unique_ptr& operator=(const jo_unique_ptr& other) = delete;
    jo_unique_ptr& operator=(jo_unique_ptr&& other) {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    ~jo_unique_ptr() {
        delete ptr;
    }
};


template<typename KEY, typename VALUE>
class jo_unordered_hash_map {
    struct Node {
        KEY key;
        VALUE value;
        Node *next;
    };
    Node *buckets;
    int bucket_count;
    Node *get_bucket(const KEY& key) {
        return buckets + (key % bucket_count);
    }
public:
    jo_unordered_hash_map() : bucket_count(0) {
        buckets = nullptr;
    }
    jo_unordered_hash_map(int bucket_count) : bucket_count(bucket_count) {
        buckets = new Node[bucket_count];
    }
    jo_unordered_hash_map(const jo_unordered_hash_map& other) : bucket_count(other.bucket_count) {
        buckets = new Node[bucket_count];
        for(int i = 0; i < bucket_count; i++) {
            Node *bucket = get_bucket(other.buckets[i]->key);
            Node *node = new Node(other.buckets[i]);
            node->next = bucket->ptr;
            bucket->ptr = node;
        }
    }
    jo_unordered_hash_map(jo_unordered_hash_map&& other) : bucket_count(other.bucket_count) {
        buckets = other.buckets;
        other.buckets = nullptr;
    }
    jo_unordered_hash_map& operator=(const jo_unordered_hash_map& other) {
        if (this != &other) {
            for(int i = 0; i < bucket_count; i++) {
                Node *bucket = get_bucket(other.buckets[i]->key);
                Node *node = new Node(other.buckets[i]);
                node->next = bucket->ptr;
                bucket->ptr = node;
            }
        }
        return *this;
    }
    jo_unordered_hash_map& operator=(jo_unordered_hash_map&& other) {
        if (this != &other) {
            for(int i = 0; i < bucket_count; i++) {
                Node *bucket = get_bucket(other.buckets[i]->key);
                Node *node = new Node(other.buckets[i]);
                node->next = bucket->ptr;
                bucket->ptr = node;
            }
            other.buckets = nullptr;
        }
        return *this;
    }
    ~jo_unordered_hash_map() {
        for(int i = 0; i < bucket_count; i++) {
            Node *bucket = get_bucket(buckets[i]->key);
            while(bucket->ptr) {
                Node *node = bucket->ptr;
                bucket->ptr = node->next;
                delete node;
            }
        }
        delete[] buckets;
    }

    VALUE& operator[](const KEY& key) {
        Node *bucket = get_bucket(key);
        while(bucket->ptr) {
            if (bucket->ptr->key == key) {
                return bucket->ptr->value;
            }
            bucket = bucket->ptr;
        }
        Node *node = new Node;
        node->key = key;
        node->value = VALUE();
        node->next = bucket->ptr;
        bucket->ptr = node;
        return node->value;
    }

    bool contains(const KEY& key) {
        Node *bucket = get_bucket(key);
        while(bucket->ptr) {
            if (bucket->ptr->key == key) {
                return true;
            }
            bucket = bucket->ptr;
        }
        return false;
    }

    VALUE& get(const KEY& key) {
        Node *bucket = get_bucket(key);
        while(bucket->ptr) {
            if (bucket->ptr->key == key) {
                return bucket->ptr->value;
            }
            bucket = bucket->ptr;
        }
        throw "Key not found";
    }

    void remove(const KEY& key) {
        Node *bucket = get_bucket(key);
        while(bucket->ptr) {
            if (bucket->ptr->key == key) {
                Node *node = bucket->ptr;
                bucket->ptr = node->next;
                delete node;
                return;
            }
            bucket = bucket->ptr;
        }
        throw "Key not found";
    }

    void clear() {
        for(int i = 0; i < bucket_count; i++) {
            Node *bucket = get_bucket(buckets[i]->key);
            while(bucket->ptr) {
                Node *node = bucket->ptr;
                bucket->ptr = node->next;
                delete node;
            }
        }
    }

    int size() {
        int size = 0;
        for(int i = 0; i < bucket_count; i++) {
            Node *bucket = get_bucket(buckets[i]->key);
            while(bucket->ptr) {
                size++;
                bucket = bucket->ptr;
            }
        }
        return size;
    }

    bool empty() {
        for(int i = 0; i < bucket_count; i++) {
            Node *bucket = get_bucket(buckets[i]->key);
            while(bucket->ptr) {
                return false;
            }
        }
        return true;
    }

    class iterator {
        Node *bucket;
        Node *node;
        jo_unordered_hash_map *map;

        void next() {
            while(bucket->ptr) {
                node = bucket->ptr;
                bucket = map->get_bucket(node->key);
            }
        }
    public:
        iterator(jo_unordered_hash_map *map) : map(map) {
            bucket = map->buckets;
            node = nullptr;
            next();
        }
        iterator(const iterator& other) : map(other.map), bucket(other.bucket), node(other.node) {}

        iterator& operator=(const iterator& other) {
            if (this != &other) {
                bucket = other.bucket;
                node = other.node;
            }
            return *this;
        }

        iterator& operator++() { node = node->next; next(); return *this; }
        iterator operator++(int) { iterator tmp(*this); operator++(); return tmp; }

        bool operator==(const iterator& other) { return node == other.node; }
        bool operator!=(const iterator& other) { return node != other.node; }
        VALUE& operator*() { return node->value; }
        VALUE* operator->() { return &node->value; }
    };

    iterator begin() { return iterator(this); }
    iterator end() { return iterator(this); }

    class const_iterator {
        Node *bucket;
        Node *node;
        jo_unordered_hash_map *map;

        void next() {
            while(bucket->ptr) {
                node = bucket->ptr;
                bucket = map->get_bucket(node->key);
            }
        }
    public:
        const_iterator(const const_iterator& other) : map(other.map), bucket(other.bucket), node(other.node) {}
        const_iterator(const iterator& other) : map(other.map), bucket(other.bucket), node(other.node) {}
        const_iterator& operator=(const const_iterator& other) {
            if (this != &other) {
                bucket = other.bucket;
                node = other.node;
            }
            return *this;
        }

        const_iterator& operator=(const iterator& other) {
            if (this != &other) {
                bucket = other.bucket;
                node = other.node;
            }
            return *this;
        }

        const_iterator& operator++() { node = node->next; next(); return *this; }
        const_iterator operator++(int) { const_iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const const_iterator& other) { return node == other.node; }
        bool operator!=(const const_iterator& other) { return node != other.node; }
        const VALUE& operator*() { return node->value; }
        const VALUE* operator->() { return &node->value; }
    };

    const_iterator begin() const { return const_iterator(this); }
    const_iterator end() const { return const_iterator(this); }

    // find implementation
    iterator find(const KEY& key) {
        Node *bucket = get_bucket(key);
        while(bucket->ptr) {
            if (bucket->ptr->key == key) {
                return iterator(this);
            }
            bucket = bucket->ptr;
        }
        return iterator(this);
    }
};

// std::function implementation
template<typename... ARGS>
class jo_function {
    typedef void (*func_ptr)(ARGS...);
    func_ptr func;
public:
    jo_function() : func(nullptr) {}
    jo_function(func_ptr func) : func(func) {}
    jo_function(const jo_function& other) : func(other.func) {}
    jo_function& operator=(const jo_function& other) {
        if (this != &other) {
            func = other.func;
        }
        return *this;
    }
    void operator()(ARGS... args) {
        if (func) {
            func(args...);
        }
    }
};

// Persistent Vector implementation (vector that supports versioning)
// For use in purely functional languages
template<typename T>
struct jo_persistent_vector
{
    struct node {
        jo_shared_ptr<node> children[32];
        T elements[32];

        node() : children(), elements() {}

        ~node() {
            for (int i = 0; i < 32; ++i) {
                children[i] = NULL;
            }
        }

        node(const jo_shared_ptr<node> other) : children(), elements() {
            if(other.ptr) {
                for (int i = 0; i < 32; ++i) {
                    children[i] = other->children[i];
                    elements[i] = other->elements[i];
                }
            }
        }
    };

    jo_shared_ptr<node> head;
    jo_shared_ptr<node> tail;
    size_t tail_length;
    size_t length;
    size_t depth;

    jo_persistent_vector() {
        head = tail = new node();
        tail_length = 0;
        length = 0;
        depth = 0;
    }

    jo_persistent_vector(const jo_persistent_vector &other) {
        head = other.head;
        tail = other.tail;
        tail_length = other.tail_length;
        length = other.length;
        depth = other.depth;
    }

    void append_tail() {
        size_t tail_offset = length - tail_length;
        size_t shift = 5 * (depth + 1);
        size_t max_size = 1 << (5 * shift);

        // check for root overflow, and if so expand tree by 1 level
        if(length >= max_size) {
            node *new_root = new node();
            new_root->children[0] = head;
            head = new_root;
            ++depth;
            shift = 5 * (depth + 1);
        }

        // Set up our tree traversal. We subtract 5 from level each time
        // in order to get all the way down to the level above where we want to
        // insert this tail node.
        jo_shared_ptr<node> cur = NULL;
        jo_shared_ptr<node> prev = head;
        size_t index = 0;
        size_t key = tail_offset;
        for(size_t level = shift; level > 0; level -= 5) {
            index = (key >> level) & 0x1f;
            cur = prev->children[index];
            // we are at the end of our tree, insert tail node
            if(!cur && level - 5 == 0) {
                prev->children[index] = tail;
                break;
            }
            // found a null node
            if(!cur) {
                cur = new node();
                prev->children[index] = cur;
            }
            prev = cur;
        }

        // Make our new tail
        tail = new node();
        tail_length = 0;
    }

    jo_persistent_vector *append(const T &value) {
        // Create a copy of our root node from which we will base our append
        jo_persistent_vector *copy = new jo_persistent_vector(*this);

        // do we have space?
        if(copy->tail_length >= 32) {
            copy->append_tail();
        }

        copy->tail->elements[copy->tail_length] = value;
        copy->tail_length++;
        copy->length++;
        return copy;
    }

    // append inplace
    jo_persistent_vector *append_inplace(const T &value) {
        // do we have space?
        if(tail_length >= 32) {
            append_tail();
        }

        tail->elements[tail_length] = value;
        tail_length++;
        length++;
        return this;
    }

    jo_persistent_vector *assoc(size_t index, const T &value) {
        int shift = 5 * depth;
        size_t tail_offset = length - tail_length;

        if(index >= length) {
            return append(value);
        }

        // Create a copy of our root node from which we will base our append
        jo_persistent_vector *copy = new jo_persistent_vector(*this);

        // traverse duplicating the way down.
        if(shift == 0) {
            copy->head = new node(copy->head);
            copy->head->elements[index] = value;
            copy->tail = copy->head;
            return copy;
        }

        jo_shared_ptr<node> cur = NULL;
        jo_shared_ptr<node> prev = copy->head;
        size_t key = index;
        for (int level = shift; level > 0; level -= 5) {
            size_t index = (key >> level) & 0x1f;
            // copy nodes as we traverse
            cur = new node(prev->children[index]);
            prev->children[index] = cur;
            prev = cur;
        }
        prev->elements[key & 0x1f] = value;

        // if we modified the tail, set it
        if(index >= tail_offset) {
            copy->tail = prev;
        }

        return copy;
    }

    jo_persistent_vector *cons(const T &value) {
        return append(value);
    }

    jo_persistent_vector *push_back(const T &value) {
        return append(value);
    }
    
    jo_persistent_vector *pop_back() {
        size_t shift = 5 * (depth + 1);
        size_t tail_offset = length - tail_length;

        // Create a copy of our root node from which we will base our append
        jo_persistent_vector *copy = new jo_persistent_vector(*this);

        // Is it in the tail?
        if(length == tail_offset) {
            // copy the tail (since we are changing the data)
            copy->tail = new node(*copy->tail);
            copy->tail_length--;
            copy->length--;
            return copy;
        }

        // traverse duplicating the way down.
        jo_shared_ptr<node> cur = NULL;
        jo_shared_ptr<node> prev = copy->head;
        size_t key = length - 1;
        for (size_t level = shift; level > 0; level -= 5) {
            size_t index = (key >> level) & 0x1f;
            // copy nodes as we traverse
            cur = new node(prev->children[index]);
            prev->children[index] = cur;
            prev = cur;
        }
        prev->elements[key & 0x1f] = T();
        copy->tail_length--;
        copy->length--;
        return copy;
    }

    T &operator[] (size_t index) {
        size_t shift = 5 * (depth + 1);
        size_t tail_offset = length - tail_length;

        // Is it in the tail?
        if(index >= tail_offset) {
            return tail->elements[index - tail_offset];
        }

        // traverse 
        jo_shared_ptr<node> cur = NULL;
        jo_shared_ptr<node> prev = head;
        size_t key = index;
        for (size_t level = shift; level > 0; level -= 5) {
            size_t index = (key >> level) & 0x1f;
            cur = prev->children[index];
            if(!cur) {
                return tail->elements[key - tail_offset];
            }
            prev = cur;
        }
        return prev->elements[key & 0x1f];
    }

    const T &operator[] (size_t index) const {
        size_t shift = 5 * (depth + 1);
        size_t tail_offset = length - tail_length;

        // Is it in the tail?
        if(index >= tail_offset) {
            return tail->elements[index - tail_offset];
        }

        // traverse

        jo_shared_ptr<node> cur = NULL;
        jo_shared_ptr<node> prev = head;
        size_t key = index;
        for (size_t level = shift; level > 0; level -= 5) {
            size_t index = (key >> level) & 0x1f;
            cur = prev->children[index];
            if(!cur) {
                return tail->elements[key - tail_offset];
            }
            prev = cur;
        }
        return prev->elements[key & 0x1f];
    }

    T &nth(size_t index) {
        return (*this)[index];
    }

    const T &nth(size_t index) const {
        return (*this)[index];
    }

    size_t size() const {
        return length;
    }

    jo_persistent_vector *clear() {
        return new jo_persistent_vector();
    }

    void print() const {
        printf("[");
        for(size_t i = 0; i < length; ++i) {
            if(i > 0) {
                printf(", ");
            }
            printf("%d", (*this)[i]);
        }
        printf("]");
    }

    // iterator
    class iterator {
    public:
        iterator(jo_shared_ptr<node> n, size_t i) : node(n), index(i) {}
        iterator(const iterator &other) : node(other.node), index(other.index) {}
        iterator &operator++() {
            ++index;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp(*this);
            ++index;
            return tmp;
        }
        operator bool () const {
            return node.ptr != NULL;
        }
        bool operator==(const iterator &other) const {
            return node == other.node && index == other.index;
        }
        bool operator!=(const iterator &other) const {
            return node != other.node || index != other.index;
        }
        T &operator*() {
            return node->elements[index];
        }
        T *operator->() {
            return &node->elements[index];
        }
    private:
        jo_shared_ptr<node> node;
        size_t index;
    };

    iterator begin() {
        return iterator(head, 0);
    }

    iterator end() {
        return iterator(tail, tail_length);
    }

    const iterator begin() const {
        return iterator(head, 0);
    }

    const iterator end() const {
        return iterator(tail, tail_length);
    }


    // reverse iterator
    class reverse_iterator {
    public:
        reverse_iterator(jo_shared_ptr<node> n, size_t i) : node(n), index(i) {}
        reverse_iterator(const reverse_iterator &other) : node(other.node), index(other.index) {}
        reverse_iterator &operator++() {
            --index;
            return *this;
        }
        reverse_iterator operator++(int) {
            reverse_iterator tmp(*this);
            --index;
            return tmp;
        }
        operator bool () const {
            return node.ptr != NULL;
        }
        bool operator==(const reverse_iterator &other) const {
            return node == other.node && index == other.index;
        }
        bool operator!=(const reverse_iterator &other) const {
            return node != other.node || index != other.index;
        }
        T &operator*() {
            return node->elements[index];
        }
        T *operator->() {
            return &node->elements[index];
        }
    private:
        jo_shared_ptr<node> node;
        size_t index;
    };

    reverse_iterator rbegin() const {
        return reverse_iterator(tail, tail_length);
    }

    reverse_iterator rend() const {
        return reverse_iterator(head, 0);
    }

    T &back() {
        return tail->elements[tail_length - 1];
    }

    const T &back() const {
        return tail->elements[tail_length - 1];
    }

    T &front() {
        return head->elements[0];
    }

    const T &front() const {
        return head->elements[0];
    }
};

// A persistent (non-destructive) linked list implementation.
template<typename T>
struct jo_persistent_list {
    struct node {
        jo_shared_ptr<node> next;
        T value;
        node() : value(), next() {}
        node(const T &value, jo_shared_ptr<node> next) : value(value), next(next) {}
        node(const node &other) : value(other.value), next(other.next) {}
        node &operator=(const node &other) {
            value = other.value;
            next = other.next;
            return *this;
        }
        bool operator==(const node &other) const { return value == other.value && next == other.next; }
        bool operator!=(const node &other) const { return !(*this == other); }
    };

    jo_shared_ptr<node> head;
    jo_shared_ptr<node> tail;
    size_t length;

    jo_persistent_list() : head(NULL), tail(NULL), length(0) {}
    jo_persistent_list(const jo_persistent_list &other) : head(other.head), tail(other.tail), length(other.length) {}
    jo_persistent_list &operator=(const jo_persistent_list &other) {
        head = other.head;
        tail = other.tail;
        length = other.length;
        infinite = other.infinite;
        return *this;
    }

    ~jo_persistent_list() {}

    jo_persistent_list *cons(const T &value) const {
        jo_persistent_list *copy = new jo_persistent_list(*this);
        copy->head = new node(value, copy->head);
        if(!copy->tail) {
            copy->tail = copy->head;
        }
        copy->length++;
        return copy;
    }

    jo_persistent_list *cons_inplace(const T &value) {
        head = new node(value, head);
        if(!tail) {
            tail = head;
        }
        length++;
        return this;
    }

    // makes a new list in reverse order of the current one
    jo_persistent_list *reverse() const {
        jo_persistent_list *copy = new jo_persistent_list();
        jo_shared_ptr<node> cur = head;
        while(cur) {
            copy->head = new node(cur->value, copy->head);
            if(!copy->tail) {
                copy->tail = copy->head;
            }
            cur = cur->next;
        }
        copy->length = length;
        return copy;
    }

    // This is a destructive operation.
    jo_persistent_list *reverse_inplace() {
        jo_shared_ptr<node> cur = head;
        jo_shared_ptr<node> prev = NULL;
        while(cur) {
            jo_shared_ptr<node> next = cur->next;
            cur->next = prev;
            prev = cur;
            cur = next;
        }
        tail = head;
        head = prev;
        return this;
    }

    jo_persistent_list *clone() const {
        jo_persistent_list *copy = new jo_persistent_list();
        jo_shared_ptr<node> cur = head;
        jo_shared_ptr<node> cur_copy = NULL;
        jo_shared_ptr<node> prev = NULL;
        while(cur) {
            cur_copy = new node(cur->value, NULL);
            if(prev) {
                prev->next = cur_copy;
            } else {
                copy->head = cur_copy;
            }
            prev = cur_copy;
            cur = cur->next;
        }
        copy->tail = prev;
        return copy;
    }

    // Clone up to and including the given index and link the rest of the list to the new list.
    jo_persistent_list *clone(int depth) const {
        jo_persistent_list *copy = new jo_persistent_list();
        jo_shared_ptr<node> cur = head;
        jo_shared_ptr<node> cur_copy = NULL;
        jo_shared_ptr<node> prev = NULL;
        while(cur && depth >= 0) {
            cur_copy = new node(cur->value, NULL);
            if(prev) {
                prev->next = cur_copy;
            } else {
                copy->head = cur_copy;
            }
            prev = cur_copy;
            cur = cur->next;
            depth--;
        }
        if(cur) {
            prev->next = cur;
            copy->tail = tail;
        } else {
            copy->tail = prev;
        }
        copy->length = length;
        return copy;
    }

    jo_persistent_list *conj(const jo_persistent_list &other) const {
        jo_persistent_list *copy = clone();
        if(copy->tail) {
            copy->tail->next = other.head;
        } else {
            copy->head = other.head;
        }
        copy->length += other.length;
        return copy;
    }

    // conj a value
    jo_persistent_list *conj(const T &value) const {
        jo_persistent_list *copy = clone();
        if(copy->tail) {
            copy->tail->next = new node(value, NULL);
            copy->tail = copy->tail->next;
        } else {
            copy->head = new node(value, NULL);
            copy->tail = copy->head;
        }
        copy->length++;
        return copy;
    }

    jo_persistent_list *assoc(size_t index, const T &value) const {
        jo_persistent_list *copy = new jo_persistent_list();
        jo_shared_ptr<node> cur = head;
        jo_shared_ptr<node> cur_copy = NULL;
        jo_shared_ptr<node> prev = NULL;
        while(cur && index >= 0) {
            cur_copy = new node(cur->value, NULL);
            if(prev) {
                prev->next = cur_copy;
            } else {
                copy->head = cur_copy;
            }
            prev = cur_copy;
            cur = cur->next;
            index--;
        }
        prev->value = value;
        if(cur) {
            prev->next = cur;
            copy->tail = tail;
        } else {
            copy->tail = prev;
        }
        copy->length = length;
        return copy;
    }

    // pop off value from front
    jo_persistent_list *pop() const {
        jo_persistent_list *copy = clone();
        if(copy->head) {
            copy->head = copy->head->next;
            copy->length--;
        }
        return copy;
    }

    const T &nth(int index) const {
        jo_shared_ptr<node> cur = head;
        while(cur && index > 0) {
            cur = cur->next;
            index--;
        }
        if(!cur) {
            throw jo_exception("nth");
        }
        return cur->value;
    }

    const T &operator[](int index) const {
        return nth(index);
    }

    jo_persistent_list *rest() const {
        jo_persistent_list *copy = new jo_persistent_list();
        if(head) {
            copy->head = head->next;
            copy->tail = tail;
            copy->length = length - 1;
        }
        return copy;
    }

    jo_persistent_list *first() const {
        // copy head node
        jo_persistent_list *copy = new jo_persistent_list();
        if(head) {
            copy->head = new node(head->value, NULL);
            copy->tail = copy->head;
            copy->length = 1;
        }
        return copy;
    }

    // push_back in_place
    jo_persistent_list *push_back_inplace(const T &value) {
        if(!head) {
            head = new node(value, NULL);
            tail = head;
        } else {
            tail->next = new node(value, NULL);
            tail = tail->next;
        }
        length++;
        return this;
    }

    jo_persistent_list *subvec(int start, int end) const {
        jo_shared_ptr<node> cur = head;
        while(cur && start > 0) {
            cur = cur->next;
            start--;
        }
        jo_persistent_list *copy = new jo_persistent_list();
        while(cur && end > start) {
            copy->push_back_inplace(cur->value);
            end--;
        }
        return copy;
    }

    // iterator
    class iterator {
        jo_shared_ptr<node> cur;
    public:
        iterator(jo_shared_ptr<node> cur) : cur(cur) {}
        iterator(const iterator &other) : cur(other.cur) {}
        iterator &operator=(const iterator &other) {
            cur = other.cur;
            return *this;
        }
        bool operator==(const iterator &other) const {
            return cur == other.cur;
        }
        bool operator!=(const iterator &other) const {
            return cur != other.cur;
        }
        operator bool() const {
            return cur;
        }
        iterator &operator++() {
            if(cur) {
                cur = cur->next;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator copy = *this;
            if(cur) {
                cur = cur->next;
            }
            return copy;
        }
        T &operator*() {
            return cur->value;
        }
        T *operator->() {
            return &cur->value;
        }
        const T &operator*() const {
            return cur->value;
        }
        const T *operator->() const {
            return &cur->value;
        }
    };

    iterator begin() const {
        return iterator(head);
    }

    iterator end() const {
        return iterator(tail);
    }

    const T &front() const {
        return head->value;
    }

    const T &back() const {
        return tail->value;
    }

    size_t size() const {
        return length;
    }

    bool empty() const {
        return !head;
    }

    void clear() {
        head = NULL;
        tail = NULL;
        length = 0;
    }
};

static const char *va(const char *fmt, ...) {
    static thread_local char tmp[0x10000];
    static thread_local int at = 0;
    char *ret = tmp+at;
    va_list args;
    va_start(args, fmt);
    at += 1 + vsnprintf(ret, sizeof(tmp)-at-1, fmt, args);
    va_end(args);
    if(at > sizeof(tmp) - 0x400) {
        at = 0;
    }
    return ret;
}

// Arbitrary precision integer
struct jo_bigint {
    jo_shared_ptr<jo_persistent_vector<int> > digits;
    bool negative;

    jo_bigint() : negative(false) {
        digits = new jo_persistent_vector<int>();
        digits->append_inplace(0);
    }
    jo_bigint(int n) : negative(n < 0) {
        digits = new jo_persistent_vector<int>();
        if(n < 0) {
            n = -n;
        }
        do {
            digits->append_inplace(n % 10);
            n /= 10;
        } while(n);
    }
    jo_bigint(const jo_bigint &other) : digits(other.digits), negative(other.negative) {}
    jo_bigint(const char *str) : negative(false) {
        digits = new jo_persistent_vector<int>();
        if(str[0] == '-') {
            negative = true;
            str++;
        }
        for(int i = strlen(str)-1; i >= 0; i--) {
            digits->append_inplace(str[i] - '0');
        }
    }
    jo_bigint(const char *str, int base) {
        if(base < 2 || base > 36) {
            throw jo_exception("jo_bigint: invalid base");
        }
        digits = new jo_persistent_vector<int>();
        negative = false;
        while(*str) {
            int digit = 0;
            digit = *str - '0';
            if(digit > 9) {
                digit = *str - 'a' + 10;
            }
            if(digit > 35) {
                digit = *str - 'A' + 10;
            }
            if(digit >= base) {
                throw jo_exception("jo_bigint: invalid digit");
            }
            digits->append_inplace(digit);
            str++;
        }
    }
    jo_bigint &operator=(const jo_bigint &other) {
        digits = other.digits;
        negative = other.negative;
        return *this;
    }
    jo_bigint &operator=(int n) {
        negative = n < 0;
        if(n < 0) {
            n = -n;
        }
        digits = digits->clear();
        while(n) {
            digits = digits->push_back(n % 10);
            n /= 10;
        }
        return *this;
    }
    jo_bigint &operator+=(const jo_bigint &other) {
        if(negative == other.negative) {
            int carry = 0;
            for(int i = 0; i < digits->size() || i < other.digits->size(); i++) {
                int a = i < digits->size() ? digits->nth(i) : 0;
                int b = i < other.digits->size() ? other.digits->nth(i) : 0;
                int sum = a + b + carry;
                carry = sum / 10;
                digits = digits->assoc(i, sum % 10);
            }
            if(carry) {
                digits = digits->push_back(carry);
            }
        } else {
            if(negative) {
                // -20 + 11 = -9 => 11 - 20 = -9
                *this = other - (-*this);
            } else {
                jo_bigint other_copy = other;
                other_copy.negative = false;
                *this -= other_copy;
            }
        }
        return *this;
    }

    jo_bigint &operator-=(const jo_bigint &other) {
        if(negative == other.negative) {
            if(negative) {
                *this += -other;
            } else {
                if(*this < other) {
                    jo_bigint other_copy = other;
                    other_copy.negative = false;
                    *this = other_copy - *this;
                    negative = true;
                } else {
                    //printf("%s - %s\n", to_string().c_str(), other.to_string().c_str());
                    int carry = 0;
                    for(int i = 0; i < digits->size() || i < other.digits->size(); i++) {
                        int a = i < digits->size() ? digits->nth(i) : 0;
                        int b = i < other.digits->size() ? other.digits->nth(i) : 0;
                        int diff = a - b - carry;
                        if(diff < 0) {
                            diff += 10;
                            carry = 1;
                        } else {
                            carry = 0;
                        }
                        digits = digits->assoc(i, diff);
                    }
                    negative = carry;
                    if(carry) {
                        digits = digits->pop_back();
                    } else {
                        // Remove trailing zeros
                        while(digits->size() > 1 && digits->nth(digits->size()-1) == 0) {
                            digits = digits->pop_back();
                        }
                    }
                }
            }
        } else {
            if(negative) {
                negative = false;
                *this += other;
                negative = true;
            } else {
                jo_bigint other_copy = other;
                other_copy.negative = false;
                *this += other_copy;
            }
        }
        return *this;
    }

    jo_bigint &operator++() {
        *this += 1;
        return *this;
    }

    jo_bigint &operator--() {
        *this -= 1;
        return *this;
    }

    jo_bigint operator++(int) {
        jo_bigint ret = *this;
        *this += 1;
        return ret;
    }

    jo_bigint operator--(int) {
        jo_bigint ret = *this;
        *this -= 1;
        return ret;
    }

    jo_bigint operator-() const {
        jo_bigint ret = *this;
        ret.negative = !ret.negative;
        return ret;
    }

    jo_bigint operator+(const jo_bigint &other) const {
        jo_bigint ret = *this;
        ret += other;
        return ret;
    }

    jo_bigint operator-(const jo_bigint &other) const {
        jo_bigint ret = *this;
        ret -= other;
        return ret;
    }

    bool operator==(const jo_bigint &other) const {
        if(negative != other.negative) {
            return false;
        }
        if(digits->size() != other.digits->size()) {
            return false;
        }
        if(digits->size() == 0) {
            return true;
        }
        auto it = digits->begin();
        auto it_other = other.digits->begin();
        while(it != digits->end() && it_other != other.digits->end()) {
            if(*it != *it_other) {
                return false;
            }
            it++;
            it_other++;
        }
        return true;
    }

    bool operator!=(const jo_bigint &other) const {
        return !(*this == other);
    }

    bool operator<(const jo_bigint &other) const {
        if(negative != other.negative) {
            return negative;
        }
        if(digits->size() != other.digits->size()) {
            return digits->size() < other.digits->size();
        }
        if(digits->size() == 0) {
            return false;
        }
        for(int i = digits->size()-1; i >= 0; --i) {
            if(digits->nth(i) != other.digits->nth(i)) {
                return digits->nth(i) < other.digits->nth(i);
            }
        }
        return false;
    }

    bool operator>(const jo_bigint &other) const {
        return other < *this;
    }

    bool operator<=(const jo_bigint &other) const {
        return !(other < *this);
    }

    bool operator>=(const jo_bigint &other) const {
        return !(*this < other);
    }

    // to string
    jo_string to_string() const {
        jo_string ret;
        if(negative) {
            ret += '-';
        }
        for(int i = digits->size() - 1; i >= 0; i--) {
            ret += digits->nth(i) + '0';
        }
        return ret;
    }

    // print
    void print() const {
        if(negative) {
            printf("-");
        }
        for(int i = digits->size() - 1; i >= 0; i--) {
            printf("%d", digits->nth(i));
        }
        printf("\n");
    }

    int to_int() const {
        int ret = 0;
        for(int i = digits->size() - 1; i >= 0; i--) {
            ret *= 10;
            ret += digits->nth(i);
        }
        if(negative) {
            ret *= -1;
        }
        return ret;
    }


};

// arbitrary precision floating point
struct jo_float {
    bool negative;
    jo_bigint mantissa;
    int exponent;

    jo_float() {
        negative = false;
        mantissa.digits = mantissa.digits->push_back(0);
        exponent = 0;
    }

    jo_float(const jo_float &other) {
        negative = other.negative;
        mantissa = other.mantissa;
        exponent = other.exponent;
    }

    jo_float(const int &other) {
        negative = other < 0;
        mantissa = jo_bigint(other);
        exponent = 0;
    }

    jo_float(const jo_bigint &other) {
        negative = other.negative;
        mantissa = other;
        exponent = 0;
    }

    jo_float(const double &other) {
        negative = other < 0;
        mantissa = jo_bigint(other);
        exponent = 0;
    }

    jo_float(const char *other) {
        negative = other[0] == '-';
        mantissa = jo_bigint(other);
        exponent = 0;
    }

    jo_float(const char *other, int base) {
        negative = other[0] == '-';
        mantissa = jo_bigint(other, base);
        exponent = 0;
    }

    jo_float(const char *other, int base, int exp) {
        negative = other[0] == '-';
        mantissa = jo_bigint(other, base);
        exponent = exp;
    }

    jo_float(const char *other, int base, int exp, int exp_base) {
        negative = other[0] == '-';
        mantissa = jo_bigint(other, base);
        exponent = exp * exp_base;
    }

    // operator overloads
    jo_float operator+(const jo_float &other) const {
        jo_float ret;
        ret.negative = negative;
        ret.exponent = exponent;
        ret.mantissa = mantissa + other.mantissa;
        return ret;
    }

    jo_float operator-(const jo_float &other) const {
        jo_float ret;
        ret.negative = negative;
        ret.exponent = exponent;
        ret.mantissa = mantissa - other.mantissa;
        return ret;
    }

    /*
    jo_float operator*(const jo_float &other) const {
        jo_float ret;
        ret.negative = negative;
        ret.exponent = exponent + other.exponent;
        ret.mantissa = mantissa * other.mantissa;
        return ret;
    }

    jo_float operator/(const jo_float &other) const {
        jo_float ret;
        ret.negative = negative;
        ret.exponent = exponent - other.exponent;
        ret.mantissa = mantissa / other.mantissa;
        return ret;
    }
    */

    // to string
    /*
    jo_string to_string() const {
        jo_string ret;
        if(negative) {
            ret += '-';
        }
        ret += mantissa.to_string();
        ret += '.';
        jo_bigint tmp = mantissa;
        tmp /= 10;
        while(tmp > 0) {
            ret += tmp.to_string();
            tmp /= 10;
        }
        ret += 'e';
        ret += jo_string::format("%i", exponent);
        return ret;
    }
    */
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // JO_STDCPP
