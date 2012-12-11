g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"VZController.d" -MT"VZController.d" -o "VZController.o" "VZController.cpp" -fpermissive
g++  -o "VZController"  ./VZController.o   -lpthread
