#ifndef _FILTERS_H
#define _FILTERS_H
#define FILTERS_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Declares functions to parse/crunch headers and pages.
 *                Functions declared include:
 *                   `acl_addr', `add_stats', `block_acl', `block_imageurl',
 *                   `block_url', `url_permissions', `domaincmp', `dsplit',
 *                   `filter_popups', `forward_url'
 *                   `ij_untrusted_url', `intercept_url', `re_process_buffer',
 *                   `show_proxy_args', and `trust_url'
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
 *    Revision 1.5  2001/05/27 22:17:04  oes
 *
 *    - re_process_buffer no longer writes the modified buffer
 *      to the client, which was very ugly. It now returns the
 *      buffer, which it is then written by chat.
 *
 *    - content_length now adjusts the Content-Length: header
 *      for modified documents rather than crunch()ing it.
 *      (Length info in csp->content_length, which is 0 for
 *      unmodified documents)
 *
 *    - For this to work, sed() is called twice when filtering.
 *
 *    Revision 1.4  2001/05/26 15:26:15  jongfoster
 *    ACL feature now provides more security by immediately dropping
 *    connections from untrusted hosts.
 *
 *    Revision 1.3  2001/05/22 18:46:04  oes
 *
 *    - Enabled filtering banners by size rather than URL
 *      by adding patterns that replace all standard banner
 *      sizes with the "Junkbuster" gif to the re_filterfile
 *
 *    - Enabled filtering WebBugs by providing a pattern
 *      which kills all 1x1 images
 *
 *    - Added support for PCRE_UNGREEDY behaviour to pcrs,
 *      which is selected by the (nonstandard and therefore
 *      capital) letter 'U' in the option string.
 *      It causes the quantifiers to be ungreedy by default.
 *      Appending a ? turns back to greedy (!).
 *
 *    - Added a new interceptor ijb-send-banner, which
 *      sends back the "Junkbuster" gif. Without imagelist or
 *      MSIE detection support, or if tinygif = 1, or the
 *      URL isn't recognized as an imageurl, a lame HTML
 *      explanation is sent instead.
 *
 *    - Added new feature, which permits blocking remote
 *      script redirects and firing back a local redirect
 *      to the browser.
 *      The feature is conditionally compiled, i.e. it
 *      can be disabled with --disable-fast-redirects,
 *      plus it must be activated by a "fast-redirects"
 *      line in the config file, has its own log level
 *      and of course wants to be displayed by show-proxy-args
 *      Note: Boy, all the #ifdefs in 1001 locations and
 *      all the fumbling with configure.in and acconfig.h
 *      were *way* more work than the feature itself :-(
 *
 *    - Because a generic redirect template was needed for
 *      this, tinygif = 3 now uses the same.
 *
 *    - Moved GIFs, and other static HTTP response templates
 *      to project.h
 *
 *    - Some minor fixes
 *
 *    - Removed some >400 CRs again (Jon, you really worked
 *      a lot! ;-)
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
 *    Revision 1.1.1.1  2001/05/15 13:58:52  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ACL_FILES
extern int block_acl(struct access_control_addr *dst, struct client_state *csp);
extern int acl_addr(char *aspec, struct access_control_addr *aca);
#endif /* def ACL_FILES */

extern char *block_url(struct http_request *http, struct client_state *csp);
#ifdef TRUST_FILES
extern char *trust_url(struct http_request *http, struct client_state *csp);
#endif /* def TRUST_FILES */
extern char *intercept_url(struct http_request *http, struct client_state *csp);
extern char *redirect_url(struct http_request *http, struct client_state *csp);

#if defined(DETECT_MSIE_IMAGES) || defined(USE_IMAGE_LIST)
extern int block_imageurl(struct http_request *http, struct client_state *csp);
#endif /* defined(DETECT_MSIE_IMAGES) || defined(USE_IMAGE_LIST) */

#ifdef USE_IMAGE_LIST
extern int block_imageurl_using_imagelist(struct http_request *http, struct client_state *csp);
#endif /* def USE_IMAGE_LIST */

extern int url_permissions(struct http_request *http, struct client_state *csp);
extern const struct gateway *forward_url(struct http_request *http, struct client_state *csp);

extern struct url_spec dsplit(char *domain);
extern int domaincmp(struct url_spec *pattern, struct url_spec *fqdn);

extern char *show_proxy_args(struct http_request *http, struct client_state *csp);
extern char *ijb_send_banner(struct http_request *http, struct client_state *csp);

#ifdef TRUST_FILES
extern char *ij_untrusted_url(struct http_request *http, struct client_state *csp);
#endif /* def TRUST_FILES */

#ifdef STATISTICS
extern char *add_stats(char *s);
#endif /* def STATISTICS */

#ifdef PCRS
extern char *re_process_buffer(struct client_state *csp);
#endif /* def PCRS */

/* Revision control strings from this header and associated .c file */
extern const char filters_rcs[];
extern const char filters_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef _FILTERS_H */

/*
  Local Variables:
  tab-width: 3
  end:
*/
