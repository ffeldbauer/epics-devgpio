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

//! TODO: I/O interrupt handling hard coded on both flanks...Option to set by user?

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

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
#include "devGpioManager.h"
#include "devGpioErrors.h"
#include "GpioConst.h"
#include "GpioIntHandler.h"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________

//_____ L O C A L S ____________________________________________________________
static GpioIntHandler* intHandler = NULL;

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

    intHandler = new GpioIntHandler();

  } else {

    static bool firstRunAfter = true;
    if ( !firstRunAfter ) return OK;
    firstRunAfter = false;

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
long devGpioInitRecord( dbCommon *prec, devGpio_rec_t* pconf ){
  epicsUInt32 gpioID = 0xffffffff;
  std::vector< std::string > options;

  if( INST_IO != pconf->ioLink->type ) {
    std::cerr << prec->name << ": Invalid link type for INP/OUT field: "
              << pamaplinkType[ pconf->ioLink->type ].strvalue
              << std::endl;
    return ERROR;
  }

  std::istringstream ss( pconf->ioLink->value.instio.string );
  std::string option;
  while( std::getline( ss, option, ' ' ) ) options.push_back( option );

  if( options.size() > 2 || options.empty() ) {
    std::cerr << prec->name << ": Invalid INP/OUT field: " << ss.str() << "\n"
              << "    Syntax is \"<GPIO> [LOGIC]\"" << std::endl;
    return ERROR;
  }

  GpioManager::LOGIC_VALUE logic = GpioManager::ACTIVE_HIGH;
  if( options.size() == 2 ) {
    if( iequals( options.at(1), "high" ) || iequals( options.at(1), "h" ) ) {
      logic = GpioManager::ACTIVE_HIGH;
    } else if( iequals( options.at(1), "low" ) || iequals( options.at(1), "l" ) ) {
      logic = GpioManager::ACTIVE_LOW;
    } else {
      std::cerr << prec->name << ": Invalid option for logic: " << options.at(1) << std::endl;
      return ERROR;
    }
  }

  try{
    if( GpioConst::exists() ) {
      gpioID = GpioConst::instance()->findGPIO( options.at(0) );
    } else {
      sscanf( options.at(0).c_str(), "%u", &gpioID );
    }
    if( 0xffffffff == gpioID ) { // None of the supported boards has a gpio 4294967295
      std::cerr << prec->name << ": Invalid GPIO number " << options.at(0) << std::endl;
      return ERROR;
    }
    GpioManager::instance().exportPin( gpioID );
  } catch( GpioManagerWarning &e ) {
    std::cerr << prec->name << ": " << e.what() << std::endl;
  } catch( DevGpioException &e ) {
    std::cerr << prec->name << ": " << e.what() << std::endl;
    return ERROR;
  }

  try{ 
    // On BeagleBone Black write permissions to gpios are granted via udev rule.
    // Udev rule needs about 25 ms to set permissions for the files belonging to
    // one GPIO.
    GpioManager::instance().waitForUdev( gpioID );

    GpioManager::instance().setLogic( gpioID, logic );

    if( pconf->output ) {
      GpioManager::instance().setDirection( gpioID, GpioManager::OUTPUT );
    } else {
      GpioManager::instance().setDirection( gpioID, GpioManager::INPUT );
      pconf->initialValue = GpioManager::instance().getValue( gpioID );
    }

  } catch( GpioManagerWarning &e ) {
    std::cerr << prec->name << ": " << e.what() << std::endl;
  } catch( GpioManagerError &e ) {
    std::cerr << prec->name << ": " << e.what() << std::endl;
    return ERROR;
  }

  devGpio_info_t *pinfo = new devGpio_info_t;
  pinfo->gpio = gpioID;
  pinfo->prec = prec;
  pinfo->pcallback = NULL;  // just to be sure

  // I/O Intr handling
  scanIoInit( &pinfo->ioscanpvt );

  prec->dpvt = pinfo;

  return OK;
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
    GpioManager::instance().setEdge( pinfo->gpio, GpioManager::BOTH );
    intHandler->registerInterrupt( pinfo );
  } else {
    intHandler->cancelInterrupt( pinfo );
    GpioManager::instance().setEdge( pinfo->gpio, GpioManager::NONE );
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

  devGpio_info_t *pinfo = (devGpio_info_t *)puser;

  dbScanLock( pinfo->prec );
  dbProcess( pinfo->prec );
  dbScanUnlock( pinfo->prec );
}

//------------------------------------------------------------------------------
//! @brief   Wrapper to read GPIO value
//!
//! @param   [in]  pinfo  Address of private data from record
//!
//! @return  ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devGpioRead( devGpio_info_t *pinfo ){
  try{
    pinfo->value = GpioManager::instance().getValue( pinfo->gpio );
  } catch( GpioManagerError &e ) {
    strncpy( pinfo->errmsg, e.what(), 255 );
    return ERROR;
  }
  return OK;
}

//------------------------------------------------------------------------------
//! @brief   Wrapper to write GPIO value
//!
//! @param   [in]  pinfo  Address of private data from record
//!
//! @return  ERROR in case of an error, otherwise OK
//------------------------------------------------------------------------------
long devGpioWrite( devGpio_info_t *pinfo ){
  try{
    GpioManager::instance().setValue( pinfo->gpio, pinfo->value );
  } catch( GpioManagerError &e ) {
    strncpy( pinfo->errmsg, e.what(), 255 );
    return ERROR;
  }
  return OK;
}

extern "C" {
  //----------------------------------------------------------------------------
  //! @brief   EPICS iocsh callable function to call constructor
  //!          for the GpioConst class
  //!
  //! @param   [in]  board  type of ARM-Board
  //----------------------------------------------------------------------------
  int devGpioConstConfigure( const char *board ) {
    if( strcmp( board, "RASPI B REV2" ) == 0 )           GpioConst::create( GpioConst::RASPI_B_REV2 );
    else if( strcmp( board, "RASPI B+" ) == 0 )          GpioConst::create( GpioConst::RASPI_BP );
    else if( strcmp( board, "BEAGLEBONE BLACK" ) == 0 )  GpioConst::create( GpioConst::BEAGLEBONE_BLACK );
    else {
      std::cerr << "GpioConstConfigure: Invalid argument! '" << board << "'" << std::endl;
      return ERROR;
    }
    return OK;
  }
  static const iocshArg initArg0 = { "board", iocshArgString };
  static const iocshArg * const initArgs[] = { &initArg0 };
  static const iocshFuncDef initFuncDef = { "GpioConstConfigure", 1, initArgs };
  static void initCallFunc( const iocshArgBuf *args ) {
    devGpioConstConfigure( args[0].sval );
  }

  //----------------------------------------------------------------------------
  //! @brief   Register functions to EPICS
  //----------------------------------------------------------------------------
  void devGpioConstRegister( void ) {
    static int firstTime = 1;
    if ( firstTime ) {
      iocshRegister( &initFuncDef, initCallFunc );
      firstTime = 0;
    }
  }
  epicsExportRegistrar( devGpioConstRegister );
}

