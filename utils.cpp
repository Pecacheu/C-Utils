//C++ Utils, ©2021 Pecacheu; GNU GPL 3.0

#include "utils.h"

namespace utils {

void error(string e) { cerr << "\033[31mError: "+e+"\033[0m\n"; }
void error(string f, int c) { cerr << "\033[31mError "+to_string(c)+" in "+f+": "+to_string(errno)+" "+strerror(errno)+"\033[0m\n"; }
bool ckErr(int e, string f) { if(e<0) { error(f,e); return 1; } return 0; }

Buffer::Buffer():buf(0),len(0),nul(0),db(0) {}
Buffer::Buffer(const size_t l):buf(l&&l!=NPOS?new char[l]:0),len(l),nul(0),db(buf) {}
Buffer::Buffer(const char *b, const size_t l, bool d, bool n):buf(b),len(l),nul(n),db(d?b:0) {}
Buffer::Buffer(const char *t):buf(t),len(t?strlen(t):0),nul(1),db(t) {}
Buffer::Buffer(const string& s):buf(s.data()),len(s.size()),nul(1),db(0) {}

const char *Buffer::toCStr(bool f) { return nul&&!f?buf:strCpy(buf,len); }
Buffer Buffer::copy(size_t nl) { Buffer b(nl>len?nl:len); memcpy((void*)b.buf,buf,len); b.nul=nul; return b; }
void Buffer::operator=(Buffer b) { buf=b.buf,len=b.len,nul=b.nul,db=b.db; b.db=0; }
void Buffer::del() { delete[] db; buf=0,db=0,len=0; }

const char *Buffer::toBase64(char *b) {
	if(!len) return ""; size_t i=len; char *np=b,*p=(char*)buf;
	if(!b) b=new char[(len*4/3)+2];
	while(i >= 3) {
		*((uint32_t*)np) = bChar64[*(p+2)&0x3f]<<24 | bChar64[((*(p+1)&0x0f)<<2) + ((*(p+2)&0xc0)>>6)]<<16
		| bChar64[((*p&0x03)<<4) + ((*(p+1)&0xf0)>>4)]<<8 | bChar64[(*p&0xfc)>>2]; p+=3; i-=3; np+=4;
	}
	if(i) {
		*np++ = bChar64[(*p&0xfc)>>2];
		*np++ = bChar64[((*p&0x03)<<4) + (((i>1?*(p+1):0)&0xf0)>>4)];
		if(i>1) *np++ = bChar64[((*(p+1)&0x0f)<<2)];
	}
	*np=0; return b;
}

Buffer Buffer::sub(size_t o, size_t l) {
	if(!buf) return *this; if(o>len) return Buffer();
	Buffer b(buf+o,min(l,len-o)); b.db=db; return b;
}
bool Buffer::match(const char *s) {
	size_t i=0,l=strlen(s); if(!l || l != len) return 0;
	for(; i<l; i++) if(buf[i] != s[i]) return 0; return 1;
}
bool Buffer::matchPart(const char *s, size_t ofs) {
	size_t i=0,l=strlen(s); if(!l || l+ofs > len) return 0;
	const char *t = buf+ofs; for(; i<l; i++) if(t[i] != s[i]) return 0; return 1;
}

size_t bFind(Buffer& b, const char *s, size_t ofs, size_t end) {
	size_t i,o=ofs,sl=strlen(s),l=b.len-sl; if(!sl || sl+ofs > b.len) return NPOS;
	const char *t = b.buf+o; if(end && end < l) l = end;
	for(; o<=l; o++,t++) { for(i=0; i<sl; i++) if(t[i] != s[i]) break; if(i == sl) return o; }
	return NPOS;
}
vector<Buffer> bSplit(Buffer& b, const char *sp) {
	size_t i,o=0,sl=strlen(sp); vector<Buffer> bl; const char *t = b.buf;
	while(1) {
		if((i=bFind(b,sp,o)) == NPOS) { if(o < b.len) bl.push_back(Buffer(t+o,b.len-o)); break; }
		if(o < i) bl.push_back(Buffer(t+o,i-o)); o = i+sl;
	}
	return bl;
}

string intToHex(size_t i) { stringstream s; s << hex << i; return s.str(); }
size_t strToUint(string s) {
	size_t n=0,ss=s.size(); if(!ss) return NPOS;
	char c,*p=&s[0],*l=p+ss;
	while(p<l) { c = *p++ - '0'; if(c > 9 || c < 0) return NPOS; n=n*10+c; }
	return n;
}
size_t hexStrToUint(string s) {
	size_t n=0,ss=s.size(); if(!ss) return NPOS;
	char c,*p=&s[0],*l=p+ss;
	while(p<l) {
		c = *p++; if(c <= '9') { if(c < '0') return NPOS; c -= '0'; }
		else if(c <= 'F') { if(c < 'A') return NPOS; c -= 'A'-10; }
		else if(c <= 'f') { if(c < 'a') return NPOS; c -= 'a'-10; }
		else return NPOS; n=n*16+c;
	}
	return n;
}

void replaceAll(string& s, string from, string to) {
	size_t fs=from.size(),ts=to.size(),st=0;
	while((st=s.find(from,st)) != NPOS) { s.replace(st,fs,to); st+=ts; }
}

string toCamelCase(string s) {
	char *d=s.data(); for(size_t i=0,l=s.size(); i<l; i++)
		if(!i || d[i-1]=='-' || d[i-1]==' ') d[i]=toupper(d[i]); else d[i]=tolower(d[i]);
	return s;
}

const regex uriEsc("%[0-9A-F]{2}");
string decodeURIComponent(string s) {
	string::iterator i=s.begin(); match_results<string::iterator> m;
	while(regex_search(i, s.end(), m, uriEsc)) {
		size_t n=hexStrToUint(m.str().substr(1));
		if((n == 10 || n >= 32) && n != NPOS) {
			i+=m.position(); s.replace(i, i+3, string(1,n));
		}
		i=s.begin()+m.position()+1;
	}
	return s;
}

stringmap fromQuery(string s) {
	if(startsWith(s,"?")) s=s.substr(1);
	stringmap m; size_t o=0,n=s.find('&'),q;
	string p,k,v; while(1) {
		p=s.substr(o,n==NPOS?NPOS:n-o);
		q=p.find('='); if(q == NPOS) return stringmap();
		k=p.substr(0,q), v=p.substr(q+1);
		if(!k.size() || !v.size()) return stringmap();
		m[k]=decodeURIComponent(v); if(n == NPOS) return m;
		o=n+1, n=s.find('&',o);
	}
}
string toQuery(stringmap m) {
	string s; bool n=0;
	for(auto& k: m) { s += (n?"&":"")+k.first+'='+k.second; if(!n) n=1; }
	return s;
}

const char *strCpy(const char *s) {
	if(!s) return 0; size_t l = strlen(s)+1;
	char *n = new char[l]; memcpy(n,s,l); return n;
}
const char *strCpy(const char *s, size_t max) {
	if(!s) return 0; max = strnlen(s,max);
	char *n = new char[max+1]; memcpy(n,s,max); n[max]=0; return n;
}
void strCpy(char *d, const char *s, size_t max) {
	if(!d || !s) return; max = strnlen(s,max);
	memcpy(d,s,max); d[max]=0;
}

bool startsWith(const string& s, const char *t) {
	size_t i=0,l=strlen(t); if(s.size()<l) return 0;
	for(; i<l; i++) if(t[i] != s[i]) return 0; return 1;
}
bool startsWith(const char *s, const char *t) {
	size_t i=0,l=strlen(t); if(strlen(s)<l) return 0;
	for(; i<l; i++) if(t[i] != s[i]) return 0; return 1;
}
bool endsWith(const string& s, const char *t) {
	size_t si=s.size(),l=strlen(t),i=0; if(si<l) return 0; si-=l;
	for(; i<l; i++,si++) if(t[i] != s[si]) return 0; return 1;
}
bool endsWith(const char *s, const char *t) {
	size_t si=strlen(s),l=strlen(t),i=0; if(si<l) return 0; si-=l;
	for(; i<l; i++,si++) if(t[i] != s[si]) return 0; return 1;
}

Buffer runCmd(string cmd, string e) {
	const char *a[4] {"/bin/sh","-c",cmd.data(),0};
	int p[2]; pid_t pid; if(pipe(p)) { error(e,-1); return Buffer(NPOS); }
	if((pid=fork()) < 0) { error(e,-2); return Buffer(NPOS); }
	else if(!pid) { //Runs in fork:
		dup2(p[1],STDOUT_FILENO); dup2(p[1],STDERR_FILENO);
		close(p[1]); close(p[0]); execv(a[0],(char*const*)a); exit(-1);
	}
	close(p[1]);
	char *o,*b; size_t l=0,r=1; while(r) {
		o=b,b=new char[l+UTILS_CMD_READ]; if(l) { memcpy(b,o,l); delete[] o; }
		r=read(p[0],b+l,UTILS_CMD_READ); l+=r;
	}
	close(p[0]); waitpid(pid,0,0);
	return Buffer(b,l);
}

uint64_t usTime() {
	return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}
uint64_t msTime() {
	return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

string Month[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
string tFill(int t) { string s=to_string(t); return t>9?s:'0'+s; }

string getDate(uint64_t t, bool sec) {
	if(!t) t=time(0); tm *d=localtime((time_t*)&t); bool p=0;
	int h=d->tm_hour; if(h>=12) p=1; if(h>12) h-=12; else if(h==0) h=12;
	return to_string(d->tm_year+1900)+' '+Month[d->tm_mon]+' '+tFill(d->tm_mday)+' '+tFill(h)+':'
	+tFill(d->tm_min)+(sec?':'+tFill(d->tm_sec):"")+(p?" PM":" AM");
}

EventLoop GlobalEventLoop;
void EventLoop::run(bool ex) {
	if(rl) return; rl=1,wl=0; uint64_t t;
	while(rl) {
		lck.lock();
		while(wl) { mutex& m=(mutex&)*wl; lck.unlock(); m.lock(); wl=0; m.unlock(); lck.lock(); } //Queue for unlocks.
		t=msTime();
		unordered_map<size_t,EVData>::iterator i=ev.begin(),v,l=ev.end();
		if(ex && i==l) rl=0; while(i!=l) {
			v=i++; EVData& e = (*v).second; if(t >= e.t) {
				e.f(e.p); if(e.i) e.t = t+e.i; else ev.erase(v);
			}
		}
		lck.unlock();
		this_thread::sleep_for(chrono::milliseconds(1));
	}
}
void EventLoop::wait(mutex& m) {
	lck.lock(); while((rl && wl) || !m.try_lock()) { //Queue for locks.
		lck.unlock(); this_thread::yield(); lck.lock();
	}
	wl=&m; lck.unlock();
}
void EventLoop::stop() { rl=0; }

size_t EventLoop::setTimeout(EVLFunc f, uint64_t ms, void *p) {
	if(!f||!rl) return 0; uint64_t t=ms+msTime(); size_t n=0; lck.lock();
	auto e=ev.end(); while(n<max) if(ev.find(++n) == e) break;
	if(n == max) { lck.unlock(); return 0; }
	ev.emplace(piecewise_construct,forward_as_tuple(n),forward_as_tuple(f,t,p,0));
	lck.unlock(); return n;
}

size_t EventLoop::setInterval(EVLFunc f, uint64_t ms, void *p) {
	if(!f||!ms||!rl) return 0; uint64_t t=ms+msTime(); size_t n=0; lck.lock();
	auto e=ev.end(); while(n<max) if(ev.find(++n) == e) break;
	if(n == max) { lck.unlock(); return 0; }
	ev.emplace(piecewise_construct,forward_as_tuple(n),forward_as_tuple(f,t,p,ms));
	lck.unlock(); return n;
}

bool EventLoop::clearTimeout(size_t id) {
	if(!id||!rl) return 0; lck.lock();
	auto t=ev.find(id); if(t==ev.end()) { lck.unlock(); return 0; }
	ev.erase(t); lck.unlock(); return 1;
}

}