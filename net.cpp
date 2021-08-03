//C++ Utils, ©2021 Pecacheu; GNU GPL 3.0

#include "net.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

namespace net {

socklen_t AddrLen=0;
inline void setNb(int s) { fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK); }

NetAddr::~NetAddr() { delete (sockaddr_in*)a; a=0; }
NetAddr::NetAddr(uint16_t port):port(port),ip(0) {}
NetAddr::NetAddr(const char *addr, uint16_t port):host(addr),port(port),ip(inet_addr(addr)) {}
NetAddr::NetAddr(const NetAddr &a):host(a.host),port(a.port),ip(a.ip) {}
void NetAddr::in() {
	if(a) return; sockaddr_in *s=new sockaddr_in(); a=s; if(!AddrLen) AddrLen=sizeof(*s);
	s->sin_family=AF_INET; s->sin_addr.s_addr=ip?ip:INADDR_ANY;
	s->sin_port=htons(port);
}

NetAddr *NetAddr::resolve(const char *host, uint16_t port) {
	NetAddr *a=new NetAddr(port); a->in(); hostent *h=gethostbyname(host);
	if(!h || h->h_addrtype != AF_INET) return 0;
	((sockaddr_in*)a->a)->sin_addr.s_addr=a->ip=*(uint32_t*)h->h_addr;
	a->host=host; return a;
}

int netStartServer(NetAddr a, int backlog) {
	a.in(); int srv=socket(AF_INET, SOCK_STREAM, 0), r=1; if(srv < 0) return -1;
	if(setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &r, sizeof(int))) return -2;
	if(bind(srv, (sockaddr *)a.a, AddrLen)) return -3;
	if(listen(srv, backlog)) return -4;
	return srv;
}

Socket netAccept(int srv, bool nb) {
	sockaddr_in a=sockaddr_in(); socklen_t len=AddrLen;
	int sck=accept(srv, (sockaddr *)&a, &len); if(sck < 0) return Socket(-1);
	if(nb) setNb(sck); len=INET6_ADDRSTRLEN+1; char as[len];
	if(!inet_ntop(a.sin_family,&a.sin_addr.s_addr,(char*)&as,len)) { close(sck); return Socket(-2); }
	return Socket(srv,sck,NetAddr((char*)&as,ntohs(a.sin_port)));
}

Socket netConnect(NetAddr a, bool nb) {
	a.in(); int sck=socket(AF_INET, SOCK_STREAM, 0); if(sck < 0) return Socket(-1);
	if(connect(sck, (sockaddr *)a.a, AddrLen)) { close(sck); return Socket(-2); }
	if(nb) setNb(sck); return Socket(0,sck,a);
}

Socket::Socket(int e):err(e),srv(0),sck(0) {}
Socket::Socket(int s, int c, NetAddr a):err(0),srv(s),sck(c),addr(a) {}

int Socket::setTimeout(time_t sec) {
	timeval t=timeval(); t.tv_sec=sec;
	return setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
}

ssize_t Socket::read(char *buf, size_t len) { return recv(sck,buf,len,0); }
ssize_t Socket::write(const char *buf, size_t len) { return send(sck,buf,len,0); }
void Socket::close() { ::close(sck); }
void netClose(int s) { close(s); }

//UDP Protocol:

Dgram::Dgram(size_t buf, bool nb):bs(buf) {
	sck=socket(AF_INET, SOCK_DGRAM, 0);
	err=(sck<0)?-1:0; if(!err && nb) setNb(sck);
}

ssize_t Dgram::send(NetAddr to, const char *buf, size_t len) {
	to.in(); ssize_t s=sendto(sck, buf, len, MSG_CONFIRM, (sockaddr *)to.a, AddrLen);
	return s;
}
ssize_t Dgram::recv(char *buf, size_t size, char **addr, uint16_t *port) {
	sockaddr_in ab; socklen_t len=AddrLen;
	ssize_t r = recvfrom(sck, buf, size, MSG_WAITALL, (sockaddr *)&ab, &len);
	if(r <= 0) return r; len = INET6_ADDRSTRLEN+1; char *as = new char[len];
	if(!inet_ntop(ab.sin_family,&ab.sin_addr.s_addr,as,len)) return -2;
	*addr=as; *port=ntohs(ab.sin_port); return r;
}

int Dgram::onData(NetAddr a, DgramFunc cb) {
	if(dRun) return -1; dRun=1; if(!bind(a)) { dRun=0; return -2; }
	thread([this,cb]() {
		char dBuf[bs]; char *addr; uint16_t port;
		while(dRun) {
			ssize_t r = recv(dBuf, bs, &addr, &port); if(!dRun) break;
			if(r <= 0) { err=r?r:-1; cb(*this,Buffer(),0,0); break; }
			else { err=0; cb(*this,Buffer(dBuf,r,0),addr,port); }
		}
	}).detach();
	return 0;
}

bool Dgram::bind(NetAddr a) { a.in(); bool s=::bind(sck, (sockaddr *)a.a, AddrLen) == 0; return s; }
void Dgram::close() { dRun=0; ::close(sck); }

}