/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
#ifndef _CONFIG_H
#define _CONFIG_H
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  This file should be the first thing included in every
 *                .c file.  (Before even system headers).  It contains 
 *                #define statements for various features.  It was
 *                introduced because the compile command line started
 *                getting ludicrously long with feature defines.
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
 *    Revision 1.6  2001/05/29 09:50:24  jongfoster
 *    Unified blocklist/imagelist/permissionslist.
 *    File format is still under discussion, but the internal changes
 *    are (mostly) done.
 *
 *    Also modified interceptor behaviour:
 *    - We now intercept all URLs beginning with one of the following
 *      prefixes (and *only* these prefixes):
 *        * http://i.j.b/
 *        * http://ijbswa.sf.net/config/
 *        * http://ijbswa.sourceforge.net/config/
 *    - New interceptors "home page" - go to http://i.j.b/ to see it.
 *    - Internal changes so that intercepted and fast redirect pages
 *      are not replaced with an image.
 *    - Interceptors now have the option to send a binary page direct
 *      to the client. (i.e. ijb-send-banner uses this)
 *    - Implemented show-url-info interceptor.  (Which is why I needed
 *      the above interceptors changes - a typical URL is
 *      "http://i.j.b/show-url-info?url=www.somesite.com/banner.gif".
 *      The previous mechanism would not have intercepted that, and
 *      if it had been intercepted then it then it would have replaced
 *      it with an image.)
 *
 *    Revision 1.5  2001/05/26 00:28:36  jongfoster
 *    Automatic reloading of config file.
 *    Removed obsolete SIGHUP support (Unix) and Reload menu option (Win32).
 *    Most of the global variables have been moved to a new
 *    struct configuration_spec, accessed through csp->config->globalname
 *    Most of the globals remaining are used by the Win32 GUI.
 *
 *    Revision 1.4  2001/05/25 22:17:28  jongfoster
 *    Resurrecting these files which are required for the MS Visual C++
 *    build only.
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
 *    Revision 1.1.1.1  2001/05/15 13:58:49  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/*
 * Version number - Major (X._._)
 */
#define VERSION_MAJOR 2

/*
 * Version number - Minor (_.X._)
 */
#define VERSION_MINOR 9

/*
 * Version number - Point (_._.X)
 */
#define VERSION_POINT 4

/*
 * Version number, as a string
 */
#define VERSION "2.9.4"

/*
 * Regular expression matching for URLs.  (Highly recommended).  If this is 
 * not defined then you can ony use prefix matching.
 */
#define REGEX 1

/*
 * Allow JunkBuster to be "disabled" so it is just a normal non-blocking
 * non-anonymizing proxy.  This is useful if you're trying to access a
 * blocked or broken site - just change the setting in the config file
 * and send a SIGHUP (UN*X), or use the handy "Disable" menu option (Windows
 * GUI).
 */
#define TOGGLE 1

/*
 * Enables arbitrary content modification regexps
 */
#define PCRS 1

/*
 * If a stream is compressed via gzip (Netscape specific I think), then
 * it cannot be modified with Perl regexps.  This forces it to be 
 * uncompressed.
 */
#define DENY_GZIP 1

/*
 * Enables statistics function.
 */
#define STATISTICS 1

/*
 * Bypass filtering for 1 page only
 */
#define FORCE_LOAD 1

/*
 * Locally redirect remote script-redirect URLs
 */
#define FAST_REDIRECTS 1

/*
 * Split the show-proxy-args page into a page for each config file.
 */
#define SPLIT_PROXY_ARGS 1

/*
 * Kills JavaScript popups - window.open, onunload, etc.
 */
#define KILLPOPUPS 1

/*
 * Support for webDAV - e.g. so Microsoft Outlook can access HotMail e-mail
 */
#define WEBDAV 1

/*
 * Detect image requests automatically for MSIE.  Will fall back to
 * other image-detection methods (i.e. "+image" permission) for other
 * browsers.
 *
 * You must also define IMAGE_BLOCKING to use this feature.
 *
 * It detects the following header pair as an image request:
 *
 * User-Agent: Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)
 * Accept: * / *
 *
 * And the following as a HTML request:
 *
 * User-Agent: Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)
 * Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, * / *
 *
 * And no, I haven't got that backwards - IE is being wierd.
 *
 * Known limitations: 
 * 1) If you press shift-reload on a blocked HTML page, you get
 *    the image "blocked" page, not the HTML "blocked" page.
 * 2) Once an image "blocked" page has been sent, viewing it 
 *    in it's own browser window *should* bring up the HTML
 *    "blocked" page, but it doesn't.  You need to clear the 
 *    browser cache to get the HTML version again.
 *
 * These limitations are due to IE making inconsistent choices
 * about which "Accept:" header to send.
 */
#define DETECT_MSIE_IMAGES 1

/*
 * Allow blocking using images as well as HTML.
 * If you do not define this then everything is blocked as HTML.
 *
 * Note that this is required if you want to use DETECT_MSIE_IMAGES.
 */
#define IMAGE_BLOCKING 1

/*
 * Allows the use of ACL files to control access to the proxy by IP address.
 */
#define ACL_FILES 1

/*
 * Allows the use of trust files.
 */
#define TRUST_FILES 1

/*
 * Allows the use of jar files to capture cookies.
 */
#define JAR_FILES 1

/*
 * Use PCRE rather than GNU Regex
 */
#define PCRE 1

/* Define if you have the bcopy function.  */
#define HAVE_BCOPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

#endif /* _CONFIG_H */
