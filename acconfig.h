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
 *    Revision 1.7  2001/07/25 22:53:59  jongfoster
 *    Will #error if pthreads is enabled under BeOs
 *
 *    Revision 1.6  2001/07/15 17:54:29  jongfoster
 *    Renaming #define STATIC to STATIC_PCRE
 *    Adding new #define FEATURE_PTHREAD that will be used to enable
 *    POSIX threads support.
 *
 *    Revision 1.5  2001/07/13 13:48:37  oes
 *     - (Fix:) Copied CODE_STATUS #define from config.h.in
 *     - split REGEX #define into REGEX_GNU and REGEX_PCRE
 *       and removed PCRE.
 *       (REGEX = REGEX_GNU || REGEX_PCRE per project.h)
 *     - Moved STATIC (for pcre) here from Makefile.in
 *     - Introduced STATIC_PCRS #define to allow for dynaimc linking with
 *       libpcrs
 *     - Removed PCRS #define, since pcrs is now needed for CGI anyway
 *
 *    Revision 1.4  2001/05/29 09:50:24  jongfoster
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
 *    Revision 1.3  2001/05/26 01:26:34  jongfoster
 *    New #define, WIN_GUI_EDIT, enables the (embryonic) Win32 GUI editor.
 *    This #define cannot be set from ./configure - there's no point, it
 *    doesn't work yet.  See feature request # 425722
 *
 *    Revision 1.2  2001/05/22 17:43:35  oes
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
 *    - Many minor fixes
 *
 *    - Removed some >400 CRs again (Jon, you really worked
 *      a lot! ;-)
 *
 *    Revision 1.1.1.1  2001/05/15 13:58:45  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/

@TOP@

/*
 * Version number - Major (X._._)
 */
#undef VERSION_MAJOR

/*
 * Version number - Minor (_.X._)
 */
#undef VERSION_MINOR

/*
 * Version number - Point (_._.X)
 */
#undef VERSION_POINT

/*
 * Version number, as a string
 */
#undef VERSION

/*
 * Status of the code: alpha, beta or stable
 */
#undef CODE_STATUS

/*
 * Regular expression matching for URLs.  (Highly recommended).
 * If neither of these are defined then you can ony use prefix matching.
 * Don't bother to change this here! Use configure instead.
 */
#undef REGEX_GNU
#undef REGEX_PCRE

/* 
 * Should pcre be statically built in instead of linkling with libpcre?
 * (This is determined by configure depending on the availiability of
 * libpcre and user preferences). The name is ugly, but pcre needs it.
 * Don't bother to change this here! Use configure instead.
 */
#undef STATIC_PCRE

/* 
 * Should pcrs be statically built in instead of linkling with libpcrs?
 * (This is determined by configure depending on the availiability of
 * libpcrs and user preferences).
 * Don't bother to change this here! Use configure instead.
 */
#undef STATIC_PCRS

/*
 * Allow JunkBuster to be "disabled" so it is just a normal non-blocking
 * non-anonymizing proxy.  This is useful if you're trying to access a
 * blocked or broken site - just change the setting in the config file
 * and send a SIGHUP (UN*X), or use the handy "Disable" menu option (Windows
 * GUI).
 */
#undef TOGGLE

/*
 * If a stream is compressed via gzip (Netscape specific I think), then
 * it cannot be modified with Perl regexps.  This forces it to be 
 * uncompressed.
 */
#undef DENY_GZIP

/*
 * Enables statistics function.
 */
#undef STATISTICS

/*
 * Bypass filtering for 1 page only
 */
#undef FORCE_LOAD

/*
 * Locally redirect remote script-redirect URLs
 */
#undef FAST_REDIRECTS

/*
 * Split the show-proxy-args page into a page for each config file.
 */
#undef SPLIT_PROXY_ARGS

/*
 * Kills JavaScript popups - window.open, onunload, etc.
 */
#undef KILLPOPUPS

/*
 * Support for webDAV - e.g. so Microsoft Outlook can access HotMail e-mail
 */
#undef WEBDAV

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
#undef DETECT_MSIE_IMAGES

/*
 * Allow blocking using images as well as HTML.
 * If you do not define this then everything is blocked as HTML.
 *
 * Note that this is required if you want to use DETECT_MSIE_IMAGES.
 */
#undef IMAGE_BLOCKING

/*
 * Allows the use of ACL files to control access to the proxy by IP address.
 */
#undef ACL_FILES

/*
 * Allows the use of trust files.
 */
#undef TRUST_FILES

/*
 * Allows the use of jar files to capture cookies.
 */
#undef JAR_FILES

/*
 * Define this to use the Windows GUI for editing the blocklist.
 * FIXME: This feature is only partially implemented and does not work
 * FIXME: This #define can never be set by ./configure.
 */
#undef WIN_GUI_EDIT

/*
 * Use POSIX threads instead of native threads.
 */
#undef FEATURE_PTHREAD

@BOTTOM@

/*
 * Need to set up this define only for the Pthreads library for
 * Win32, available from http://sources.redhat.com/pthreads-win32/
 */
#if defined(FEATURE_PTHREAD) && defined(_WIN32)
#define __CLEANUP_C
#endif /* defined(FEATURE_PTHREAD) && defined(_WIN32) */

/*
 * BEOS does not currently support POSIX threads.
 * This *should* be detected by ./configure, but let's be sure.
 */
#if defined(FEATURE_PTHREAD) && defined(__BEOS__)
#error BEOS does not support pthread - please run ./configure again with "--disable-pthread"

#endif /* defined(FEATURE_PTHREAD) && defined(__BEOS__) */


#endif /* _CONFIG_H */
