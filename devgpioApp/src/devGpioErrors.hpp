//******************************************************************************
// Copyright (C) 2015 Florian Feldbauer <feldbaue@kph.uni-mainz.de>
//                    - Helmholtz-Institut Mainz
//
// This file is part of devGpio
//
// devGpio is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// devGpio is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// version 1.0.0; Aug 13, 2015
//
//******************************************************************************

#ifndef DEV_GPIO_ERRORS_H
#define DEV_GPIO_ERRORS_H

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <exception>
#include <string>

//_____ D E F I N I T I O N S __________________________________________________

//----------------------------------------------------------------------------
//! @brief   Abstract base class of all exception types within the devGpio
//----------------------------------------------------------------------------
class DevGpioException : public std::exception {
 public:
  DevGpioException(const DevGpioException& e) throw() :
    std::exception(e),
    what_(e.what_)
  {}

  DevGpioException& operator=(const DevGpioException& rhs) throw() {
    what_ = rhs.what_;
    return *this;
  }
  ~DevGpioException() throw() { }


  //--------------------------------------------------------------------------
  //! @brief   Get error message
  //--------------------------------------------------------------------------
  virtual const char* what() const throw() {
    return what_.c_str();
  }

 protected:
  DevGpioException(const char* w = "") throw() :
    what_(w)
  {}

  DevGpioException(const std::string& w) throw() :
    what_(w)
  {}

  std::string what_;
};

//----------------------------------------------------------------------------
//! @brief   Exception for minor errors within devGpioManager
//----------------------------------------------------------------------------
class GpioManagerWarning : public DevGpioException {
 public:
  GpioManagerWarning ( const std::string& error ) :
    DevGpioException(error)
  {}
  GpioManagerWarning ( const char *error ) :
    DevGpioException(error)
  {}

  ~GpioManagerWarning () throw() {}
};

//----------------------------------------------------------------------------
//! @brief   Exception for major errors within devGpioManager
//----------------------------------------------------------------------------
class GpioManagerError : public DevGpioException {
 public:
  GpioManagerError ( const std::string& error ) :
    DevGpioException(error)
  {}
  GpioManagerError ( const char *error ) :
    DevGpioException(error)
  {}

  ~GpioManagerError () throw() {}
};

//-----------------------------------------------------------------------------
//! @brief   Exception for major errors within GpioConst
//----------------------------------------------------------------------------
class GpioConstError : public DevGpioException {
 public:
  GpioConstError ( const std::string& error ) :
    DevGpioException(error)
  {}
  GpioConstError ( const char *error ) :
    DevGpioException(error)
  {}

  ~GpioConstError () throw() {}
};

#endif
