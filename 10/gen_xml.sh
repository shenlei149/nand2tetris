g++ --std=c++17 -g compiler.cc -o compiler

rm ExpressionLessSquare/MainT.xml.g
rm ExpressionLessSquare/SquareT.xml.g
rm ExpressionLessSquare/SquareGameT.xml.g
rm ExpressionLessSquare/Main.xml.g
rm ExpressionLessSquare/Square.xml.g
rm ExpressionLessSquare/SquareGame.xml.g
rm Square/MainT.xml.g
rm Square/SquareT.xml.g
rm Square/SquareGameT.xml.g
rm Square/Main.xml.g
rm Square/Square.xml.g
rm Square/SquareGame.xml.g
rm ArrayTest/MainT.xml.g
rm ArrayTest/Main.xml.g

./compiler ExpressionLessSquare/
./compiler Square/
./compiler ArrayTest/
