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
#include <vector>

// EPICS includes
#include <epicsTypes.h>

// local includes

//_____ D E F I N I T I O N S __________________________________________________

class GpioManager {
  public:

    epicsUInt32 registerGpio( epicsUInt32 gpio, epicsUInt64 flags );
    void request();
    void setValue( epicsUInt64 mask, epicsUInt32 val );
    epicsUInt32 getValue( epicsUInt64 mask );

    epicsUInt32 event();

    static GpioManager& instance() {
      static GpioManager rinstance;
      return rinstance;
    }

  private:
    struct gpio_t {
      epicsUInt32 id;
      epicsUInt64 flags;
    };

    GpioManager();
    ~GpioManager();
    GpioManager( GpioManager const& rother ) = delete;
    GpioManager& operator=( GpioManager const& rother ) = delete;

    std::vector< gpio_t > _inp;
    std::vector< gpio_t > _out;

    int _fdInp;
    int _fdOut;
};

#endif

