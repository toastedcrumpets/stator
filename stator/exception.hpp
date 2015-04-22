/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <exception>
#include <sstream>

#define stator_throw()					\
  throw stator::Exception(__LINE__,__FILE__, __func__)

namespace stator
{
  /*! \brief An exception class that works like a stream object.
   
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
  class Exception : public virtual std::exception
  {
  public:
    
    inline ~exception() throw() {}
    
    /*! \brief Constructor called by the stator_throw() macro.
     
      \param line The line number in the source file.
      \param file The name of the source file.
      \param funcname The name of the function throwing the exception.
    */
    exception(int line, const char* file, const char* funcname) throw()
    {
      _message << "Exception thrown in [" << funcname
	       << "] (" << file << ":" << line << ")\n";
    }

    /*! \brief Copy constructor.  

      Please catch exceptions by reference, they should not be copied
      except at the throw site.
    */
    exception(const exception&e)
    { _message << e._message.str(); }


    /*! \brief The stream operator engine for the class
     */
    template<class T>
    exception& operator<<(const T& m) 
    { 
      _message << m;
      return *this;
    }
    
    /*! \brief Get the stored message from the class.
      
      This returns a pointer to a traditional c-type string.  This is
      guaranteed to be valid at least until the exception object from
      which it is obtained is destroyed or until a non-const member
      function of the exception object is called.

      This is not reentrant and is annoyingly C-like, but it is in the
      standard.
    */
    const char* what() const throw()
    {
      _whatval = _message.str();
      return _whatval.c_str();
    }

  private:
    //! \brief Stores the message of the exception.
    std::ostringstream _message;
    //! \brief A temporary store of the what() message.
    mutable std::string _whatval;
  }; 
}

