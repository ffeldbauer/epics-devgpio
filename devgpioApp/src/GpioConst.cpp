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

//! @file GpioConst.cpp
//! @author F.Feldbauer
//! @date 13 Aug 2015
//! @brief Implementation look-up table get GPIOs by name or key

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

// EPICS includes

// local includes
#include "GpioConst.h"
#include "devGpioErrors.h"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________
GpioConst *GpioConst::_pinstance = NULL;

//_____ L O C A L S ____________________________________________________________
static const char ERR_BEGIN[] = "\033[31;1m";
static const char ERR_END[] = "\033[0m\n";

//_____ F U N C T I O N S ______________________________________________________

//------------------------------------------------------------------------------
//! @brief   Standard Constructor
//------------------------------------------------------------------------------
GpioConst::GpioConst( BOARD board )
 : _selection( board )
{
  switch( board ) {
    case RASPI_B_REV2:     init_raspi_b_rev2();    break;
    case RASPI_BP:         init_raspi_bp();        break;
    case BEAGLEBONE_BLACK: init_beagleboneblack(); break;
  }
}

//------------------------------------------------------------------------------
//! @brief   Standard Destructor
//------------------------------------------------------------------------------
GpioConst::~GpioConst(){
  _gpioByKey.clear();
  _gpioByName.clear();
  _gpioByNumber.clear();
}

//------------------------------------------------------------------------------
//! @brief   Check if instance already exists
//------------------------------------------------------------------------------
bool GpioConst::exists(){
  if( _pinstance ) return true;
  return false;
}

//------------------------------------------------------------------------------
//! @brief   Get Instance
//------------------------------------------------------------------------------
GpioConst* GpioConst::instance(){
  if( !_pinstance ) throw GpioConstError( "GpioConst::instance: No instance was created" );
  return _pinstance;
}

//------------------------------------------------------------------------------
//! @brief   Create instance
//------------------------------------------------------------------------------
void GpioConst::create( BOARD board ) {
  if( _pinstance ) {
    std::cerr << "GpioConst::create: Instance already created" << std::endl;
    return;
  }
  _pinstance = new GpioConst( board );
}

//------------------------------------------------------------------------------
//! @brief   Get GPIO number from name or key
//------------------------------------------------------------------------------
epicsUInt32 GpioConst::findGPIO( std::string const& keyOrName ) {
  std::map< std::string, epicsUInt32 >::const_iterator it;

  // first try to find GPIO by key
  it = _gpioByKey.find( keyOrName );
  if( it != _gpioByKey.end() ) return it->second;

  // if not found try to find by name
  it = _gpioByName.find( keyOrName );
  if( it != _gpioByName.end() ) return it->second;

  // if still not found, interpret keyOrName as GPIO number
  it = _gpioByNumber.find( keyOrName );
  if( it != _gpioByNumber.end() ) return it->second;

  // if we end here, keyOrName is invalid
  std::stringstream errmsg;
  errmsg << ERR_BEGIN
         << "GpioConst::findGPIO: Error: Cannot find GPIO " << keyOrName
         << ERR_END;
  throw GpioConstError( errmsg.str() );

}

