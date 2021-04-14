set -e
GPP=g++-8
FLAGS="-g -std=c++17 -Wno-psabi -Werror=return-type" #-g = Debug
mkdir -p build; cd build
echo "Compile Utils"
if [[ $1 == "shared" ]]
then
	$GPP -c -fPIC $FLAGS ../utils.cpp ../net.cpp
	echo "Link Shared"
	$GPP utils.o -shared -o libutils.so
	$GPP net.o -shared -o libnet.so
else
	$GPP -c $FLAGS ../utils.cpp ../net.cpp
	echo "Link Static"
	ar rvs libutils.a utils.o
	ar rvs libnet.a net.o
fi