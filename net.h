//C++ Utils v2.1, ©2021 Pecacheu; GNU GPL 3.0
#pragma once

#include "utils.h"

namespace net {

struct NetAddr {
	NetAddr(uint16_t port=0);
	NetAddr(const char *addr, uint16_t port=0);
	static NetAddr *resolve(const char *host, uint16_t port=80);
	const char *host; uint16_t port; void *a;
};

struct Socket {
	Socket(int e):err(e) {}
	Socket(int s, int c, NetAddr a):err(0),srv(s),sck(c),addr(a) {}
	ssize_t read(char *buf, size_t size); ssize_t write(const char *buf, size_t len);
	int setTimeout(time_t sec); void close();
	int err,srv,sck; NetAddr addr;
};

struct Dgram;
typedef void (*DgramFunc)(Dgram& d, utils::Buffer data, char *addr, uint16_t port);
struct Dgram {
	Dgram(size_t buf=4096, bool nb=0);
	inline ssize_t send(NetAddr to, string& s) { return send(to,s.data(),s.size()); }
	ssize_t send(NetAddr to, const char *buf, size_t len); bool bind(NetAddr a);
	ssize_t recv(char *buf, size_t size, char **addr, uint16_t *port);
	int onData(NetAddr a, DgramFunc cb); void close();
	int err,sck; private: size_t bs; volatile bool dRun=0;
};

int netStartServer(NetAddr a, int backlog);
Socket netAccept(int srv, bool nb=0);
Socket netConnect(NetAddr a, char nb=0);
void netClose(int s);

}