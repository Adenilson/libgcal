/*
Copyright (c) 2008 Instituto Nokia de Tecnologia
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the INdT nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @file   gcal_status.h
 * @author Adenilson Cavalcanti <adenilson.silva@indt.org.br>
 * @date   Fri Jun 13 15:02:02 2008
 *
 * @brief  Status and debugging auxiliary functions.
 *
 * Use then to check internal status when something failed.
 */

#ifndef __GCAL_STATUS_LIB__
#define __GCAL_STATUS_LIB__

#include "gcal.h"

/** Returns HTTP status of a gcal object.
 *
 * Use it to check for status of object after last HTTP request.
 *
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource.
 *
 * @return The HTTP code (can be 200, 201, 301, etc).
 */
int gcal_status_httpcode(struct gcal_resource *ptr_gcal);


/** Sets a file name to use as logging.
 *
 * Use it to enable file logging in libgcal
 *
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource.
 *
 * @param filepath The file path (e.g.: "/tmp/libgcal.log")
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_status_setlog(struct gcal_resource *ptr_gcal, char *filepath);


/** Returns internal error messages from gcal object.
 *
 * Use it to check for information of object after last HTTP request.
 *
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource.
 *
 * @return NULL if everything is ok, pointer to string with error message.
 */
const char *gcal_status_msg(struct gcal_resource *ptr_gcal);
#endif
