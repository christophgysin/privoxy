#ifndef PARSERS_H_INCLUDED
#define PARSERS_H_INCLUDED
#define PARSERS_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Declares functions to parse/crunch headers and pages.
 *                Functions declared include:
 *                   `add_to_iob', `client_cookie_adder', `client_from',
 *                   `client_referrer', `client_send_cookie', `client_ua',
 *                   `client_uagent', `client_x_forwarded',
 *                   `client_x_forwarded_adder', `client_xtra_adder',
 *                   `content_type', `crumble', `destroy_list', `enlist',
 *                   `flush_socket', `free_http_request', `get_header',
 *                   `list_to_text', `parse_http_request', `sed',
 *                   and `server_set_cookie'.
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
 *    Revision 1.15  2001/10/07 18:01:55  oes
 *    Changed server_http11 to server_http
 *
 *    Revision 1.14  2001/10/07 15:45:48  oes
 *    added client_accept_encoding, client_te, client_accept_encoding_adder
 *
 *    renamed content_type and content_length
 *
 *    fixed client_host and strclean prototypes
 *
 *    Revision 1.13  2001/09/29 12:56:03  joergs
 *    IJB now changes HTTP/1.1 to HTTP/1.0 in requests and answers.
 *
 *    Revision 1.12  2001/09/13 23:05:50  jongfoster
 *    Changing the string paramater to the header parsers a "const".
 *
 *    Revision 1.11  2001/07/31 14:46:53  oes
 *    Added prototype for connection_close_adder
 *
 *    Revision 1.10  2001/07/30 22:08:36  jongfoster
 *    Tidying up #defines:
 *    - All feature #defines are now of the form FEATURE_xxx
 *    - Permanently turned off WIN_GUI_EDIT
 *    - Permanently turned on WEBDAV and SPLIT_PROXY_ARGS
 *
 *    Revision 1.9  2001/07/29 18:43:08  jongfoster
 *    Changing #ifdef _FILENAME_H to FILENAME_H_INCLUDED, to conform to
 *    ANSI C rules.
 *
 *    Revision 1.8  2001/07/13 14:01:54  oes
 *    Removed all #ifdef PCRS
 *
 *    Revision 1.7  2001/06/29 13:32:14  oes
 *    Removed logentry from cancelled commit
 *
 *    Revision 1.6  2001/06/03 19:12:38  oes
 *    deleted const struct interceptors
 *
 *    Revision 1.5  2001/05/31 21:30:33  jongfoster
 *    Removed list code - it's now in list.[ch]
 *    Renamed "permission" to "action", and changed many features
 *    to use the actions file rather than the global config.
 *
 *    Revision 1.4  2001/05/27 13:19:06  oes
 *    Patched Joergs solution for the content-length in.
 *
 *    Revision 1.3  2001/05/26 13:39:32  jongfoster
 *    Only crunches Content-Length header if applying RE filtering.
 *    Without this fix, Microsoft Windows Update wouldn't work.
 *
 *    Revision 1.2  2001/05/20 01:21:20  jongfoster
 *    Version 2.9.4 checkin.
 *    - Merged popupfile and cookiefile, and added control over PCRS
 *      filtering, in new "permissionsfile".
 *    - Implemented LOG_LEVEL_FATAL, so that if there is a configuration
 *      file error you now get a message box (in the Win32 GUI) rather
 *      than the program exiting with no explanation.
 *    - Made killpopup use the PCRS MIME-type checking and HTTP-header
 *      skipping.
 *    - Removed tabs from "config"
 *    - Moved duplicated url parsing code in "loaders.c" to a new funcition.
 *    - Bumped up version number.
 *
 *    Revision 1.1.1.1  2001/05/15 13:59:01  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const struct parsers client_patterns[];
extern const struct parsers server_patterns[];

extern void (* const add_client_headers[])(struct client_state *);
extern void (* const add_server_headers[])(struct client_state *);

extern int flush_socket(int fd, struct client_state *csp);
extern int add_to_iob(struct client_state *csp, char *buf, int n);
extern char *get_header(struct client_state *csp);

extern char *sed(const struct parsers pats[], void (* const more_headers[])(struct client_state *), struct client_state *csp);

extern void free_http_request(struct http_request *http);
extern void parse_http_request(char *req, struct http_request *http, struct client_state *csp);

extern char *crumble(const struct parsers *v, const char *s, struct client_state *csp);

extern char *client_referrer(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_uagent(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_ua(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_from(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_send_cookie(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_x_forwarded(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_accept_encoding(const struct parsers *v, const char *s, struct client_state *csp);
extern char *client_te(const struct parsers *v, const char *s, struct client_state *csp);

extern void client_cookie_adder(struct client_state *csp);
extern void client_xtra_adder(struct client_state *csp);
extern void client_accept_encoding_adder(struct client_state *csp);
extern void client_x_forwarded_adder(struct client_state *csp);

extern void connection_close_adder(struct client_state *csp); 

extern char *server_set_cookie(const struct parsers *v, const char *s, struct client_state *csp);
extern char *server_content_type(const struct parsers *v, const char *s, struct client_state *csp);
extern char *server_content_length(const struct parsers *v, const char *s, struct client_state *csp);
extern char *server_content_md5(const struct parsers *v, const char *s, struct client_state *csp);
extern char *server_transfer_encoding(const struct parsers *v, const char *s, struct client_state *csp);
extern char *server_http(const struct parsers *v, const char *s, struct client_state *csp);

#ifdef FEATURE_FORCE_LOAD
extern char *client_host(const struct parsers *v, const char *s, struct client_state *csp);
extern int strclean(const char *string, const char *substring);
#endif /* def FEATURE_FORCE_LOAD */

#if defined(FEATURE_IMAGE_DETECT_MSIE)
extern char *client_accept(const struct parsers *v, const char *s, struct client_state *csp);
#endif /* defined(FEATURE_IMAGE_DETECT_MSIE) */

/* Revision control strings from this header and associated .c file */
extern const char parsers_rcs[];
extern const char parsers_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef PARSERS_H_INCLUDED */

/*
  Local Variables:
  tab-width: 3
  end:
*/
