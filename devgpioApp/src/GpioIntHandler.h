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

#ifndef DEV_GPIO_INT_HANDLER_H
#define DEV_GPIO_INT_HANDLER_H

//_____ I N C L U D E S ________________________________________________________

// ANSI C/C++ includes
#include <map>
#include <string>

// EPICS includes
#include <epicsThread.h>
#include <epicsTypes.h>

// local includes
#include "devGpio.h"

//_____ D E F I N I T I O N S __________________________________________________

// forward declaration
struct epoll_event;

//! @brief   thread handling interrupts from GPIOs
class GpioIntHandler: public epicsThreadRunable {
 public:
  GpioIntHandler();
  virtual ~GpioIntHandler();
  GpioIntHandler( GpioIntHandler const& rother ); // Not implemented
  GpioIntHandler& operator=( GpioIntHandler const& rother ); // Not implemented

  virtual void run();

  epicsThread thread;

  void registerInterrupt( devGpio_info_t* pinfo );
  void cancelInterrupt( devGpio_info_t const* pinfo );

 private:
  struct HANDLE {
    devGpio_info_t* pinfo;
    struct epoll_event *pev;
  };

  int _efd;
  double _pause;
  std::map< int, HANDLE* > _recs;
};

#endif

