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
#include <cstdio>
#include <exception>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <sys/epoll.h>
#include <unistd.h>

// EPICS includes

// local includes
#include "devGpio.h"
#include "GpioIntHandler.h"

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

  _efd = epoll_create1(0);
  if( -1 == _efd ) {
    perror("epoll_create1");
    return;
  }

}

//------------------------------------------------------------------------------
//! @brief   Standard Destructor
//------------------------------------------------------------------------------
GpioIntHandler::~GpioIntHandler() {
  _recs.clear();
  close( _efd );
}

//------------------------------------------------------------------------------
//! @brief   Run operation of thread
//!
//------------------------------------------------------------------------------
void GpioIntHandler::run() {

  int max_events = 0;
  int nfds = 0;
  struct epoll_event *events = NULL;

  while( true ) {
    if( _recs.empty() ) {
      this->thread.sleep( _pause );
      continue;
    }

    max_events = _recs.size();
    events = new struct epoll_event[max_events];

    nfds = epoll_wait( _efd, events, max_events, 500 );
    if( -1 == nfds ) {
      perror("epoll_wait");
      break;
    }

    for( int n = 0; n < nfds; ++n ) {

      std::map<int, HANDLE*>::iterator it = _recs.find( events[n].data.fd );
      if( it != _recs.end() ) {
        callbackRequest( it->second->pinfo->pcallback );
      }
    }
    delete[] events;
    events = NULL;
  }
}

//------------------------------------------------------------------------------
//! @brief   Add a record to the list
//!
//! Registers a new record to be checked by the thread
//!
//! @param   [in]  prec  Address of the record to be added
//------------------------------------------------------------------------------
void GpioIntHandler::registerInterrupt( devGpio_info_t *pinfo ) {
  static std::string gpiobase = "/sys/class/gpio/gpio";

  if( !pinfo->pcallback ) {
    CALLBACK *pcallback = new CALLBACK;
    callbackSetCallback( devGpioCallback, pcallback );
    callbackSetUser( (void*)pinfo, pcallback );
    callbackSetPriority( priorityLow, pcallback );
    pinfo->pcallback = pcallback;
  }

  std::stringstream filename;
  filename << gpiobase << pinfo->gpio << "/value";

  int fd = open( filename.str().c_str(), O_RDONLY | O_NONBLOCK );

  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
  ev.data.fd = fd;

  if( epoll_ctl( _efd, EPOLL_CTL_ADD, fd, &ev ) == -1 ) {
    perror( "epoll_ctl" );
    return;
  }

  HANDLE *phandle = new HANDLE;
  phandle->pinfo = pinfo;
  phandle->pev = &ev;

  _recs.insert( std::make_pair( fd, phandle ) );

}

//------------------------------------------------------------------------------
//! @brief   Remove a record to the list
//!
//! Removes a record from the list which is checked by the thread for updates
//!
//! @param   [in]  pinfo  Address of the record's private data structure
//------------------------------------------------------------------------------
void GpioIntHandler::cancelInterrupt( devGpio_info_t const* pinfo ) {
  std::map<int, HANDLE*>::iterator it = _recs.begin();
  for( ; it != _recs.end(); ++it ) {
    if( pinfo == it->second->pinfo ) {
      if( epoll_ctl( _efd, EPOLL_CTL_DEL, it->first, it->second->pev ) == -1 ) {
        perror( "epoll_ctl" );
        return;
      }
      close( it->first );
      _recs.erase( it );
      break;
    }
  } 
}

