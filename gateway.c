const char gateway_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Contains functions to connect to a server, possibly
 *                using a "forwarder" (i.e. HTTP proxy and/or a SOCKS4
 *                proxy).
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
 *    Revision 1.2  2001/06/07 23:11:38  jongfoster
 *    Removing gateways[] list - no longer used.
 *    Replacing function pointer in struct gateway with a directly
 *    called function forwarded_connect(), which can do the common
 *    task of deciding whether to connect to the web server or HTTP
 *    proxy.
 *    Replacing struct gateway with struct forward_spec
 *    Fixing bug with SOCKS4A and HTTP proxy server in combination.
 *    It was a bug which led to the connection being made to the web
 *    server rather than the HTTP proxy, and also a buffer overrun.
 *
 *    Revision 1.1.1.1  2001/05/15 13:58:54  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#endif /* def _WIN32 */

#include "jcc.h"
#include "errlog.h"
#include "jbsockets.h"
#include "gateway.h"

const char gateway_h_rcs[] = GATEWAY_H_VERSION;

static int socks4_connect(const struct forward_spec * fwd, 
                          const char * target_host,
                          int target_port,
                          struct client_state *csp);


#define SOCKS_REQUEST_GRANTED          90
#define SOCKS_REQUEST_REJECT           91
#define SOCKS_REQUEST_IDENT_FAILED     92
#define SOCKS_REQUEST_IDENT_CONFLICT   93

/* structure of a socks client operation */
struct socks_op {
   unsigned char vn;          /* socks version number */
   unsigned char cd;          /* command code */
   unsigned char dstport[2];  /* destination port */
   unsigned char dstip[4];    /* destination address */
   unsigned char userid;      /* first byte of userid */
   /* more bytes of the userid follow, terminated by a NULL */
};

/* structure of a socks server reply */
struct socks_reply {
   unsigned char vn;          /* socks version number */
   unsigned char cd;          /* command code */
   unsigned char dstport[2];  /* destination port */
   unsigned char dstip[4];    /* destination address */
};

static const char socks_userid[] = "anonymous";


/*********************************************************************
 *
 * Function    :  forwarded_connect
 *
 * Description :  Connect to a specified web server, possibly via
 *                a HTTP proxy and/or a SOCKS proxy.
 *
 * Parameters  :
 *          1  :  gw = pointer to a gateway structure (such as gw_default)
 *          2  :  http = the http request and apropos headers
 *          3  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  -1 => failure, else it is the socket file descriptor.
 *
 *********************************************************************/
int forwarded_connect(const struct forward_spec * fwd, 
                      struct http_request *http, 
                      struct client_state *csp)
{
   const char * dest_host;
   int dest_port;

   /* Figure out if we need to connect to the web server or a HTTP proxy. */
   if (fwd->forward_host)
   {
      /* HTTP proxy */
      dest_host = fwd->forward_host;
      dest_port = fwd->forward_port;
   }
   else
   {
      /* Web server */
      dest_host = http->host;
      dest_port = http->port;
   }

   /* Connect, maybe using a SOCKS proxy */
   switch (fwd->type)
   {
      case SOCKS_NONE:
         return (connect_to(dest_host, dest_port, csp));

      case SOCKS_4:
      case SOCKS_4A:
         return (socks4_connect(fwd, dest_host, dest_port, csp));

      default:
         /* Should never get here */
         log_error(LOG_LEVEL_FATAL, "SOCKS4 impossible internal error - bad SOCKS type.");
         errno = EINVAL;
         return(-1);
   }
}


/*********************************************************************
 *
 * Function    :  socks4_connect
 *
 * Description :  Connect to the SOCKS server, and connect through
 *                it to the specified server.   This handles
 *                all the SOCKS negotiation, and returns a file
 *                descriptor for a socket which can be treated as a
 *                normal (non-SOCKS) socket.
 *
 * Parameters  :
 *          1  :  gw = pointer to a gateway structure (such as gw_default)
 *          2  :  http = the http request and apropos headers
 *          3  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  -1 => failure, else a socket file descriptor.
 *
 *********************************************************************/
