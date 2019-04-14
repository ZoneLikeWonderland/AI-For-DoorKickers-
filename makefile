all: main.exe

main.exe: makefile main.o logic.h logic.o const.h avoid.cpp waypoint.cpp playerAI.h playerAI.cpp  jsoncpp/json/json-forwards.h jsoncpp/json/json.h jsoncpp/jsoncpp.cpp geometry.o geometry.h 
ifeq ($(OS),Windows_NT)
	g++ main.o logic.o avoid.cpp waypoint.cpp playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.exe -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -lwsock32 -std=c++11 -O2
else
	g++ main.o logic.o avoid.cpp waypoint.cpp playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.exe -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -pthread -O2
endif

logic.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h logic.cpp
	g++ -c logic.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -O2

geometry.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h geometry.cpp
	g++ -c geometry.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -O2

main.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h main.cpp
	g++ -c main.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

.PHONY: clean
clean:
	-rm -r *.exe *.o
	-del -r *.exe *.o
