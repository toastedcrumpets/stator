/*
  Copyright (C) 2017 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

  This file is part of stator.

  stator is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  stator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with stator. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// C++
#include <exception>
#include <sstream>

#define stator_throw()					\
  throw stator::Exception(__LINE__,__FILE__, __func__)

namespace stator {
  /*! \brief An exception class that also appears to function like an
      ostream object.

    This class is thrown using the stator_throw() macro like so :-

    \code{.cpp}
    stator_throw() << "My custom error message";
    \endcode

    An important note, you should always catch the exception classes
    by reference, as this prevents type conversions to base classes
    and maintains the virtual overrides.

    \code{.cpp}
    try {
      //Some code which may throw a stator exception.
    } catch (stator::Exception& ex) {
      //Some response to the exception
    }
    \endcode

    Please note, as the stator::Exception class inherits from
    std::exception, you can catch it using a more generic catch
    statement

    \code{.cpp}
    try {
      //Some code which may throw a stator exception.
    } catch (std::exception& ex) {
      //Some response to the exception
    }
    \endcode

    To allow the combining of multiple exception types, all exceptions
    should inherit virtually from base exception types.
  */
  class Exception : public std::exception {
   public:
    /*! \brief Constructor called by the stator_throw() macro.

      \param line The line number in the source file.
      \param file The name of the source file.
      \param funcname The name of the function throwing the exception.
    */
    Exception(int line, const char* file, const char* funcname) throw() :
      message_(), messageStr_() {

      message_ << "Exception thrown in [" << funcname << "] (" << file <<
         ":" << line << ")" << std::endl;
    }

    /*! \brief Copy constructor.

      Please catch exceptions by reference, they should not be copied
      except at the throw site.
    */
    Exception(const Exception& e) : message_(e.message_.str()), messageStr_() {
    }

    inline ~Exception() throw() {}

    /*! \brief The stream operator engine for the class
     */
    template<class T>
    Exception& operator<<(const T& m) {
      message_ << m;
      return *this;
    }

    /*! \brief Get the stored message from the class.

      This returns a pointer to a traditional C-type string.  This is
      guaranteed to be valid at least until the exception object from
      which it is obtained is destroyed or until a non-const member
      function of the exception object is called.

      This is not reentrant and is annoyingly C-like, but it is in the
      standard.
    */
    const char* what() const throw() {
      messageStr_ = message_.str();
      return messageStr_.c_str();
    }

   private:
    //! \brief Stores the message of the exception.
    std::ostringstream message_;
    //! \brief A temporary store of the what() message.
    mutable std::string messageStr_;
  };
} // namespace stator