static int socks4_connect(const struct forward_spec * fwd, 
                          const char * target_host,
                          int target_port,
                          struct client_state *csp)
{
   int web_server_addr;
   unsigned char cbuf[BUFSIZ];
   unsigned char sbuf[BUFSIZ];
   struct socks_op    *c = (struct socks_op    *)cbuf;
   struct socks_reply *s = (struct socks_reply *)sbuf;
   int n;
   int csiz;
   int sfd;
   int err = 0;
   char *errstr;

   if ((fwd->gateway_host == NULL) || (*fwd->gateway_host == '\0'))
   {
      log_error(LOG_LEVEL_CONNECT, "socks4_connect: NULL gateway host specified");
      err = 1;
   }

   if (fwd->gateway_port <= 0)
   {
      log_error(LOG_LEVEL_CONNECT, "socks4_connect: invalid gateway port specified");
      err = 1;
   }

   if (err)
   {
      errno = EINVAL;
      return(-1);
   }

   /* build a socks request for connection to the web server */

   strcpy((char *)&(c->userid), socks_userid);

   csiz = sizeof(*c) + sizeof(socks_userid) - 1;

   switch (fwd->type)
   {
      case SOCKS_4:
         web_server_addr = htonl(resolve_hostname_to_ip(target_host));
         break;
      case SOCKS_4A:
         web_server_addr = 0x00000001;
         n = csiz + strlen(target_host) + 1;
         if (n > sizeof(cbuf))
         {
            errno = EINVAL;
            return(-1);
         }
         strcpy(((char *)cbuf) + csiz, target_host);
         csiz = n;
         break;
      default:
         /* Should never get here */
         log_error(LOG_LEVEL_FATAL, "SOCKS4 impossible internal error - bad SOCKS type.");
         errno = EINVAL;
         return(-1);
   }

   c->vn          = 4;
   c->cd          = 1;
   c->dstport[0]  = (target_port       >> 8  ) & 0xff;
   c->dstport[1]  = (target_port             ) & 0xff;
   c->dstip[0]    = (web_server_addr   >> 24 ) & 0xff;
   c->dstip[1]    = (web_server_addr   >> 16 ) & 0xff;
   c->dstip[2]    = (web_server_addr   >>  8 ) & 0xff;
   c->dstip[3]    = (web_server_addr         ) & 0xff;

   /* pass the request to the socks server */
   sfd = connect_to(fwd->gateway_host, fwd->gateway_port, csp);

   if (sfd < 0)
   {
      return(-1);
   }

   if ((n = write_socket(sfd, (char *)c, csiz)) != csiz)
   {
      log_error(LOG_LEVEL_CONNECT, "SOCKS4 negotiation write failed...");
      close_socket(sfd);
      return(-1);
   }

   if ((n = read_socket(sfd, sbuf, sizeof(sbuf))) != sizeof(*s))
   {
      log_error(LOG_LEVEL_CONNECT, "SOCKS4 negotiation read failed...");
      close_socket(sfd);
      return(-1);
   }

   switch (s->cd)
   {
      case SOCKS_REQUEST_GRANTED:
         return(sfd);
         break;
      case SOCKS_REQUEST_REJECT:
         errstr = "SOCKS request rejected or failed";
         errno = EINVAL;
         break;
      case SOCKS_REQUEST_IDENT_FAILED:
         errstr = "SOCKS request rejected because "
            "SOCKS server cannot connect to identd on the client";
         errno = EACCES;
         break;
      case SOCKS_REQUEST_IDENT_CONFLICT:
         errstr = "SOCKS request rejected because "
            "the client program and identd report "
            "different user-ids";
         errno = EACCES;
         break;
      default:
         errstr = (char *) cbuf;
         errno = ENOENT;
         sprintf(errstr,
                 "SOCKS request rejected for reason code %d\n", s->cd);
   }

   log_error(LOG_LEVEL_CONNECT, "socks4_connect: %s ...", errstr);

   close_socket(sfd);
   return(-1);

}


/*
  Local Variables:
  tab-width: 3
  end:
*/
