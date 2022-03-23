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

//! @file GpioIntHandler.cpp
//! @author F.Feldbauer
//! @date 13 Aug 2015
//! @brief Implementation of GPIO Manager class

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

// EPICS includes

// local includes
#include "devGpio.h"
#include "GpioIntHandler.hpp"

//_____ D E F I N I T I O N S __________________________________________________

//_____ G L O B A L S __________________________________________________________

//_____ L O C A L S ____________________________________________________________

//_____ F U N C T I O N S ______________________________________________________

//------------------------------------------------------------------------------
//! @brief   Standard Constructor
//------------------------------------------------------------------------------
GpioIntHandler::GpioIntHandler()
  : thread( *this, "devGpio", epicsThreadGetStackSize( epicsThreadStackSmall ), 50 )
{
  _pause = 5;
  _recs.clear();
}

//------------------------------------------------------------------------------
//! @brief   Standard Destructor
//------------------------------------------------------------------------------
GpioIntHandler::~GpioIntHandler() {
  _recs.clear();
}

//------------------------------------------------------------------------------
//! @brief   Run operation of thread
//!
//------------------------------------------------------------------------------
void GpioIntHandler::run() {

  while( true ) {
    if( _recs.empty() ) {
      this->thread.sleep( _pause );
      continue;
    }

    for( auto r : _recs ) {
      struct gpio_v2_line_event event;
      int rtn = read( r->fd, &event, sizeof(event));
      if( -1 == rtn ) {
        if( errno == -EAGAIN ) {
          continue;
        } else {
          perror( "GpioIntHandler: Failed to read event: " );
          continue;
        }
      }
      if( rtn != sizeof( event )) {
        perror( "GpioIntHandler: Failed to read event: " );
        continue;
      }
      callbackRequest( r->pcallback );
    }
  }
}

//------------------------------------------------------------------------------
//! @brief   Add a record to the list
//!
//! Registers a new record to be checked by the thread
//!
//! @param   [in]  prec  Address of the record to be added
//------------------------------------------------------------------------------
void GpioIntHandler::registerInterrupt( dbCommon *prec ) {
  devGpio_info_t *pinfo = (devGpio_info_t *)prec->dpvt;
  if( !pinfo->pcallback ) {
    CALLBACK *pcallback = new CALLBACK;
    callbackSetCallback( devGpioCallback, pcallback );
    callbackSetUser( (void*)prec, pcallback );
    callbackSetPriority( priorityLow, pcallback );
    pinfo->pcallback = pcallback;
  }
  _recs.push_back( pinfo );
}

//------------------------------------------------------------------------------
//! @brief   Remove a record to the list
//!
//! Removes a record from the list which is checked by the thread for updates
//!
//! @param   [in]  pinfo  Address of the record's private data structure
//------------------------------------------------------------------------------
void GpioIntHandler::cancelInterrupt( devGpio_info_t* pinfo ) {
  if( pinfo->pcallback ) {
    delete pinfo->pcallback;
    pinfo->pcallback = nullptr;
  }
  std::vector<devGpio_info_t const*>::iterator it = std::find( _recs.begin(), _recs.end(), pinfo );
  _recs.erase(it);
}

