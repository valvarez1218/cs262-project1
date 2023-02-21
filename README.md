# How to use
All subsequent steps require that you first download this repository and open the file containing it in your command line.

## Server
1. In the `./server` directory, run 
```
g++ -std=c++11 -o chatServer.o chatServer.cc
```
2. Run `./chatServer.o`
3. Give the IP address printed to the command line to any clients you want to use your server.

## Client
1. In the `./client` directory, run
```
g++ -std=c++11 -o chatClient.o chatClient.cc
```
2. Run `./chatClient.o`
3. When prompted, input the IP address of the server (the server machine will print this to its command line)
4. For usage directions, run `help`

## Tests
1. Install cmake (any version greater than 3.2)
2. Install c++ (any version greater than 14)
3. Run the following commands in the terminal:

```
cmake -S . -B build
cmake --build build
cd build && ctest
```
4. Logs for tests are available in `./build/testing/LastTest.log`

