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

#ifndef DEV_GPIO_MANAGER_H
#define DEV_GPIO_MANAGER_H

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <map>
#include <string>

// EPICS includes
#include <epicsTypes.h>

// local includes

//_____ D E F I N I T I O N S __________________________________________________

class GpioManager {
 public:
  enum DIRECTION {
    INPUT = 0,
    OUTPUT = 1,
    UNDIFIEND = 2
  };

// Definition of PIN_VALUE depends on
// Logic...therefore this is not usefull
//  enum PIN_VALUE {
//    LOW = 0,
//    HIGH = 1
//  };

  enum EDGE_VALUE {
    NONE = 0,
    RISING = 1,
    FALLING = 2,
    BOTH = 3
  };

  enum LOGIC_VALUE {
    ACTIVE_HIGH = 0,
    ACTIVE_LOW = 1
  };

  void exportPin( epicsUInt32 gpio );
  void unexportPin( epicsUInt32 gpio );

  void setDirection( epicsUInt32 gpio, DIRECTION dir );
  DIRECTION getDirection( epicsUInt32 gpio );

  void setValue( epicsUInt32 gpio, epicsUInt32 val );
  epicsUInt32 getValue( epicsUInt32 gpio );

  void setEdge( epicsUInt32 gpio, EDGE_VALUE edge );
  EDGE_VALUE getEdge( epicsUInt32 gpio );

  void setLogic( epicsUInt32 gpio, LOGIC_VALUE logic  );
  LOGIC_VALUE getLogic( epicsUInt32 gpio );

  void waitForUdev( epicsUInt32 gpio );

  static GpioManager& instance() {
    static GpioManager rinstance;
    return rinstance;
  }

 private:
  struct GPIO {
    bool exported;
    LOGIC_VALUE logic;
    DIRECTION dir;
  };

  GpioManager();
  ~GpioManager();
  GpioManager( GpioManager const& rother ); // Not implemented
  GpioManager& operator=( GpioManager const& rother ); // Not implemented

  std::map< epicsUInt32, GPIO > _mgpio;
  std::map< EDGE_VALUE, std::string > _mEdgeToString;
  std::map< std::string, EDGE_VALUE > _mStringToEdge;
  std::string _gpiobase;
};

#endif

