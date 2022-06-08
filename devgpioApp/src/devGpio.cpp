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
#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>

// EPICS includes
#include <alarm.h>
#include <biRecord.h>
#include <boRecord.h>
#include <dbAccess.h>
#include <errlog.h>
#include <epicsExport.h>
#include <iocsh.h>
#include <recGbl.h>

// local includes
#include "devGpio.h"
#include "GpioIntHandler.hpp"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________

//_____ L O C A L S ____________________________________________________________
static GpioIntHandler* intHandler = nullptr;
static int gpiochip = -1;

//_____ F U N C T I O N S ______________________________________________________

//------------------------------------------------------------------------------
//! @brief   Case insensitve comparison between two strings
//!
//! Similar function is available within BOOST. But BOOST is not available on
//! Raspberry Pi. Therefore using BOOST would make this code non-portable...
//------------------------------------------------------------------------------
static bool iequals( std::string const & a, std::string const& b) {
  if( b.size() != a.size() ) return false;
  for( unsigned i = 0; i < a.size(); ++i )
    if( tolower( a[i] ) != tolower( b[i] ) ) return false;
  return true;
}

//------------------------------------------------------------------------------
//! @brief   Check if string is a number
//------------------------------------------------------------------------------
static bool is_number( std::string const& s ) {
  return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

//------------------------------------------------------------------------------
//! @brief   Initialization of device support
//!
//! @param   [in]  after  flag telling if function is called after or before
//!                       record initialization
//!
//! @return  In case of error return -1, otherwise return 0
//------------------------------------------------------------------------------
long devGpioInit( int after ) {

  if ( 0 == after ) { // before records have been initialized
    static bool firstRunBefore = true;
    if ( !firstRunBefore ) return OK;
    firstRunBefore = false;

    gpiochip = open( "/dev/gpiochip0", 0 );
    if( 0 > gpiochip ) {
      perror( "Could not open GPIO device: " );
      return ERROR;
    }

    intHandler = new GpioIntHandler();

  } else {

    static bool firstRunAfter = true;
    if ( !firstRunAfter ) return OK;
    firstRunAfter = false;

    if( 0 <= gpiochip ) {
      close( gpiochip );
    }

    intHandler->thread.start();
  }

  return OK;

}

//-------------------------------------------------------------------------------
//! @brief   Common initialization of the record
//!
//! @param   [in]  prec       Address of the record calling this function
//! @param   [in]  pconf      Address of record configuration
//!
//! @return  In case of error return -1, otherwise return 0
//------------------------------------------------------------------------------
epicsUInt16 devGpioInitRecord( dbCommon *prec, devGpio_rec_t* pconf ){
  if( 0 > gpiochip )  return ERROR;

  if( INST_IO != pconf->ioLink->type ) {
    std::cerr << prec->name << ": Invalid link type for INP/OUT field: "
              << pamaplinkType[ pconf->ioLink->type ].strvalue
              << std::endl;
    return ERROR;
  }

  std::istringstream ss( pconf->ioLink->value.instio.string );
  std::string option;
  std::vector< std::string > options;
  while( std::getline( ss, option, ' ' ) ) options.push_back( option );

  if( options.empty() ) {
    std::cerr << prec->name << ": Invalid INP/OUT field: " << ss.str() << "\n"
              << "    Syntax is \"@<GPIO1> [GPIO2] [LOW] [FALLING/RISING/BOTH]\"" << std::endl;
    return ERROR;
  }

  std::vector<epicsUInt32> gpios;
  for( auto opt : options ){
    if( iequals( opt, "low" ) || iequals( opt, "l" ) ) {
      pconf->flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
    } else if( iequals( opt, "falling" ) || iequals( opt, "f" ) ) {
      pconf->flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING;
    } else if( iequals( opt, "rising" ) || iequals( opt, "r" ) ) {
      pconf->flags |= GPIO_V2_LINE_FLAG_EDGE_RISING;
    } else if( iequals( opt, "both" ) || iequals( opt, "b" ) ) {
      pconf->flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING | GPIO_V2_LINE_FLAG_EDGE_RISING;
    } else if( is_number( opt )) {
      gpios.push_back( std::stoi( opt ));
    } else {
      std::cerr << prec->name << ": Invalid option: " << opt << std::endl;
      return ERROR;
    }
  }

  struct gpio_v2_line_request req;
  memset( &req, 0, sizeof( req ));
  strcpy( req.consumer, "EPICS devGpio" );

  epicsUInt16 nobt = 0;
  for( auto g : gpios ){
    struct gpio_v2_line_info linfo;
    memset( &linfo, 0, sizeof( linfo ));
    linfo.offset = g;
    int rtn = ioctl( gpiochip, GPIO_V2_GET_LINEINFO_IOCTL, &linfo );
    if( -1 == rtn ) {
      std::cerr << prec->name << ": Unable to get line info: " << strerror( errno ) << std::endl;
      return ERROR;
    }
    if( linfo.flags & GPIO_V2_LINE_FLAG_USED ) {
      std::cerr << prec->name << ": GPIO " << g << " already in use" << std::endl;
      return ERROR;
    }
    req.offsets[nobt++] = g;
  }
  req.num_lines = nobt;
  req.config.flags = pconf->flags;

  int rtn = ioctl( gpiochip, GPIO_V2_GET_LINE_IOCTL, &req );
  if( -1 == rtn ) {
    std::cerr << prec->name << ": Request gpio lines failed: " << strerror( errno ) << std::endl;
    return ERROR;
  }

  devGpio_info_t *pinfo = new devGpio_info_t;
  pinfo->fd = req.fd;
  pinfo->pcallback = nullptr;

  // I/O Intr handling
  scanIoInit( &pinfo->ioscanpvt );

  prec->dpvt = pinfo;

  return nobt;
}

//------------------------------------------------------------------------------
//! @brief   Get I/O Intr Information of record
//!
//! @param   [in]  cmd   0 if record is placed in, 1 if taken out of an I/O scan list 
//! @param   [in]  prec  Address of record calling this funciton
//! @param   [out] ppvt  Address of IOSCANPVT structure
//!
//! @return  ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devGpioGetIoIntInfo( int cmd, dbCommon *prec, IOSCANPVT *ppvt ){
  devGpio_info_t *pinfo = (devGpio_info_t *)prec->dpvt;
  *ppvt = pinfo->ioscanpvt;
  if ( 0 == cmd ) {
    intHandler->registerInterrupt( prec );
  } else {
    intHandler->cancelInterrupt( pinfo );
  }
  return OK;
}

//------------------------------------------------------------------------------
//! @brief   Callback for asynchronous handling of set parameters
//!
//! This callback processes the the record defined in callback user.
//!
//! @param   [in]  pcallback   Address of EPICS CALLBACK structure
//------------------------------------------------------------------------------
void devGpioCallback( CALLBACK *pcallback ) {
  void *puser;
  callbackGetUser( puser, pcallback );
  dbCommon* prec = (dbCommon *)puser;
  dbScanLock( prec );
  dbProcess( prec );
  dbScanUnlock( prec );
}

