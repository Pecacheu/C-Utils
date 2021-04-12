# C++ Utils
**Utils:** A small library with a number of helpful functions\
**Net:** Small, portable networking library for C++

### Dependencies for utils:
- C++14 & std library (including std::thread)

### Dependencies for net:
- Utils.h
- Unix sys/socket.h (May port to winsock2.h in the future)

## Namespace: utils

### Macros
- `ARCH64` True if 64-bit.
- `WINDOWS` True if Windows.
- `ARM` Version of ARM Arch, 0 if not ARM.
- `static_block` Java-style `static` block.

### Typedefs
- stringmap `unordered_map<string,string>` JS-style array of strings.
- EVLFunc `void func(void *p)` Used for EventLoop.

### Error Help
- `void error(string e)` Print `e` in red.
- `void error(string f, int c)` Print that error `c` occurred in `f`. Also reads standard error codes from global `errno`.
- `bool ckErr(int e, string f)` Convenience function to check for errors in a single line. If `e` is negative, the function prints the error using `error(string f, int c)` and returns true.

### [struct] Buffer
This is arguably the most useful part of the library. These buffers work similar to Node.js buffers, which vastly improves code portability and readability.
- `Buffer()` Create empty buffer.
- `Buffer(const size_t l, bool n=0)` Create buffer of size `l`. If `l` is 0 or `string::npos`, no data will be allocated. Allocation via `new`, delete with `delete[]`. `n` sets the null flag.
- `Buffer(const char *b, const size_t l, bool n=0)` Create buffer from existing data. The original pointer is used. `n` sets the null flag.
- `Buffer(const char *t)` Create buffer from a null-terminated string. The original pointer is used.
- `Buffer(const string& s)` Create buffer from a string. The original pointer is used.
- `string toStr()` Convert buffer to string. The data is copied.
- `string toStr(size_t s, size_t l=0)` Convert buffer to string, starting at `s` with length `l`. The data is copied.
- `const char *toCStr()` Convert buffer to C string. The data is copied. Delete with `delete[]`.
- `const char *toBase64()` Convert buffer to base64 using the `bChar64` table. Padding is stripped. Because padding is stupid.
- `bool match(const char *s)` Check if entire buffer matches a string.
- `bool matchPart(const char *s, size_t ofs=0)` Check for string match in buffer at offset.
- `char& operator[](size_t i)` Read/write index in buffer.
- `void operator=(Buffer& b)` Copy buffer. The original pointer is used.
- `const char *buf` Pointer to buffer.
- `size_t len` Buffer length.
- `bool nul` Null flag. This flag indicates that the buffer ends with a null terminator.

### Buffer Help
- `size_t bFind(Buffer& b, const char *s, size_t ofs=0, size_t end=0)` Find first match of `s` in buffer `b`, starting at `ofs`, and optionally ending at `end`. Can also be used as `indexOf`.
- `vector<Buffer> bSplit(Buffer& b, const char *sp)` Uses `bFind` to split a buffer by separator `sp`. For maximum performance, all Buffers reference the original data.

### C String Help
*Note:* These functions allocate with `new`. Delete with `delete[]`.
- `const char *strCpy(const char *s)` Create copy of C string.
- `const char *strCpy(const char *s, size_t max)` Create copy of C string, up to size `max`.
- `void strCpy(char *d, const char *s, size_t max)` Copy C string to new string, up to size `max`.

### String Help
- `string intToHex(size_t i)` Convert uint to hex.
- `size_t strToUint(string s)` Convert string to uint. Returns `string::npos` if invalid.
- `size_t hexStrToUint(string s)` Convert hex string to uint. Returns `string::npos` if invalid.
- `void replaceAll(string& s, string from, string to)` Replace all instances of `from` with `to` in `s`.
- `bool startsWith(const string& s, const char *t)`\
`bool startsWith(const char *s, const char *t)` True if string `s` starts with `t`.
- `bool endsWith(const string& s, const char *t)`\
`bool endsWith(const char *s, const char *t)` True if string `s` ends with `t`.

### URI Help
- `string toCamelCase(string s)` Convert string to camel case (space or dash separator).
- `string decodeURIComponent(string s)` Decode URI component.
- `stringmap fromQuery(string s)` Extract and decode query data (optionally including leading `?`) from URI.
- `string toQuery(stringmap m)` Convert stringmap to query string, not including leading `?`. Does not encode URI components.

