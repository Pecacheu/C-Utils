//C++ Utils v2.2.3, ©2021 Pecacheu; GNU GPL 3.0
#pragma once

#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <functional>

using namespace std;

#define UTILS_MAX_TIMERS 65536
#define NPOS string::npos

//64-bit:
#if _WIN64 || __amd64__ || __aarch64__
	#define ARCH64 1
#else
	#define ARCH64 0
#endif

//Win Patform:
#if _WIN32 || __WIN32
	#define WINDOWS 1
#else
	#define WINDOWS 0
#endif

//ARM Platform:
#if __ARM_ARCH_6__
	#define ARM 6
#elif __ARM_ARCH_7__
	#define ARM 7
#elif __ARM_ARCH_8__
	#define ARM 8
#elif __arm__ || __aarch64__
	#define ARM 1
#else
	#define ARM 0
#endif

//static_block macro:
#define CONCATENATE(s1, s2) s1##s2
#define EXPAND_THEN_CONCATENATE(s1, s2) CONCATENATE(s1, s2)
#ifdef __COUNTER__
	#define UNIQUE_IDENTIFIER(prefix) EXPAND_THEN_CONCATENATE(prefix, __COUNTER__)
#else
	#define UNIQUE_IDENTIFIER(prefix) EXPAND_THEN_CONCATENATE(prefix, __LINE__)
#endif
#define static_block STATIC_BLOCK_IMPL1(UNIQUE_IDENTIFIER(_static_block_))
#define STATIC_BLOCK_IMPL1(prefix) STATIC_BLOCK_IMPL2(CONCATENATE(prefix,_fn),CONCATENATE(prefix,_var))
#define STATIC_BLOCK_IMPL2(function_name,var_name) static void function_name(); \
static int var_name __attribute((unused)) = (function_name(), 0) ; static void function_name()

namespace utils {

typedef unordered_map<string,string> stringmap;
typedef function<void(void*)> EVLFunc;

//C String Help
const char *strCpy(const char *s);
const char *strCpy(const char *s, size_t max);
void strCpy(char *d, const char *s, size_t max);

const char bChar64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//Node.js Buffers
struct Buffer {
	Buffer(); Buffer(const size_t l);
	Buffer(const char *b, const size_t l, bool d=1, bool n=0);
	Buffer(const char *t); Buffer(const string& s);
	inline string toStr() { return string(buf,len); }
	inline string toStr(size_t s, size_t l=0) {
		s=min(s,len-1); return string(buf+s,l?min(l,len-s):len-s);
	}
	const char *toCStr(bool f=0); Buffer copy(size_t nl=0);
	const char *toBase64(char *b=0); Buffer sub(size_t o, size_t l=NPOS);
	bool match(const char *s); bool matchPart(const char *s, size_t ofs=0);
	inline char& operator[](size_t i){ return (char&)buf[i]; }
	void operator=(Buffer b); void del();
	const char *buf,*db; size_t len; bool nul;
};

void error(string e);
void error(string f, int c);
bool ckErr(int e, string f);

//String/Buffer Help
size_t bFind(Buffer& b, const char *s, size_t ofs=0, size_t end=0);
vector<Buffer> bSplit(Buffer& b, const char *sp);

string intToHex(size_t i);
size_t strToUint(string s);
size_t hexStrToUint(string s);
void replaceAll(string& s, string from, string to);

bool startsWith(const string& s, const char *t);
inline bool startsWith(const string& s, const string& t) { return startsWith(s,t.data()); }
bool startsWith(const char *s, const char *t);
inline bool endsWith(const string& s, const string& t) { return endsWith(s,t.data()); }
bool endsWith(const string& s, const char *t);
bool endsWith(const char *s, const char *t);

string toCamelCase(string s);
string decodeURIComponent(string s);
stringmap fromQuery(string s);
string toQuery(stringmap m);

//JavaScript Time & Event Loop
uint64_t usTime(); uint64_t msTime();
string getDate(uint64_t t=0, bool sec=0);


struct EVData {
	EVData(EVLFunc f, uint64_t t, void *p, uint64_t i):f(f),t(t),p(p),i(i) {}
	EVLFunc f; uint64_t t,i; void *p;
};
class EventLoop {
	public:
	EventLoop(size_t m=UTILS_MAX_TIMERS):max(m) {}
	size_t setTimeout(EVLFunc f, uint64_t ms, void *p=0);
	size_t setInterval(EVLFunc f, uint64_t ms, void *p=0);
	bool clearTimeout(size_t id); void run(bool ex=0);
	void stop(); private: volatile bool rl=0;
	mutex lck; unordered_map<size_t,EVData> ev;
	const size_t max;
};

extern EventLoop GlobalEventLoop;
inline size_t setTimeout(EVLFunc f, uint64_t ms, void *p=0) { return GlobalEventLoop.setTimeout(f,ms,p); }
inline size_t setInterval(EVLFunc f, uint64_t ms, void *p=0) { return GlobalEventLoop.setInterval(f,ms,p); }
inline bool clearTimeout(size_t id) { return GlobalEventLoop.clearTimeout(id); }
inline bool clearInterval(size_t id) { return GlobalEventLoop.clearTimeout(id); }

class Append {
	public: template<typename T, typename... U>
	static Buffer buf(const T& arg, const U&... args) {
		vector<Buffer> ref; return run(&ref, arg, args...);
	}
	template<typename T, typename... U>
	static const char *str(const T& arg, const U&... args) {
		vector<Buffer> ref; return run(&ref, arg, args...).buf;
	}
	private: static Buffer run(vector<Buffer> *ref) {
		vector<Buffer>& arr=*ref; size_t al=arr.size(),s=0;
		for(Buffer& b: arr) s+=b.len; char *a=new char[s+1], *o=a;
		for(Buffer& b: arr) { memcpy(o,b.buf,b.len); o+=b.len; if(b.db) b.del(); }
		a[s]=0; return Buffer(a,s,1,1);
	}
	template<typename T, typename... U>
	static Buffer run(vector<Buffer> *ref, const T& arg, const U&... args) {
		if constexpr(is_same_v<T,Buffer> || is_same_v<T,string>) ref->emplace_back(arg);
		else if constexpr(is_convertible_v<T,const char *>) ref->emplace_back(arg).db=0;
		else if constexpr(is_convertible_v<T,long>) ref->emplace_back(to_string(arg));
		else { error("Append TypeError: "+string(typeid(T).name())); exit(1); }
		return run(ref, args...);
	}
};

}