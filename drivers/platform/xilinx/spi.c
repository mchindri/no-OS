/***************************************************************************//**
 *   @file   spi.c
 *   @brief  Implementation of Xilinx SPI Generic Driver.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>

#include <xparameters.h>
#ifdef XPAR_XSPI_NUM_INSTANCES
#include <xspi.h>
#endif
#ifdef XPAR_XSPIPS_NUM_INSTANCES
#include <xspips.h>
#endif

#include "error.h"
#include "spi.h"
#include "spi_extra.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param init_param - The structure that contains the SPI parameters.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_init(struct spi_desc **desc,
		 const struct spi_init_param *param)
{
	int32_t		ret;
	spi_desc	*sdesc;
	xil_spi_desc	*xdesc;
	xil_spi_init	*xinit;

	sdesc = (struct spi_desc *)malloc(sizeof(*sdesc));
	xdesc = (struct xil_spi_desc *)malloc(sizeof(*xdesc));
	if(!sdesc || !xdesc)
	{
		free(sdesc);
		free(xdesc);
		return FAILURE;
	}

	sdesc->max_speed_hz = param->max_speed_hz;
	sdesc->chip_select = param->chip_select;
	sdesc->mode = param->mode;

	xinit = param->extra;
	sdesc->extra = xdesc;

	switch (xinit->type){
	case SPI_PL:
#ifdef XSPI_H
		xdesc->instance = (XSpi*))malloc(sizeof(*(xdesc->instance)));
		if(!xdesc->instance)
			goto error;

		xdesc->config = XSpi_LookupConfig(xinit->device_id);
		if(xdesc->config == NULL)
			goto error;

		//TODO: verify is this updated

		ret = XSpi_Initialize(&xil_descriptor->instance, xil_param->id);
		if (ret != 0)
			goto error;

		XSpi_SetOptions(&xil_descriptor->instance,
				XSP_MASTER_OPTION |
				((descriptor->mode & SPI_CPOL) ?
				XSP_CLK_ACTIVE_LOW_OPTION : 0) |
				((descriptor->mode & SPI_CPHA) ?
				XSP_CLK_PHASE_1_OPTION : 0));

		XSpi_Start(&xil_descriptor->instance);

		XSpi_IntrGlobalDisable(&xil_descriptor->instance);
#endif
		break;
	case SPI_PS:
#ifdef XSPIPS_H
		xdesc->instance = (XSpiPs*))malloc(sizeof(*(xdesc->instance)));
		if(!xdesc->instance)
			goto error;

		xdesc->config = XSpiPs_LookupConfig(xinit->device_id);
		if(xdesc->config == NULL)
			goto error;

		ret = XSpiPs_CfgInitialize(xdesc->instance,
					   xdesc->config,
					   ((XSpiPs_Config*)xdesc->config)
					   ->BaseAddress);
		if(ret != SUCCESS)
			goto error;

		XSpiPs_SetClkPrescaler(xdesc->instance,
				XSPIPS_CLK_PRESCALE_64);
		break;
#endif
		goto not_defined;

	case SPI_ENGINE:
#ifdef SPI_ENGINE_H
		//TODO: Implement SPI engine feature

		break;
#endif
		/* Intended fallthrough */
	default:
		goto not_defined;
		break;
	}

	*desc = sdesc;
	return SUCCESS;

	error:
		free(xdesc->instance);
		free(xdesc);
		free(sdesc);
	not_defined:
		return FAILURE;
}

/**
 * @brief Free the resources allocated by spi_init().
 * @param desc - The SPI descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_remove(struct spi_desc *desc)
{
	int32_t		ret;
	xil_spi_desc	*xdesc;

	xdesc = desc->extra;

	switch (xdesc->type){
	case SPI_PL:
#ifdef XSPI_H
		//TODO check if any aditional setup is needed here

#endif
		break;
	case SPI_PS:
#ifdef XSPIPS_H
		//TODO check if any aditional setup is needed here
#endif
		break;
	case SPI_ENGINE:
#ifdef SPI_ENGINE_H
		//TODO check if any aditional setup is needed here
#endif
		/* Intended fallthrough */
	error:
	default:
		return FAILURE;
		break;
	}

	free(xdesc->instance);
	free(desc->extra);
	free(desc);

	return SUCCESS;
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */

int32_t spi_write_and_read(struct spi_desc *desc,
			   uint8_t *data,
			   uint8_t bytes_number)
{
	int32_t		ret;
	xil_spi_desc	*xdesc;

	xdesc = desc->extra;

	switch (xdesc->type){
	case SPI_PL:
#ifdef XSPI_H
		//TODO: verify is this updated

		XSpi_SetOptions(xdesc->instance,
				XSP_MASTER_OPTION |
				((desc->mode & SPI_CPOL) ?
				XSP_CLK_ACTIVE_LOW_OPTION : 0) |
				((desc->mode & SPI_CPHA) ?
				XSP_CLK_PHASE_1_OPTION : 0));

		XSpi_SetSlaveSelect(xdesc->instance,
				    0x01 << desc->chip_select);

		XSpi_Transfer(xdesc->instance,
			      data,
			      data,
			      bytes_number);
#endif
		break;
	case SPI_PS:
#ifdef XSPIPS_H
		XSpiPs_SetOptions(xdesc->instance,
				  XSPIPS_MASTER_OPTION |
				  ((xdesc->flags & SPI_CS_DECODE) ?
				  XSPIPS_DECODE_SSELECT_OPTION : 0) |
				  XSPIPS_FORCE_SSELECT_OPTION |
				  ((desc->mode & SPI_CPOL) ?
				  XSPIPS_CLK_ACTIVE_LOW_OPTION : 0) |
				  ((desc->mode & SPI_CPHA) ?
				  XSPIPS_CLK_PHASE_1_OPTION : 0));

		XSpiPs_SetSlaveSelect(xdesc->instance,
				      desc->chip_select);
		XSpiPs_PolledTransfer(xdesc->instance,
				      data,
				      data,
				      bytes_number);
#endif
		break;
	case SPI_ENGINE:
#ifdef SPI_ENGINE_H

#endif
		/* Intended fallthrough */
	error:
	default:
		return FAILURE;
		break;
	}

	return SUCCESS;
}
