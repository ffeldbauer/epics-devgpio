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

//! @file devGpioManager.cpp
//! @author F.Feldbauer
//! @date 13 Aug 2015
//! @brief Implementation of GPIO Manager class

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <iterator>
#include <sstream>

// EPICS includes

// local includes
#include "devGpioManager.hpp"
#include "devGpioErrors.hpp"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________

//_____ L O C A L S ____________________________________________________________
static const char ERR_BEGIN[] = "\033[31;1m";
static const char ERR_END[] = "\033[0m\n";

//_____ F U N C T I O N S ______________________________________________________

//------------------------------------------------------------------------------
//! @brief   Standard Constructor
//------------------------------------------------------------------------------
GpioManager::GpioManager() {
}

//------------------------------------------------------------------------------
//! @brief   Standard Destructor
//------------------------------------------------------------------------------
GpioManager::~GpioManager() {
  close( _fdInp );
  close( _fdOut );
}

//------------------------------------------------------------------------------
//! @brief   Register GPIOs to GpioManager
//!
//! @param   [in]  gpio : id of requested gpio
//! @param   [out] flags: Config flags for Gpio
//------------------------------------------------------------------------------
epicsUInt32 GpioManager::registerGpio( epicsUInt32 gpio, epicsUInt64 flags ) {
  int gpiochip = open( "/dev/gpiochip0", 0 );
  if( 0 > gpiochip ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::registerGpio: Could not open GPIO device: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  struct gpio_v2_line_info linfo;
  memset( &linfo, 0, sizeof( linfo ));
  linfo.offset = gpio;

  int rtn = ioctl( gpiochip, GPIO_V2_GET_LINEINFO_IOCTL, &linfo );
  if( -1 == rtn ){
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::registerGpio: Unable to get line info: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  if( linfo.flags & GPIO_V2_LINE_FLAG_USED ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::registerGpio: GPIO "
           << gpio << " already in use" <<ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  close( gpiochip );

  gpio_t newgpio = { gpio, flags };

  epicsUInt32 offset = 0;
  if( flags & GPIO_V2_LINE_FLAG_OUTPUT ){
    _out.push_back( newgpio );
    offset = _out.size() - 1;
  } else {
    _inp.push_back( newgpio );
    offset = _inp.size() - 1;
  }

  return offset;
}

//------------------------------------------------------------------------------
//! @brief   Requests the registered GPIO lines
//!
//! @TODO:   How to handle flags others then INP/OUT?
//------------------------------------------------------------------------------
void GpioManager::request() {

  int fd = open( "/dev/gpiochip0", O_RDONLY );
  if( 0 > fd ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::request: Could not open GPIO device: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  struct gpio_v2_line_request reqinp;
  memset( &reqinp, 0, sizeof( reqinp ));
  strcpy( reqinp.consumer, "devGpio" );
  for( size_t i = 0; i < _inp.size(); ++i )
    reqinp.offsets[i] = _inp.at(i).id;
  reqinp.num_lines    = _inp.size();
  reqinp.config.flags = GPIO_V2_LINE_FLAG_INPUT;

  int rtn = ioctl( fd, GPIO_V2_GET_LINE_IOCTL, &reqinp );
  if( -1 == rtn ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::request: Could not request gpios: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  struct gpio_v2_line_request reqout;
  memset( &reqout, 0, sizeof( reqout ));
  strcpy( reqout.consumer, "devGpio" );
  for( size_t i = 0; i < _out.size(); ++i )
    reqout.offsets[i] = _out.at(i).id;
  reqout.num_lines    = _out.size();
  reqout.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;

  rtn = ioctl( fd, GPIO_V2_GET_LINE_IOCTL, &reqout );
  if( -1 == rtn ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::request: Could not request gpios: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  _fdInp = reqinp.fd;
  _fdOut = reqout.fd;

  close( fd );
}
//------------------------------------------------------------------------------
//! @brief
//------------------------------------------------------------------------------
void GpioManager::setValue( epicsUInt64 mask, epicsUInt32 val ) {
  epicsUInt64 bits = 0;
  epicsUInt64 msk  = mask;
  int n = 0;
  int m = 0;

  while( msk ) {
    if( msk & 1 ) {
      epicsUInt64 buf = ( val & (1<<n) ) >> n;
      bits |= buf << m;
      ++n;
    }
    msk >>= 1;
    ++m;
  }

  struct gpio_v2_line_values values = { bits, mask };
  int ret = ioctl( _fdOut, GPIO_V2_LINE_SET_VALUES_IOCTL, values );
  if( -1 == ret ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setValue: Could not set gpios: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
}

//------------------------------------------------------------------------------
//! @brief
//------------------------------------------------------------------------------
epicsUInt32 GpioManager::getValue( epicsUInt64 mask ){
  struct gpio_v2_line_values values = { 0, mask };
  int ret = ioctl( _fdInp, GPIO_V2_LINE_GET_VALUES_IOCTL, &values );
  if( -1 == ret ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::getValue: Could not read gpios: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  epicsUInt32 val = 0;
  int n = 0;
  while( values.bits ) {
    if( values.mask & 1 ) {
      val |= ( values.bits & 1 ) << n;
      ++n;
    }
    values.bits >>= 1;
    values.mask >>= 1;
  }

  return val;
}

//------------------------------------------------------------------------------
//! @brief
//------------------------------------------------------------------------------
epicsUInt32 GpioManager::event() {
  struct gpio_v2_line_event event;

  int ret = read( _fdInp, &event, sizeof(event));
  if( -1 == ret ) {
    if( errno == -EAGAIN ) {
      return 0xffffffff;
    } else {
      std::stringstream errmsg;
      errmsg << ERR_BEGIN << "GpioManager::events: Failed to read event: "
             << strerror( errno ) << ERR_END;
      throw GpioManagerError( errmsg.str() );
    }
  }
  if( ret != sizeof( event )) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::events: Failed to read event: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  return event.offset;
}

