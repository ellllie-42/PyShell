#include "PyShell.h"
#include <boost/python.hpp>
#include <boost/python/import.hpp>
#include <fstream>
#include <iostream>

PyShell::PyShell()
    : std::iostream(this) // Initialize the iostream with the streambuf
{
    // Initialize the Python interpreter
    Py_Initialize();

    // Import the __main__ module and get its namespace
    main_module = boost::python::import("__main__");
    main_namespace = main_module.attr("__dict__");

    // Set up the StringIO module for capturing output
    string_io = boost::python::import("io").attr("StringIO")();
    sys_module = boost::python::import("sys");

    // Redirect stdout and stderr to StringIO
    sys_module.attr("stdout") = string_io;
    sys_module.attr("stderr") = string_io;
}

PyShell::~PyShell() {
    // Finalize the Python interpreter
    *this << "exit()"; //apparently this fixes the memory access error. neat.
    Py_Finalize();
}

void PyShell::execute_file(const std::string &file_path) {
    std::lock_guard<std::mutex> guard(interpreterMutex);

    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    std::string script((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        boost::python::exec(script.c_str(), main_namespace);
    } catch (boost::python::error_already_set const &) {
        PyErr_Print();
    }
}

bool PyShell::running() {
    // In this context, "running" can just check if Python is initialized
    return Py_IsInitialized();
}

std::streamsize PyShell::xsputn(const char* s, std::streamsize n) {
    std::lock_guard<std::mutex> guard(interpreterMutex);
    input_buffer.append(s, n);

    try {
        boost::python::exec(input_buffer.c_str(), main_namespace);
        input_buffer.clear();
    } catch (boost::python::error_already_set const &) {
        PyErr_Print();
    }

    return n;
}

int PyShell::overflow(int c) {
    if (c != EOF) {
        char z = c;
        return xsputn(&z, 1);
    }
    return c;
}

int PyShell::underflow() {
    std::lock_guard<std::mutex> guard(interpreterMutex);

    // Get the output from StringIO
    output_buffer = boost::python::extract<std::string>(string_io.attr("getvalue")());

    // Clear the StringIO buffer
    string_io.attr("seek")(0);
    string_io.attr("truncate")(0);

    if (output_buffer.empty()) {
        return EOF;
    }

    setg(&output_buffer[0], &output_buffer[0], &output_buffer[0] + output_buffer.size());
    return std::char_traits<char>::to_int_type(*gptr()); // Use std::char_traits<char> for traits_type

}

std::vector<std::string> PyShell::safe_flush()
{
    std::vector<std::string> out;
    std::string temp;
    std::getline(*this,temp);
    do
    {
        out.push_back(temp);
    }
    while(std::getline(*this,temp));
    this->clear();
    return out;
}
