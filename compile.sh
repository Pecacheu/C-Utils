#Debug: -g flag
g++-8 -g -fPIC -shared -std=c++17 -Wno-psabi -Werror=return-type utils.cpp net.cpp -o utils.so