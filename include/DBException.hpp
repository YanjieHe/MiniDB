#ifndef DBEXCEPTION_HPP
#define DBEXCEPTION_HPP
#include <exception>
#include <string>

using std::string;

class DBException : public std::exception {
 public:
  string message;
  explicit DBException(string message) : message{message} {}

  const char *what() const throw() override { return message.c_str(); }
};

#endif  // DBEXCEPTION_HPP