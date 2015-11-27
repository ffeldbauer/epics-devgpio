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
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

// EPICS includes

// local includes
#include "devGpioManager.h"
#include "devGpioErrors.h"

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
  _gpiobase = "/sys/class/gpio/gpio";
  _mgpio.clear();

  _mEdgeToString.insert( std::make_pair( NONE,    "none" ) );
  _mEdgeToString.insert( std::make_pair( RISING,  "rising" ) );
  _mEdgeToString.insert( std::make_pair( FALLING, "falling" ) );
  _mEdgeToString.insert( std::make_pair( BOTH,    "both" ) );

  _mStringToEdge.insert( std::make_pair( "none",    NONE ) );
  _mStringToEdge.insert( std::make_pair( "rising",  RISING ) );
  _mStringToEdge.insert( std::make_pair( "falling", FALLING ) );
  _mStringToEdge.insert( std::make_pair( "both",    BOTH ) );
}

//------------------------------------------------------------------------------
//! @brief   Standard Destructor
//------------------------------------------------------------------------------
GpioManager::~GpioManager() {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.begin();
  for( ; it != _mgpio.end(); ++it ) {
    unexportPin( it->first );
  }
  _mgpio.clear();
}

//------------------------------------------------------------------------------
//! @brief   Export GPIO
//------------------------------------------------------------------------------
void GpioManager::exportPin( epicsUInt32 gpio ) {
  static std::string _exportFile = "/sys/class/gpio/export";

  // Each GPIO should only be handled by a single record
  // If GPIO is already exported by GpioManager throw an exception
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it != _mgpio.end() ){
    if ( it->second.exported ) {
      std::stringstream errmsg;
      errmsg << ERR_BEGIN << "GpioManager::exportPin: Error: GPIO " << gpio << " already exported" << ERR_END;
      throw GpioManagerError( errmsg.str() );
    }
  }

  // Check if Pin was exported by another process
  std::stringstream gpioDir;
  gpioDir << _gpiobase << gpio << "/direction";
  if( access( gpioDir.str().c_str(), F_OK ) == 0 ) {
    GPIO nfo;
    nfo.exported = true;
    _mgpio.insert( std::make_pair( gpio, nfo ) );
    nfo.logic    = getLogic( gpio );
    nfo.dir      = getDirection( gpio );

    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::exportPin: Warning: GPIO " << gpio << " already exported! Might be used by another process!" << ERR_END;
    throw GpioManagerWarning( errmsg.str() );
  }

  std::fstream exportFs( _exportFile.c_str(), std::fstream::out );

  if( !exportFs.is_open() || !exportFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::exportPin: Could not open export file: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  exportFs << gpio;
  exportFs.flush();
  if( exportFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::exportPin: Could not export pin "
           << gpio << ": "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  exportFs.close();

  GPIO nfo = { true, ACTIVE_HIGH, UNDIFIEND };
  _mgpio.insert( std::make_pair( gpio, nfo ) );
}

//------------------------------------------------------------------------------
//! @brief   Unexport GPIO
//------------------------------------------------------------------------------
void GpioManager::unexportPin( epicsUInt32 gpio ) {
  static std::string _exportFile = "/sys/class/gpio/unexport";

  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::unexportPin: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    return; // Nothing to do
  }

  std::fstream exportFs( _exportFile.c_str(), std::fstream::out );
  if( !exportFs.is_open() || !exportFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::unexportPin: Could not open unexport file: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  exportFs << it->first;
  exportFs.flush();
  if( exportFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::unexportPin: Could not write to file: "
           << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  exportFs.close();

  it->second.exported = false;

}

//------------------------------------------------------------------------------
//! @brief   Set Direction of GPIO
//------------------------------------------------------------------------------
void GpioManager::setDirection( epicsUInt32 gpio, DIRECTION dir ) {

  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::setDirection: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::setDirection: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/direction";

  std::fstream dirFs( filename.str().c_str(), std::fstream::out );
  if( !dirFs.is_open() || !dirFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setDirection: Could not open direction file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  if( OUTPUT == dir ) dirFs << "out";
  else                dirFs << "in";
  dirFs.flush();
  if( dirFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setDirection: Could not write to direction file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  dirFs.close();
  
  it->second.dir = dir;
}

//------------------------------------------------------------------------------
//! @brief   Get Direction of GPIO
//------------------------------------------------------------------------------
GpioManager::DIRECTION GpioManager::getDirection( epicsUInt32 gpio ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::getDirection: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::getDirection: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  // Assuming we are the only ones managing this GPIO pin....
  // if( it->second.dir != UNDIFIEND ) return it->second.dir;

  std::stringstream filename;
  filename << _gpiobase << gpio << "/direction";

  std::fstream dirFs( filename.str().c_str(), std::fstream::in );
  if( !dirFs.is_open() || !dirFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::getDirection: Could not open direction file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  std::string direction;
  dirFs >> direction;
  dirFs.close();

  if( direction.compare( "out" ) == 0 ) {
    it->second.dir = OUTPUT;
    return OUTPUT;
  }
  
  it->second.dir = INPUT;
  return INPUT;

}

//------------------------------------------------------------------------------
//! @brief   Set Value of GPIO
//------------------------------------------------------------------------------
void GpioManager::setValue( epicsUInt32 gpio, epicsUInt32 val ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::setValue: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::setValue: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

//  if( it->second.dir != OUTPUT ) {
//    std::stringstream errmsg;
//    errmsg << "GpioManager::setValue: Error: GPIO " << gpio << " is not configured as output.";
//    throw GpioManagerError( errmsg.str() );
//  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/value";

  std::fstream valFs( filename.str().c_str(), std::fstream::out );
  if( !valFs.is_open() || !valFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setValue: Could not open value file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  valFs << val;
  valFs.flush();
  if( valFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setValue: Could not write to value file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  valFs.close();
}

//------------------------------------------------------------------------------
//! @brief   Get Value of GPIO
//------------------------------------------------------------------------------
epicsUInt32 GpioManager::getValue( epicsUInt32 gpio ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::getValue: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::getValue: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/value";

  std::fstream valFs( filename.str().c_str(), std::fstream::in );
  if( !valFs.is_open() || !valFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::getValue: Could not open value file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  epicsUInt32 value = 0;
  valFs >> value;
  valFs.close();

  return value;
//  if( 1 == value ) {
//    return HIGH;
//  }
//  return LOW;
}

//------------------------------------------------------------------------------
//! @brief   Set Edge of GPIO
//------------------------------------------------------------------------------
void GpioManager::setEdge( epicsUInt32 gpio, EDGE_VALUE edge ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::setEdge: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::setEdge: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  if( it->second.dir != INPUT ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::setEdge: Error: GPIO " << gpio << " is not configured as input.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/edge";

  std::fstream valFs( filename.str().c_str(), std::fstream::out );
  if( !valFs.is_open() || !valFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setEdge: Could not open edge file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

//  switch( edge ) {
//    case NONE:    valFs << "none";    break;
//    case RISING:  valFs << "rising";  break;
//    case FALLING: valFs << "falling"; break;
//    case BOTH:    valFs << "both";    break;
//  }
  valFs << _mEdgeToString[ edge ];
  valFs.flush();
  if( valFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setEdge: Could not write to edge file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  valFs.close();
}

//------------------------------------------------------------------------------
//! @brief   Get Edge of GPIO
//------------------------------------------------------------------------------
GpioManager::EDGE_VALUE GpioManager::getEdge( epicsUInt32 gpio ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::getEdge: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::getEdge: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/edfe";

  std::fstream valFs( filename.str().c_str(), std::fstream::in );
  if( !valFs.is_open() || !valFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::getEdge: Could not open edge file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  std::string edge;
  valFs >> edge;
  valFs.close();

//  if( edge.compare( "none" ) == 0 )    return NONE;
//  if( edge.compare( "rising" ) == 0 )  return RISING;
//  if( edge.compare( "falling" ) == 0 ) return FALLING;
//  if( edge.compare( "both" ) == 0 )    return BOTH;
  
  return _mStringToEdge[ edge ];
}

//------------------------------------------------------------------------------
//! @brief   Set Logic of GPIO
//------------------------------------------------------------------------------
void GpioManager::setLogic( epicsUInt32 gpio, LOGIC_VALUE logic ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::setLogic: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::setLogic: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/active_low";

  std::fstream logicFs( filename.str().c_str(), std::fstream::out );
  if( !logicFs.is_open() || !logicFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setLogic: Could not open file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  logicFs << logic;
  logicFs.flush();
  if( logicFs.bad() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::setLogic: Could not write to file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

  it->second.logic = logic;

  logicFs.close();
}

//------------------------------------------------------------------------------
//! @brief   Get Logic of GPIO
//------------------------------------------------------------------------------
GpioManager::LOGIC_VALUE GpioManager::getLogic( epicsUInt32 gpio ) {
  std::map< epicsUInt32, GPIO >::iterator it = _mgpio.find( gpio );
  if( it == _mgpio.end() ){
    std::stringstream errmsg;
    errmsg << "GpioManager::getLogic: Error: GPIO " << gpio << " not managed.";
    throw GpioManagerError( errmsg.str() );
  } else if ( !it->second.exported ) {
    std::stringstream errmsg;
    errmsg << "GpioManager::getLogic: Error: GPIO " << gpio << " not exported.";
    throw GpioManagerError( errmsg.str() );
  }

  std::stringstream filename;
  filename << _gpiobase << gpio << "/active_low";

  std::fstream logicFs( filename.str().c_str(), std::fstream::in );
  if( !logicFs.is_open() || !logicFs.good() ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN << "GpioManager::getLogic: Could not open file '"
           << filename.str() << "': " << strerror( errno ) << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }
  epicsUInt32 value = 0;
  logicFs >> value;
  logicFs.close();

  if( 1 == value ) {
    it->second.logic = ACTIVE_LOW;
    return ACTIVE_LOW;
  }

  it->second.logic = ACTIVE_HIGH;
  return ACTIVE_HIGH;
}

//------------------------------------------------------------------------------
//! @brief   Wait for write permissions
//!
//! On BeagleBone Black write permissions to gpios are granted via udev rule.
//! Udev rule needs about 25 ms to set permissions for the files belonging to
//! on GPIO.
//------------------------------------------------------------------------------
void GpioManager::waitForUdev( epicsUInt32 gpio ) {
  static const epicsUInt32 MAX_TRIES = 100;

  epicsUInt32 ntries = 0;

  std::stringstream filename;
  filename << _gpiobase << gpio << "/direction";
  
  int accessOK = access( filename.str().c_str(), R_OK | W_OK );
  while( accessOK < 0 && ntries < MAX_TRIES ) {
    accessOK = access( filename.str().c_str(), R_OK | W_OK );
    ++ntries;
    usleep( 500 );
  }
  if( ntries >= MAX_TRIES ) {
    std::stringstream errmsg;
    errmsg << ERR_BEGIN
           << "GpioManager::waitForUdev: Cannot access gpio "
           << gpio
           << " after "
           << MAX_TRIES
           << " tries: "
           << strerror( errno )
           << ERR_END;
    throw GpioManagerError( errmsg.str() );
  }

}

