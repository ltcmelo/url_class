/*****************************************************************************
 The MIT License

 Copyright (c) since 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*****************************************************************************/

#ifndef URL_SYNTAX_EXCEPTION_HPP
#define URL_SYNTAX_EXCEPTION_HPP

#include "config.hpp"
#include <exception>

BUNDLE_NAMESPACE_BEGIN

/*
 * The error message is stored as a char pointer. Beware of the supplied strings life-time.
 * String literals and any other static string fit this requirement. This class does not embed
 * a std::string for safety (details at: http://www.boost.org/community/error_handling.html).
 *
 */


class UrlSyntaxException : public std::exception
{
private:
  char const* error_msg_;

public:
  UrlSyntaxException(char const* msg):error_msg_(msg){}
  virtual ~UrlSyntaxException() throw() {}

  virtual char const* what() const throw() { return get_error_msg(); }

  char const* get_error_msg() const throw() { return error_msg_; }
};

NAMESPACE_END

#endif //URL_SYNTAX_EXCEPTION_HPP
