#ifndef PyShell_H
#define PyShell_H

#include <boost/python.hpp>
#include <streambuf>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

class PyShell : public std::streambuf, public std::iostream
{
public:
    PyShell();
    ~PyShell();

    void execute_file(const std::string &file_path);
    std::vector<std::string> safe_flush();

    bool running();

protected:
    // Override functions to handle the custom input/output
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int overflow(int c = EOF) override;
    int underflow() override;

private:
    boost::python::object main_module;
    boost::python::object main_namespace;
    boost::python::object string_io;
    boost::python::object sys_module;

    std::mutex interpreterMutex;

    std::string input_buffer;
    std::string output_buffer;
};

#endif // PyShell_H