//------------------------------------------------------------------------------
//! @brief   Init maps for Raspberry Pi B Revision 2
//------------------------------------------------------------------------------
void GpioConst::init_raspi_b_rev2() {
  std::cout << "GpioConst: Loading Look-Up table for Raspberry Pi B Rev2" << std::endl;
  // Key list
  _gpioByKey.insert( std::make_pair( "P1-08", 14 ) );
  _gpioByKey.insert( std::make_pair( "P1-10", 15 ) );
  _gpioByKey.insert( std::make_pair( "P1-12", 18 ) );
  _gpioByKey.insert( std::make_pair( "P1-16", 23 ) );
  _gpioByKey.insert( std::make_pair( "P1-18", 24 ) );
  _gpioByKey.insert( std::make_pair( "P1-22", 25 ) );
  _gpioByKey.insert( std::make_pair( "P1-24", 8 ) );
  _gpioByKey.insert( std::make_pair( "P1-26", 7 ) );
  _gpioByKey.insert( std::make_pair( "P1-03", 2 ) );
  _gpioByKey.insert( std::make_pair( "P1-05", 3 ) );
  _gpioByKey.insert( std::make_pair( "P1-07", 4 ) );
  _gpioByKey.insert( std::make_pair( "P1-11", 17 ) );
  _gpioByKey.insert( std::make_pair( "P1-13", 27 ) );
  _gpioByKey.insert( std::make_pair( "P1-15", 22 ) );
  _gpioByKey.insert( std::make_pair( "P1-19", 10 ) );
  _gpioByKey.insert( std::make_pair( "P1-21", 9 ) );
  _gpioByKey.insert( std::make_pair( "P1-23", 11 ) );
  _gpioByKey.insert( std::make_pair( "P5-03", 28 ) );
  _gpioByKey.insert( std::make_pair( "P5-05", 30 ) );
  _gpioByKey.insert( std::make_pair( "P5-04", 29 ) );
  _gpioByKey.insert( std::make_pair( "P5-06", 31 ) );

  // Name list
  _gpioByName.insert( std::make_pair( "GPIO14", 14 ) );
  _gpioByName.insert( std::make_pair( "GPIO15", 15 ) );
  _gpioByName.insert( std::make_pair( "GPIO18", 18 ) );
  _gpioByName.insert( std::make_pair( "GPIO23", 23 ) );
  _gpioByName.insert( std::make_pair( "GPIO24", 24  ) );
  _gpioByName.insert( std::make_pair( "GPIO25", 25 ) );
  _gpioByName.insert( std::make_pair( "GPIO08", 8 ) );
  _gpioByName.insert( std::make_pair( "GPIO07", 7 ) );
  _gpioByName.insert( std::make_pair( "GPIO2", 2  ) );
  _gpioByName.insert( std::make_pair( "GPIO3", 3 ) );
  _gpioByName.insert( std::make_pair( "GPIO4", 4 ) );
  _gpioByName.insert( std::make_pair( "GPIO17", 17 ) );
  _gpioByName.insert( std::make_pair( "GPIO27", 27 ) );
  _gpioByName.insert( std::make_pair( "GPIO22", 22 ) );
  _gpioByName.insert( std::make_pair( "GPIO10", 10 ) );
  _gpioByName.insert( std::make_pair( "GPIO9", 9 ) );
  _gpioByName.insert( std::make_pair( "GPIO11", 11 ) );
  _gpioByName.insert( std::make_pair( "GPIO28", 28 ) );
  _gpioByName.insert( std::make_pair( "GPIO30", 30 ) );
  _gpioByName.insert( std::make_pair( "GPIO29", 29 ) );
  _gpioByName.insert( std::make_pair( "GPIO31", 31 ) );

  // Number list
  _gpioByNumber.insert( std::make_pair( "14", 14 ) );
  _gpioByNumber.insert( std::make_pair( "15", 15 ) );
  _gpioByNumber.insert( std::make_pair( "18", 18 ) );
  _gpioByNumber.insert( std::make_pair( "23", 23 ) );
  _gpioByNumber.insert( std::make_pair( "24", 24  ) );
  _gpioByNumber.insert( std::make_pair( "25", 25 ) );
  _gpioByNumber.insert( std::make_pair( "08", 8 ) );
  _gpioByNumber.insert( std::make_pair( "07", 7 ) );
  _gpioByNumber.insert( std::make_pair( "2", 2  ) );
  _gpioByNumber.insert( std::make_pair( "3", 3 ) );
  _gpioByNumber.insert( std::make_pair( "4", 4 ) );
  _gpioByNumber.insert( std::make_pair( "17", 17 ) );
  _gpioByNumber.insert( std::make_pair( "27", 27 ) );
  _gpioByNumber.insert( std::make_pair( "22", 22 ) );
  _gpioByNumber.insert( std::make_pair( "10", 10 ) );
  _gpioByNumber.insert( std::make_pair( "9", 9 ) );
  _gpioByNumber.insert( std::make_pair( "11", 11 ) );
  _gpioByNumber.insert( std::make_pair( "28", 28 ) );
  _gpioByNumber.insert( std::make_pair( "30", 30 ) );
  _gpioByNumber.insert( std::make_pair( "29", 29 ) );
  _gpioByNumber.insert( std::make_pair( "31", 31 ) );

}

