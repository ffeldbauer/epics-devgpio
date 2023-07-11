/*******************************************************************************
 * Copyright (C) 2015 Florian Feldbauer <feldbaue@kph.uni-mainz.de>
 *                    - Helmholtz-Institut Mainz
 *
 * This file is part of devGpio
 *
 * devGpio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * devGpio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * version 1.0.0; Aug 13, 2015
 *
*******************************************************************************/

#ifndef DEV_GPIO_H
#define DEV_GPIO_H

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C includes  */

/* EPICS includes */
#include <callback.h>
#include <dbCommon.h>
#include <dbScan.h>
#include <devSup.h>
#include <epicsTime.h>
#include <shareLib.h>

/* local includes */

/*_____ D E F I N I T I O N S ________________________________________________*/

/* Define return values for device support functions */
#define OK                    0
#define DO_NOT_CONVERT        2
#define ERROR                 -1

/**
 * @brief Record configuration
 *
 * Configuration parameters used at initialization
 * of the record
 */
typedef struct {
  struct link const* ioLink;
  epicsUInt64 flags;
} devGpio_rec_t;

/**
 * @brief Private Device Data
 *
 * Private data needed by device support routines
 */
typedef struct {
  int fd;              /**< File descriptor for GPIO handling */
  CALLBACK *pcallback; /**< Address of EPICS callback structure */
  IOSCANPVT ioscanpvt; /**< EPICS Structure needed for I/O Intrupt handling*/
} devGpio_info_t;

#ifdef __cplusplus
extern "C" {
#endif

epicsShareExtern long devGpioInit( int after );
epicsShareExtern epicsUInt16 devGpioInitRecord( dbCommon *prec, devGpio_rec_t* pconf );
epicsShareExtern long devGpioGetIoIntInfo( int cmd, dbCommon *prec, IOSCANPVT *ppvt );
epicsShareExtern void devGpioCallback( CALLBACK *pcallback );

#ifdef __cplusplus
} //extern "C"
#endif /* cplusplus */

#endif

