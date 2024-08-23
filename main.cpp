#include <iostream>
#include <string>

#include "PyShell.h"

float string_to_float (std::string& str)
{
    size_t pos;
    float value = stof (str, &pos);
    return value;
}

int main()
{
	std::cout<<"Testing the integrated python interpreter."<<std::endl;
    PyShell py_int;
    std::string out_001, out_002;

    py_int << "print(\"Hello World!\")"; //literally all you have to do the interpreter class will do the rest
    std::getline(py_int, out_001);
    std::cout << "Python Console 001: " << out_001 << std::endl;

    py_int << "print(3.14)";
    py_int << "x = 2";
    py_int << "y = 3";
    py_int << "print('x + y =' + str(x+y))";
    py_int << "print('Henlo Wormld')";
    py_int << "print('i like trains')";

	py_int >> out_002;
    std::cout << "Python Console 001-b: " << out_002 << std::endl;
	float test = string_to_float(out_002);
    std::cout << "Python Console 001-a: " << test << std::endl;
    std::getline(py_int, out_002);
    std::cout << "Python Console 001: " << out_002 << std::endl;
    while(std::getline(py_int, out_001)) //thisgaurenteesanerrorforsomereason.
    {
        std::cout << "Python Console 001: " << out_001 << std::endl;
    }
    py_int.clear();//so clear immediately after flushing the buffer
    //almost makes me want to add a flush method that returns a vector as a safe alternative to this.
    //i did.

    //py_int.safe_flush() returns a vector of strings and clears all errors on end of function.
    py_int << "print(3.145)";
    py_int >> out_002;
    std::cout << "Python Console 001-b: " << out_002 << std::endl;
	float test2 = string_to_float(out_002);
    std::cout << "Python Console 001-a: " << test2 << std::endl;
    std::getline(py_int, out_002);
    std::cout << "Python Console 001: " << out_002 << std::endl;
    py_int << "print(3.1457)";
    std::getline(py_int, out_002);
    std::cout << "Python Console 001: " << out_002 << std::endl; //strange.
    //it doesnt break when getline is used on an empty buffer, only when in a while loop.
    py_int.clear();
    //this doesnt seem to fix the memory access error either

	return 0;
}
