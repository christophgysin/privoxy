#ifndef _GATEWAY_H
#define _GATEWAY_H
#define GATEWAY_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Contains functions to connect to a server, possibly
 *                using a "gateway" (i.e. HTTP proxy and/or SOCKS4
 *                proxy).  Also contains the list of gateway types.
 *
 * Copyright   :  Written by and Copyright (C) 2001 the SourceForge
 *                IJBSWA team.  http://ijbswa.sourceforge.net
 *
 *                Based on the Internet Junkbuster originally written
 *                by and Copyright (C) 1997 Anonymous Coders and 
 *                Junkbusters Corporation.  http://www.junkbusters.com
 *
 *                This program is free software; you can redistribute it 
 *                and/or modify it under the terms of the GNU General
 *                Public License as published by the Free Software
 *                Foundation; either version 2 of the License, or (at
 *                your option) any later version.
 *
 *                This program is distributed in the hope that it will
 *                be useful, but WITHOUT ANY WARRANTY; without even the
 *                implied warranty of MERCHANTABILITY or FITNESS FOR A
 *                PARTICULAR PURPOSE.  See the GNU General Public
 *                License for more details.
 *
 *                The GNU General Public License should be included with
 *                this file.  If not, you can view it at
 *                http://www.gnu.org/copyleft/gpl.html
 *                or write to the Free Software Foundation, Inc., 59
 *                Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Revisions   :
 *    $Log$
 *    Revision 1.1  2001/05/15 13:58:54  oes
 *    Initial revision
 *
 *
 *********************************************************************/


#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const struct gateway gateways[];
extern const struct gateway *gw_default;

extern int socks4_connect(const struct gateway *gw, struct http_request *http, struct client_state *csp);
extern int direct_connect(const struct gateway *gw, struct http_request *http, struct client_state *csp);

/* Revision control strings from this header and associated .c file */
extern const char gateway_rcs[];
extern const char gateway_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef _GATEWAY_H */

/*
  Local Variables:
  tab-width: 3
  end:
*/
