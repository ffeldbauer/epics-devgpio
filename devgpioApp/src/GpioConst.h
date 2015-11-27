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

#ifndef GPIO_CONST_H
#define GPIO_CONST_H

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <map>
#include <string>

// EPICS includes
#include <epicsTypes.h>

// local includes

//_____ D E F I N I T I O N S __________________________________________________

class GpioConst {
 public:
  enum BOARD {
    RASPI_B_REV2,
    RASPI_BP,
    BEAGLEBONE_BLACK
  };

  static bool exists();
  static GpioConst* instance();
  static void create( BOARD board );

  epicsUInt32 findGPIO( std::string const& keyOrName );

 private:
  GpioConst(); // Not implemented
  GpioConst( BOARD board );
  ~GpioConst();
  GpioConst( GpioConst const& rother ); // Not implemented
  GpioConst& operator=( GpioConst const& rother ); // Not implemented

  void init_raspi_b_rev2();
  void init_raspi_bp();
  void init_beagleboneblack();

  static GpioConst *_pinstance;

  BOARD _selection;
  std::map< std::string, epicsUInt32 > _gpioByKey;
  std::map< std::string, epicsUInt32 > _gpioByName;
  std::map< std::string, epicsUInt32 > _gpioByNumber;
};

#endif

