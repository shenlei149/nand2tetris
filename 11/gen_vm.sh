g++ --std=c++17 -g compiler.cc -o compiler

rm Seven/Main.vm.g
rm ConvertToBin/Main.vm.g
rm Square/Square.vm.g
rm Square/SquareGame.vm.g
rm Square/Main.vm.g
rm Average/Main.vm.g
rm Pong/Ball.vm.g
rm Pong/Bat.vm.g
rm Pong/Main.vm.g
rm Pong/PongGame.vm.g
rm ComplexArrays/Main.vm.g

./compiler Seven/
./compiler ConvertToBin/
./compiler Square/
./compiler Average/
./compiler Pong/
./compiler ComplexArrays/