//------------------------------------------------------------------------------
//! @brief   Init maps for Raspberry Pi B+ and Raspberry Pi 2 B
//------------------------------------------------------------------------------
void GpioConst::init_raspi_bp() {
  std::cout << "GpioConst: Loading Look-Up table for Raspberry Pi B+" << std::endl;

  // Key list
  _gpioByKey.insert( std::make_pair( "J8-08", 14 ) );
  _gpioByKey.insert( std::make_pair( "J8-10", 15 ) );
  _gpioByKey.insert( std::make_pair( "J8-12", 18 ) );
  _gpioByKey.insert( std::make_pair( "J8-16", 23 ) );
  _gpioByKey.insert( std::make_pair( "J8-18", 24 ) );
  _gpioByKey.insert( std::make_pair( "J8-22", 25 ) );
  _gpioByKey.insert( std::make_pair( "J8-24", 8 ) );
  _gpioByKey.insert( std::make_pair( "J8-26", 7 ) );
  _gpioByKey.insert( std::make_pair( "J8-03", 2 ) );
  _gpioByKey.insert( std::make_pair( "J8-05", 3 ) );
  _gpioByKey.insert( std::make_pair( "J8-07", 4 ) );
  _gpioByKey.insert( std::make_pair( "J8-11", 17 ) );
  _gpioByKey.insert( std::make_pair( "J8-13", 27 ) );
  _gpioByKey.insert( std::make_pair( "J8-15", 22 ) );
  _gpioByKey.insert( std::make_pair( "J8-19", 10 ) );
  _gpioByKey.insert( std::make_pair( "J8-21", 9 ) );
  _gpioByKey.insert( std::make_pair( "J8-23", 11 ) );
  _gpioByKey.insert( std::make_pair( "J8-27", 0 ) );
  _gpioByKey.insert( std::make_pair( "J8-28", 1 ) );
  _gpioByKey.insert( std::make_pair( "J8-29", 5 ) );
  _gpioByKey.insert( std::make_pair( "J8-31", 6 ) );
  _gpioByKey.insert( std::make_pair( "J8-32", 12 ) );
  _gpioByKey.insert( std::make_pair( "J8-33", 13 ) );
  _gpioByKey.insert( std::make_pair( "J8-35", 19 ) );
  _gpioByKey.insert( std::make_pair( "J8-36", 16 ) );
  _gpioByKey.insert( std::make_pair( "J8-37", 26 ) );
  _gpioByKey.insert( std::make_pair( "J8-38", 20 ) );
  _gpioByKey.insert( std::make_pair( "J8-40", 21 ) );

  // Name list
  _gpioByName.insert( std::make_pair( "GPIO14", 14 ) );
  _gpioByName.insert( std::make_pair( "GPIO15", 15 ) );
  _gpioByName.insert( std::make_pair( "GPIO18", 18 ) );
  _gpioByName.insert( std::make_pair( "GPIO23", 23 ) );
  _gpioByName.insert( std::make_pair( "GPIO24", 24  ) );
  _gpioByName.insert( std::make_pair( "GPIO25", 25 ) );
  _gpioByName.insert( std::make_pair( "GPIO08", 8 ) );
  _gpioByName.insert( std::make_pair( "GPIO07", 7 ) );
  _gpioByName.insert( std::make_pair( "GPIO2", 2  ) );
  _gpioByName.insert( std::make_pair( "GPIO3", 3 ) );
  _gpioByName.insert( std::make_pair( "GPIO4", 4 ) );
  _gpioByName.insert( std::make_pair( "GPIO17", 17 ) );
  _gpioByName.insert( std::make_pair( "GPIO27", 27 ) );
  _gpioByName.insert( std::make_pair( "GPIO22", 22 ) );
  _gpioByName.insert( std::make_pair( "GPIO10", 10 ) );
  _gpioByName.insert( std::make_pair( "GPIO9", 9 ) );
  _gpioByName.insert( std::make_pair( "GPIO11", 11 ) );
  _gpioByName.insert( std::make_pair( "GPIO0", 0 ) );
  _gpioByName.insert( std::make_pair( "GPIO1", 1 ) );
  _gpioByName.insert( std::make_pair( "GPIO5", 5 ) );
  _gpioByName.insert( std::make_pair( "GPIO6", 6 ) );
  _gpioByName.insert( std::make_pair( "GPIO12", 12 ) );
  _gpioByName.insert( std::make_pair( "GPIO13", 13 ) );
  _gpioByName.insert( std::make_pair( "GPIO19", 19 ) );
  _gpioByName.insert( std::make_pair( "GPIO16", 16 ) );
  _gpioByName.insert( std::make_pair( "GPIO26", 26 ) );
  _gpioByName.insert( std::make_pair( "GPIO20", 20 ) );
  _gpioByName.insert( std::make_pair( "GPIO21", 21 ) );

  // Number list
  _gpioByNumber.insert( std::make_pair( "14", 14 ) );
  _gpioByNumber.insert( std::make_pair( "15", 15 ) );
  _gpioByNumber.insert( std::make_pair( "18", 18 ) );
  _gpioByNumber.insert( std::make_pair( "23", 23 ) );
  _gpioByNumber.insert( std::make_pair( "24", 24  ) );
  _gpioByNumber.insert( std::make_pair( "25", 25 ) );
  _gpioByNumber.insert( std::make_pair( "08", 8 ) );
  _gpioByNumber.insert( std::make_pair( "07", 7 ) );
  _gpioByNumber.insert( std::make_pair( "2", 2  ) );
  _gpioByNumber.insert( std::make_pair( "3", 3 ) );
  _gpioByNumber.insert( std::make_pair( "4", 4 ) );
  _gpioByNumber.insert( std::make_pair( "17", 17 ) );
  _gpioByNumber.insert( std::make_pair( "27", 27 ) );
  _gpioByNumber.insert( std::make_pair( "22", 22 ) );
  _gpioByNumber.insert( std::make_pair( "10", 10 ) );
  _gpioByNumber.insert( std::make_pair( "9", 9 ) );
  _gpioByNumber.insert( std::make_pair( "11", 11 ) );
  _gpioByNumber.insert( std::make_pair( "0", 0 ) );
  _gpioByNumber.insert( std::make_pair( "1", 1 ) );
  _gpioByNumber.insert( std::make_pair( "5", 5 ) );
  _gpioByNumber.insert( std::make_pair( "6", 6 ) );
  _gpioByNumber.insert( std::make_pair( "12", 12 ) );
  _gpioByNumber.insert( std::make_pair( "13", 13 ) );
  _gpioByNumber.insert( std::make_pair( "19", 19 ) );
  _gpioByNumber.insert( std::make_pair( "16", 16 ) );
  _gpioByNumber.insert( std::make_pair( "26", 26 ) );
  _gpioByNumber.insert( std::make_pair( "20", 20 ) );
  _gpioByNumber.insert( std::make_pair( "21", 21 ) );

}

