#include "python_interpreter.h"
#include <boost/python.hpp>
#include <boost/python/import.hpp>
#include <fstream>
#include <iostream>

python_interpreter::python_interpreter()
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

python_interpreter::~python_interpreter() {
    // Finalize the Python interpreter
    Py_Finalize();
}

void python_interpreter::execute_file(const std::string &file_path) {
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

bool python_interpreter::running() {
    // In this context, "running" can just check if Python is initialized
    return Py_IsInitialized();
}

std::streamsize python_interpreter::xsputn(const char* s, std::streamsize n) {
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

int python_interpreter::overflow(int c) {
    if (c != EOF) {
        char z = c;
        return xsputn(&z, 1);
    }
    return c;
}

int python_interpreter::underflow() {
    std::lock_guard<std::mutex> guard(interpreterMutex);

    // Flush the StringIO buffer
    string_io.attr("flush")(); // Ensure StringIO is flushed before reading

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
