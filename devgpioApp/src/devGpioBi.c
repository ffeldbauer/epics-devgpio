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

/**
 * @file devGpioBi.c
 * @author F.Feldbauer
 * @date 13 Aug 2015
 * @brief Device Support implementation for bi records
 */

/*_____ I N C L U D E S ______________________________________________________*/

/* ANSI C includes  */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>

/* EPICS includes */
#include <biRecord.h>
#include <alarm.h>
#include <dbAccess.h>
#include <errlog.h>
#include <epicsExport.h>
#include <epicsTypes.h>
#include <iocLog.h>
#include <iocsh.h>
#include <recGbl.h>

/* local includes */
#include "devGpio.h"

/*_____ D E F I N I T I O N S ________________________________________________*/
static long devGpioInitRecord_bi( biRecord *prec );
static long devGpioRead_bi( biRecord *prec );

/*_____ G L O B A L S ________________________________________________________*/
devGpio_dset_t devGpioBi = {
  6,
  NULL,
  devGpioInit,
  devGpioInitRecord_bi,
  devGpioGetIoIntInfo,
  devGpioRead_bi,
  NULL
};
epicsExportAddress( dset, devGpioBi );

/*_____ L O C A L S __________________________________________________________*/

/*_____ F U N C T I O N S ____________________________________________________*/

/**-----------------------------------------------------------------------------
 * @brief   Initialization of bi records
 *
 * @param   [in]  prec   Address of the record calling this function
 *
 * @return  In case of error return -1, otherwise return 0
 *----------------------------------------------------------------------------*/
long devGpioInitRecord_bi( biRecord *prec ){
  prec->pact = (epicsUInt8)true; /* disable record */

  devGpio_rec_t conf = { &prec->inp, GPIO_V2_LINE_FLAG_INPUT };
  epicsUInt16 nobt = devGpioInitRecord( (dbCommon*)prec, &conf );
  if( 1 != nobt ) {
    fprintf( stderr, "\033[31;1m%s: Invalid number of gpio lines: %u\033[0m\n",
             prec->name, nobt );
    return ERROR;
  }

  prec->udf = 0;
  prec->pact = (epicsUInt8)false; /* enable record */

  return OK;
}

/**-----------------------------------------------------------------------------
 * @brief   Read routine of bi records
 *
 * @param   [in]  prec   Address of the record calling this function
 *
 * @return  In case of error return -1, otherwise return 0
 *----------------------------------------------------------------------------*/
long devGpioRead_bi( biRecord *prec ) {
  devGpio_info_t *pinfo = (devGpio_info_t *)prec->dpvt;

  struct gpio_v2_line_values values = { 0, 1 };
  int ret = ioctl( pinfo->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values );
  if( -1 == ret ) {
    fprintf( stderr, "\033[31;1m%s: Could not read gpio line: %s\033[0m\n",
             prec->name, strerror( errno ) );
    recGblSetSevr( prec, READ_ALARM, INVALID_ALARM );
    return ERROR;
  }
  prec->rval = values.bits & 1;
  return OK;
}

