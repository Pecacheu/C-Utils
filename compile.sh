#!/bin/bash
set -e; cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
GPP=g++
FLAGS="-pthread -std=c++17 -Wno-psabi -Werror=return-type $3"
mkdir -p build; cd build
echo "Compile Utils"
[[ $1 = "debug" || $2 = "debug" ]] && FLAGS="$FLAGS -g"
if [[ $1 = "shared" ]]; then
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