set -e
#Debug: -g flag
GPP=g++-8
FLAGS="-g -fPIC -std=c++17 -Wno-psabi -Werror=return-type"
mkdir -p build; cd build
echo "Compile Utils"
$GPP -c $FLAGS ../utils.cpp ../net.cpp
echo "Link"
$GPP utils.o -shared -o libutils.so
$GPP net.o -shared -o libnet.so