//------------------------------------------------------------------------------
//! @brief   Init maps for BeagleBoneBlack
//------------------------------------------------------------------------------
void GpioConst::init_beagleboneblack() {
  std::cout << "GpioConst: Loading Look-Up table for BeagleBone Black" << std::endl;
  // Key list
  _gpioByKey.insert( std::make_pair( "USR0", 53 ) );
  _gpioByKey.insert( std::make_pair( "USR1", 54 ) );
  _gpioByKey.insert( std::make_pair( "USR2", 55 ) );
  _gpioByKey.insert( std::make_pair( "USR3", 56 ) );
  _gpioByKey.insert( std::make_pair( "P8_3", 38 ) );
  _gpioByKey.insert( std::make_pair( "P8_4", 39 ) );
  _gpioByKey.insert( std::make_pair( "P8_5", 34 ) );
  _gpioByKey.insert( std::make_pair( "P8_6", 35 ) );
  _gpioByKey.insert( std::make_pair( "P8_7", 66 ) );
  _gpioByKey.insert( std::make_pair( "P8_8", 67 ) );
  _gpioByKey.insert( std::make_pair( "P8_9", 69 ) );
  _gpioByKey.insert( std::make_pair( "P8_10", 68 ) );
  _gpioByKey.insert( std::make_pair( "P8_11", 45 ) );
  _gpioByKey.insert( std::make_pair( "P8_12", 44 ) );
  _gpioByKey.insert( std::make_pair( "P8_13", 23 ) );
  _gpioByKey.insert( std::make_pair( "P8_14", 26 ) );
  _gpioByKey.insert( std::make_pair( "P8_15", 47 ) );
  _gpioByKey.insert( std::make_pair( "P8_16", 46 ) );
  _gpioByKey.insert( std::make_pair( "P8_17", 27 ) );
  _gpioByKey.insert( std::make_pair( "P8_18", 65 ) );
  _gpioByKey.insert( std::make_pair( "P8_19", 22 ) );
  _gpioByKey.insert( std::make_pair( "P8_20", 63 ) );
  _gpioByKey.insert( std::make_pair( "P8_21", 62 ) );
  _gpioByKey.insert( std::make_pair( "P8_22", 37 ) );
  _gpioByKey.insert( std::make_pair( "P8_23", 36 ) );
  _gpioByKey.insert( std::make_pair( "P8_24", 33 ) );
  _gpioByKey.insert( std::make_pair( "P8_25", 32 ) );
  _gpioByKey.insert( std::make_pair( "P8_26", 61 ) );
  _gpioByKey.insert( std::make_pair( "P8_27", 86 ) );
  _gpioByKey.insert( std::make_pair( "P8_28", 88 ) );
  _gpioByKey.insert( std::make_pair( "P8_29", 87 ) );
  _gpioByKey.insert( std::make_pair( "P8_30", 89 ) );
  _gpioByKey.insert( std::make_pair( "P8_31", 10 ) );
  _gpioByKey.insert( std::make_pair( "P8_32", 11 ) );
  _gpioByKey.insert( std::make_pair( "P8_33", 9 ) );
  _gpioByKey.insert( std::make_pair( "P8_34", 81 ) );
  _gpioByKey.insert( std::make_pair( "P8_35", 8 ) );
  _gpioByKey.insert( std::make_pair( "P8_36", 80 ) );
  _gpioByKey.insert( std::make_pair( "P8_37", 78 ) );
  _gpioByKey.insert( std::make_pair( "P8_38", 79 ) );
  _gpioByKey.insert( std::make_pair( "P8_39", 76 ) );
  _gpioByKey.insert( std::make_pair( "P8_40", 77 ) );
  _gpioByKey.insert( std::make_pair( "P8_41", 74 ) );
  _gpioByKey.insert( std::make_pair( "P8_42", 75 ) );
  _gpioByKey.insert( std::make_pair( "P8_43", 72 ) );
  _gpioByKey.insert( std::make_pair( "P8_44", 73 ) );
  _gpioByKey.insert( std::make_pair( "P8_45", 70 ) );
  _gpioByKey.insert( std::make_pair( "P8_46", 71 ) );
  _gpioByKey.insert( std::make_pair( "P9_11", 30 ) );
  _gpioByKey.insert( std::make_pair( "P9_12", 60 ) );
  _gpioByKey.insert( std::make_pair( "P9_13", 31 ) );
  _gpioByKey.insert( std::make_pair( "P9_14", 50 ) );
  _gpioByKey.insert( std::make_pair( "P9_15", 48 ) );
  _gpioByKey.insert( std::make_pair( "P9_16", 51 ) );
  _gpioByKey.insert( std::make_pair( "P9_17", 5 ) );
  _gpioByKey.insert( std::make_pair( "P9_18", 4 ) );
  _gpioByKey.insert( std::make_pair( "P9_19", 13 ) );
  _gpioByKey.insert( std::make_pair( "P9_20", 12 ) );
  _gpioByKey.insert( std::make_pair( "P9_21", 3 ) );
  _gpioByKey.insert( std::make_pair( "P9_22", 2 ) );
  _gpioByKey.insert( std::make_pair( "P9_23", 49 ) );
  _gpioByKey.insert( std::make_pair( "P9_24", 15 ) );
  _gpioByKey.insert( std::make_pair( "P9_25", 117 ) );
  _gpioByKey.insert( std::make_pair( "P9_26", 14 ) );
  _gpioByKey.insert( std::make_pair( "P9_27", 115 ) );
  _gpioByKey.insert( std::make_pair( "P9_28", 113 ) );
  _gpioByKey.insert( std::make_pair( "P9_29", 111 ) );
  _gpioByKey.insert( std::make_pair( "P9_30", 112 ) );
  _gpioByKey.insert( std::make_pair( "P9_31", 110 ) );
  _gpioByKey.insert( std::make_pair( "P9_41", 20 ) );
  _gpioByKey.insert( std::make_pair( "P9_42", 7 ) );

  // Name list
  _gpioByName.insert( std::make_pair( "USR0", 53 ) );
  _gpioByName.insert( std::make_pair( "USR1", 54 ) );
  _gpioByName.insert( std::make_pair( "USR2", 55 ) );
  _gpioByName.insert( std::make_pair( "USR3", 56 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_6", 38 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_7", 39 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_2", 34 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_3", 35 ) );
  _gpioByName.insert( std::make_pair( "TIMER4", 66 ) );
  _gpioByName.insert( std::make_pair( "TIMER7", 67 ) );
  _gpioByName.insert( std::make_pair( "TIMER5", 69 ) );
  _gpioByName.insert( std::make_pair( "TIMER6", 68 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_13", 45 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_12", 44 ) );
  _gpioByName.insert( std::make_pair( "EHRPWM2B", 23 ) );
  _gpioByName.insert( std::make_pair( "GPIO0_26", 26 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_15", 47 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_14", 46 ) );
  _gpioByName.insert( std::make_pair( "GPIO0_27", 27 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_1", 65 ) );
  _gpioByName.insert( std::make_pair( "EHRPWM2A", 22 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_31", 63 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_30", 62 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_5", 37 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_4", 36 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_1", 33 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_0", 32 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_29", 61 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_22", 86 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_24", 88 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_23", 87 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_25", 89 ) );
  _gpioByName.insert( std::make_pair( "UART5_CTSN", 10 ) );
  _gpioByName.insert( std::make_pair( "UART5_RTSN", 11 ) );
  _gpioByName.insert( std::make_pair( "UART4_RTSN", 9 ) );
  _gpioByName.insert( std::make_pair( "UART3_RTSN", 81 ) );
  _gpioByName.insert( std::make_pair( "UART4_CTSN", 8 ) );
  _gpioByName.insert( std::make_pair( "UART3_CTSN", 80 ) );
  _gpioByName.insert( std::make_pair( "UART5_TXD", 78 ) );
  _gpioByName.insert( std::make_pair( "UART5_RXD", 79 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_12", 76 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_13", 77 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_10", 74 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_11", 75 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_8", 72 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_9", 73 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_6", 70 ) );
  _gpioByName.insert( std::make_pair( "GPIO2_7", 71 ) );
  _gpioByName.insert( std::make_pair( "UART4_RXD", 30 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_28", 60 ) );
  _gpioByName.insert( std::make_pair( "UART4_TXD", 31 ) );
  _gpioByName.insert( std::make_pair( "EHRPWM1A", 50 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_16", 48 ) );
  _gpioByName.insert( std::make_pair( "EHRPWM1B", 51 ) );
  _gpioByName.insert( std::make_pair( "I2C1_SCL", 5 ) );
  _gpioByName.insert( std::make_pair( "I2C1_SDA", 4 ) );
  _gpioByName.insert( std::make_pair( "I2C2_SCL", 13 ) );
  _gpioByName.insert( std::make_pair( "I2C2_SDA", 12 ) );
  _gpioByName.insert( std::make_pair( "UART2_TXD", 3 ) );
  _gpioByName.insert( std::make_pair( "UART2_RXD", 2 ) );
  _gpioByName.insert( std::make_pair( "GPIO1_17", 49 ) );
  _gpioByName.insert( std::make_pair( "UART1_TXD", 15 ) );
  _gpioByName.insert( std::make_pair( "GPIO3_21", 117 ) );
  _gpioByName.insert( std::make_pair( "UART1_RXD", 14 ) );
  _gpioByName.insert( std::make_pair( "GPIO3_19", 115 ) );
  _gpioByName.insert( std::make_pair( "SPI1_CS0", 113 ) );
  _gpioByName.insert( std::make_pair( "SPI1_D0", 111 ) );
  _gpioByName.insert( std::make_pair( "SPI1_D1", 112 ) );
  _gpioByName.insert( std::make_pair( "SPI1_SCLK", 110 ) );
  _gpioByName.insert( std::make_pair( "CLKOUT2", 20 ) );
  _gpioByName.insert( std::make_pair( "GPIO0_7", 7 ) );

  // Number list
  _gpioByNumber.insert( std::make_pair( "53", 53 ) );
  _gpioByNumber.insert( std::make_pair( "54", 54 ) );
  _gpioByNumber.insert( std::make_pair( "55", 55 ) );
  _gpioByNumber.insert( std::make_pair( "56", 56 ) );
  _gpioByNumber.insert( std::make_pair( "38", 38 ) );
  _gpioByNumber.insert( std::make_pair( "39", 39 ) );
  _gpioByNumber.insert( std::make_pair( "34", 34 ) );
  _gpioByNumber.insert( std::make_pair( "35", 35 ) );
  _gpioByNumber.insert( std::make_pair( "66", 66 ) );
  _gpioByNumber.insert( std::make_pair( "67", 67 ) );
  _gpioByNumber.insert( std::make_pair( "69", 69 ) );
  _gpioByNumber.insert( std::make_pair( "68", 68 ) );
  _gpioByNumber.insert( std::make_pair( "45", 45 ) );
  _gpioByNumber.insert( std::make_pair( "44", 44 ) );
  _gpioByNumber.insert( std::make_pair( "23", 23 ) );
  _gpioByNumber.insert( std::make_pair( "26", 26 ) );
  _gpioByNumber.insert( std::make_pair( "47", 47 ) );
  _gpioByNumber.insert( std::make_pair( "46", 46 ) );
  _gpioByNumber.insert( std::make_pair( "27", 27 ) );
  _gpioByNumber.insert( std::make_pair( "65", 65 ) );
  _gpioByNumber.insert( std::make_pair( "22", 22 ) );
  _gpioByNumber.insert( std::make_pair( "63", 63 ) );
  _gpioByNumber.insert( std::make_pair( "62", 62 ) );
  _gpioByNumber.insert( std::make_pair( "37", 37 ) );
  _gpioByNumber.insert( std::make_pair( "36", 36 ) );
  _gpioByNumber.insert( std::make_pair( "33", 33 ) );
  _gpioByNumber.insert( std::make_pair( "32", 32 ) );
  _gpioByNumber.insert( std::make_pair( "61", 61 ) );
  _gpioByNumber.insert( std::make_pair( "86", 86 ) );
  _gpioByNumber.insert( std::make_pair( "88", 88 ) );
  _gpioByNumber.insert( std::make_pair( "87", 87 ) );
  _gpioByNumber.insert( std::make_pair( "89", 89 ) );
  _gpioByNumber.insert( std::make_pair( "10", 10 ) );
  _gpioByNumber.insert( std::make_pair( "11", 11 ) );
  _gpioByNumber.insert( std::make_pair( "9", 9 ) );
  _gpioByNumber.insert( std::make_pair( "81", 81 ) );
  _gpioByNumber.insert( std::make_pair( "8", 8 ) );
  _gpioByNumber.insert( std::make_pair( "80", 80 ) );
  _gpioByNumber.insert( std::make_pair( "78", 78 ) );
  _gpioByNumber.insert( std::make_pair( "79", 79 ) );
  _gpioByNumber.insert( std::make_pair( "76", 76 ) );
  _gpioByNumber.insert( std::make_pair( "77", 77 ) );
  _gpioByNumber.insert( std::make_pair( "74", 74 ) );
  _gpioByNumber.insert( std::make_pair( "75", 75 ) );
  _gpioByNumber.insert( std::make_pair( "72", 72 ) );
  _gpioByNumber.insert( std::make_pair( "73", 73 ) );
  _gpioByNumber.insert( std::make_pair( "70", 70 ) );
  _gpioByNumber.insert( std::make_pair( "71", 71 ) );
  _gpioByNumber.insert( std::make_pair( "30", 30 ) );
  _gpioByNumber.insert( std::make_pair( "60", 60 ) );
  _gpioByNumber.insert( std::make_pair( "31", 31 ) );
  _gpioByNumber.insert( std::make_pair( "50", 50 ) );
  _gpioByNumber.insert( std::make_pair( "48", 48 ) );
  _gpioByNumber.insert( std::make_pair( "51", 51 ) );
  _gpioByNumber.insert( std::make_pair( "5", 5 ) );
  _gpioByNumber.insert( std::make_pair( "4", 4 ) );
  _gpioByNumber.insert( std::make_pair( "13", 13 ) );
  _gpioByNumber.insert( std::make_pair( "12", 12 ) );
  _gpioByNumber.insert( std::make_pair( "3", 3 ) );
  _gpioByNumber.insert( std::make_pair( "2", 2 ) );
  _gpioByNumber.insert( std::make_pair( "49", 49 ) );
  _gpioByNumber.insert( std::make_pair( "15", 15 ) );
  _gpioByNumber.insert( std::make_pair( "117", 117 ) );
  _gpioByNumber.insert( std::make_pair( "14", 14 ) );
  _gpioByNumber.insert( std::make_pair( "115", 115 ) );
  _gpioByNumber.insert( std::make_pair( "113", 113 ) );
  _gpioByNumber.insert( std::make_pair( "111", 111 ) );
  _gpioByNumber.insert( std::make_pair( "112", 112 ) );
  _gpioByNumber.insert( std::make_pair( "110", 110 ) );
  _gpioByNumber.insert( std::make_pair( "20", 20 ) );
  _gpioByNumber.insert( std::make_pair( "7", 7 ) );

}