### Time Help
- `uint64_t usTime()` Microsecond time from the high-res clock.
- `uint64_t msTime()` Millisecond time from the high-res clock.
- `string getDate(uint64_t t=0, bool sec=0)` Get user-readable date. Do not use with high-res clock. If `t` is 0, uses `time()` to get current RTC time. `sec` enables seconds.

### JavaScript Event Loop
An extremely useful event loop and scheduling system that runs in a single thread.

*Note:* While the event loop is thread-safe, it uses mutexes internally, so please avoid mutex race conditions (ex. your timer function locks the same mutex as the function that is attempting to set/clear it).

### [class] EventLoop
- `EventLoop(size_t max=UTILS_MAX_TIMERS)` Create EventLoop. `max` sets max scheduled timers.
- `size_t setTimeout(EVLFunc f, uint64_t ms, void *p=0)` Set timeout `f` with optional data pointer `p`. Returns ID, or 0 if queue full.
- `size_t setInterval(EVLFunc f, uint64_t ms, void *p=0)` Set interval `f` with optional data pinter `p`. Returns ID, or 0 if queue full.
- `bool clearTimeout(size_t id)` Clear timeout/interval with ID. Returns true if successful.
- `void run(bool ex=0)` Run EventLoop in current thread. Set `ex` to auto-exit when the queue ends.
- `void stop()` Stop the EventLoop, causing `run` to exit. Fully thread-safe.

`extern EventLoop GlobalEventLoop` Default, global event loop. Run with `GlobalEventLoop.run()`.

### [class] Append
This dynamic class can append several data types, including Buffers, integers, strings, and C strings. Allocation via `new`, delete with `delete[]`.
- `Buffer Append::buf(...)` Append and return as Buffer.
- `const char *Append::str(...)` Append and return as C string.

<br>

## Namespace: net

### Typedefs
- DgramFunc `void func(Dgram& d, Buffer data, char *addr, uint16_t port)` Used for Dgram callback.

### Functions
- `int netStartServer(NetAddr a, int backlog)` Start server, listening at address/port `a`. Backlog is the length of the pending connections queue. A negative number is returned on error.
- `Socket netAccept(int srv, bool nb=0)` Accept a client from the server. `nb` enables non-blocking mode after connection. A Socket with the `err` property set is returned on error.
- `Socket netConnect(NetAddr a, bool nb=0)` Connect to a server at address/port `a`. `nb` enables non-blocking mode after connection. A Socket with the `err` property set is returned on error.
- `void netClose(int s)` Close server.

### [struct] NetAddr
Represents network addresses and ports.
- `NetAddr(uint16_t port=0)` Address defaults to INADDR_ANY.
- `NetAddr(const char *addr, uint16_t port=0)` Currently only IPv4 supported.
- `static NetAddr *resolve(const char *host, uint16_t port=80)` Resolve from a DNS host address. Returns NULL on error.

### [struct] Socket
Represents a socket.
- `ssize_t read(char *buf, size_t size)` Read bytes. Returns data read, or negative on error.
- `ssize_t write(const char *buf, size_t len)` Write bytes. Returns data written, or negative on error.
- `int setTimeout(time_t sec)` Set socket timeout. Returns negative on error.
- `void close()` Close socket.
- `int err` Error, if any.
- `int srv` Server ID.
- `int sck` Socket ID.
- `NetAddr addr` Address of socket.

### [struct] Dgram
Represents a datagram.
- `Dgram(size_t buf=4096, bool nb=0)` Create datagram with receive buffer size `buf`. `nb` enables non-blocking mode. `err` is set on error.
- `send(NetAddr to, const char *buf, size_t len)`\
`send(NetAddr to, string& s)` Send a packet to address/port. Returns data written, or negative on error.
- `ssize_t recv(char *buf, size_t size, char **addr, uint16_t *port)` Receive directly to an external buffer. Returns data read, or negative on error.
- `onData(NetAddr a, DgramFunc cb)` Listen for data from address/port `a` with data callback `cb`. On error, `cb` is called with `d.err` set.
- `void close()` Close listener used by `onData`.
- `int err` Error, if any.