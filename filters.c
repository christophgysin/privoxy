const char filters_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Declares functions to parse/crunch headers and pages.
 *                Functions declared include:
 *                   `acl_addr', `add_stats', `block_acl', `block_imageurl',
 *                   `block_url', `url_actions', `domaincmp', `dsplit',
 *                   `filter_popups', `forward_url', 'redirect_url',
 *                   `ij_untrusted_url', `intercept_url', `re_process_buffer',
 *                   `show_proxy_args', 'ijb_send_banner', and `trust_url'
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
 *    Revision 1.17  2001/06/09 10:55:28  jongfoster
 *    Changing BUFSIZ ==> BUFFER_SIZE
 *
 *    Revision 1.16  2001/06/07 23:10:26  jongfoster
 *    Allowing unanchored domain patterns to back off and retry
 *    if they partially match.  Optimized right-anchored patterns.
 *    Moving ACL and forward files into config file.
 *    Replacing struct gateway with struct forward_spec
 *
 *    Revision 1.15  2001/06/03 19:12:00  oes
 *    extracted-CGI relevant stuff
 *
 *    Revision 1.15  2001/06/03 11:03:48  oes
 *    Makefile/in
 *
 *    introduced cgi.c
 *
 *    actions.c:
 *
 *    adapted to new enlist_unique arg format
 *
 *    conf loadcfg.c
 *
 *    introduced confdir option
 *
 *    filters.c filtrers.h
 *
 *     extracted-CGI relevant stuff
 *
 *    jbsockets.c
 *
 *     filled comment
 *
 *    jcc.c
 *
 *     support for new cgi mechansim
 *
 *    list.c list.h
 *
 *    functions for new list type: "map"
 *    extended enlist_unique
 *
 *    miscutil.c .h
 *    introduced bindup()
 *
 *    parsers.c parsers.h
 *
 *    deleted const struct interceptors
 *
 *    pcrs.c
 *    added FIXME
 *
 *    project.h
 *
 *    added struct map
 *    added struct http_response
 *    changes struct interceptors to struct cgi_dispatcher
 *    moved HTML stuff to cgi.h
 *
 *    re_filterfile:
 *
 *    changed
 *
 *    showargs.c
 *    NO TIME LEFT
 *
 *    Revision 1.14  2001/06/01 10:30:55  oes
 *    Added optional left-anchoring to domaincmp
 *
 *    Revision 1.13  2001/05/31 21:21:30  jongfoster
 *    Permissionsfile / actions file changes:
 *    - Changed "permission" to "action" throughout
 *    - changes to file format to allow string parameters
 *    - Moved helper functions to actions.c
 *
 *    Revision 1.12  2001/05/31 17:35:20  oes
 *
 *     - Enhanced domain part globbing with infix and prefix asterisk
 *       matching and optional unanchored operation
 *
 *    Revision 1.11  2001/05/29 11:53:23  oes
 *    "See why" link added to "blocked" page
 *
 *    Revision 1.10  2001/05/29 09:50:24  jongfoster
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
 *    Revision 1.9  2001/05/27 22:17:04  oes
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
 *    Revision 1.8  2001/05/26 17:13:28  jongfoster
 *    Filled in a function comment.
 *
 *    Revision 1.7  2001/05/26 15:26:15  jongfoster
 *    ACL feature now provides more security by immediately dropping
 *    connections from untrusted hosts.
 *
 *    Revision 1.6  2001/05/26 00:28:36  jongfoster
 *    Automatic reloading of config file.
 *    Removed obsolete SIGHUP support (Unix) and Reload menu option (Win32).
 *    Most of the global variables have been moved to a new
 *    struct configuration_spec, accessed through csp->config->globalname
 *    Most of the globals remaining are used by the Win32 GUI.
 *
 *    Revision 1.5  2001/05/25 22:34:30  jongfoster
 *    Hard tabs->Spaces
 *
 *    Revision 1.4  2001/05/22 18:46:04  oes
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
 *    Revision 1.3  2001/05/20 16:44:47  jongfoster
 *    Removing last hardcoded JunkBusters.com URLs.
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


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include "project.h"
#include "filters.h"
#include "encode.h"
#include "jcc.h"
#include "showargs.h"
#include "parsers.h"
#include "ssplit.h"
#include "gateway.h"
#include "jbsockets.h"
#include "errlog.h"
#include "jbsockets.h"
#include "miscutil.h"
#include "actions.h"
#include "cgi.h"

#ifdef _WIN32
#include "win32.h"
#endif

const char filters_h_rcs[] = FILTERS_H_VERSION;

/* Fix a problem with Solaris.  There should be no effect on other
 * platforms.
 * Solaris's isspace() is a macro which uses it's argument directly
 * as an array index.  Therefore we need to make sure that high-bit
 * characters generate +ve values, and ideally we also want to make
 * the argument match the declared parameter type of "int".
 */
#define ijb_isdigit(__X) isdigit((int)(unsigned char)(__X))


#ifdef ACL_FILES
/*********************************************************************
 *
 * Function    :  block_acl
 *
 * Description :  Block this request?
 *                Decide yes or no based on ACL file.
 *
 * Parameters  :
 *          1  :  dst = The proxy or gateway address this is going to.
 *                      Or NULL to check all possible targets.
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *                      Also includes the client IP address.
 *
 * Returns     : 0 = FALSE (don't block) and 1 = TRUE (do block)
 *
 *********************************************************************/
int block_acl(struct access_control_addr *dst,
              struct client_state *csp)
{
   struct access_control_list *acl = csp->config->acl;

   /* if not using an access control list, then permit the connection */
   if (acl == NULL)
   {
      return(0);
   }

   /* search the list */
   while (acl != NULL)
   {
      if ((csp->ip_addr_long & acl->src->mask) == acl->src->addr)
      {
         if (dst == NULL)
         {
            /* Just want to check if they have any access */
            if (acl->action == ACL_PERMIT)
            {
               return(0);
            }
         }
         else if ( ((dst->addr & acl->dst->mask) == acl->dst->addr)
           && ((dst->port == acl->dst->port) || (acl->dst->port == 0)))
         {
            if (acl->action == ACL_PERMIT)
            {
               return(0);
            }
            else
            {
               return(1);
            }
         }
      }
      acl = acl->next;
   }

   return(1);

}


/*********************************************************************
 *
 * Function    :  acl_addr
 *
 * Description :  Called from `load_aclfile' to parse an ACL address.
 *
 * Parameters  :
 *          1  :  aspec = String specifying ACL address.
 *          2  :  aca = struct access_control_addr to fill in.
 *
 * Returns     :  0 => Ok, everything else is an error.
 *
 *********************************************************************/
int acl_addr(char *aspec, struct access_control_addr *aca)
{
   int i, masklength, port;
   char *p;

   masklength = 32;
   port       =  0;

   if ((p = strchr(aspec, '/')))
   {
      *p++ = '\0';

      if (ijb_isdigit(*p) == 0)
      {
         return(-1);
      }
      masklength = atoi(p);
   }

   if ((masklength < 0) || (masklength > 32))
   {
      return(-1);
   }

   if ((p = strchr(aspec, ':')))
   {
      *p++ = '\0';

      if (ijb_isdigit(*p) == 0)
      {
         return(-1);
      }
      port = atoi(p);
   }

   aca->port = port;

   aca->addr = ntohl(resolve_hostname_to_ip(aspec));

   if (aca->addr == -1)
   {
      log_error(LOG_LEVEL_ERROR, "can't resolve address for %s", aspec);
      return(-1);
   }

   /* build the netmask */
   aca->mask = 0;
   for (i=1; i <= masklength ; i++)
   {
      aca->mask |= (1 << (32 - i));
   }

   /* now mask off the host portion of the ip address
    * (i.e. save on the network portion of the address).
    */
   aca->addr = aca->addr & aca->mask;

   return(0);

}
#endif /* def ACL_FILES */


/*********************************************************************
 *
 * Function    :  block_url
 *
 * Description :  Called from `chat'.  Check to see if we need to block this.
 *
 * Parameters  :
 *          1  :  http = http_request request to "check" for blocked
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  NULL => unblocked, else string to HTML block description.
 *
 *********************************************************************/
char *block_url(struct http_request *http, struct client_state *csp)
{
   char *p;
   int n;
   int factor = 2;

   if ((csp->action->flags & ACTION_BLOCK) == 0)
   {
      return(NULL);
   }
   else
   {
#ifdef FORCE_LOAD
      factor++;
#endif /* def FORCE_LOAD */

      n  = strlen(CBLOCK);
      n += factor * strlen(http->hostport);
      n += factor * strlen(http->path);

      p = (char *)malloc(n);

#ifdef FORCE_LOAD
      sprintf(p, CBLOCK, http->hostport, http->path, http->hostport, http->path,
              http->hostport, http->path);
#else
      sprintf(p, CBLOCK, http->hostport, http->path, http->hostport, http->path);
#endif /* def FORCE_LOAD */

      return(p);
   }
}


#ifdef IMAGE_BLOCKING
/*********************************************************************
 *
 * Function    :  block_imageurl
 *
 * Description :  Given a URL which is blocked, decide whether to 
 *                send the "blocked" image or HTML.
 *
 * Parameters  :
 *          1  :  http = URL to check.
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  True (nonzero) if URL is in image list, false (0)
 *                otherwise
 *
 *********************************************************************/
int block_imageurl(struct http_request *http, struct client_state *csp)
{
#ifdef DETECT_MSIE_IMAGES
   if ((csp->accept_types 
       & (ACCEPT_TYPE_IS_MSIE|ACCEPT_TYPE_MSIE_IMAGE|ACCEPT_TYPE_MSIE_HTML))
       == (ACCEPT_TYPE_IS_MSIE|ACCEPT_TYPE_MSIE_IMAGE))
   {
      return 1;
   }
   else if ((csp->accept_types 
       & (ACCEPT_TYPE_IS_MSIE|ACCEPT_TYPE_MSIE_IMAGE|ACCEPT_TYPE_MSIE_HTML))
       == (ACCEPT_TYPE_IS_MSIE|ACCEPT_TYPE_MSIE_HTML))
   {
      return 0;
   }
#endif

   return ((csp->action->flags & ACTION_IMAGE) != 0);
}
#endif /* def IMAGE_BLOCKING */


#ifdef TRUST_FILES
/*********************************************************************
 *
 * Function    :  trust_url
 *
 * Description :  Should we "trust" this URL?  See "trustfile" line in config.
 *
 * Parameters  :
 *          1  :  http = http_request request for requested URL
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  NULL => trusted, else string to HTML "untrusted" description.
 *
 *********************************************************************/
char *trust_url(struct http_request *http, struct client_state *csp)
{
   struct file_list *fl;
   struct block_spec *b;
   struct url_spec url[1], **tl, *t;
   char *p, *h;
   char *hostport, *path, *refer;
   struct http_request rhttp[1];
   int n;

   if (((fl = csp->tlist) == NULL) || ((b  = fl->f) == NULL))
   {
      return(NULL);
   }

   *url = dsplit(http->host);

   /* if splitting the domain fails, punt */
   if (url->dbuf == NULL) return(NULL);

   memset(rhttp, '\0', sizeof(*rhttp));

   for (b = b->next; b ; b = b->next)
   {
      if ((b->url->port == 0) || (b->url->port == http->port))
      {
         if ((b->url->domain[0] == '\0') || (domaincmp(b->url, url) == 0))
         {
            if ((b->url->path == NULL) ||
#ifdef REGEX
               (regexec(b->url->preg, http->path, 0, NULL, 0) == 0)
#else
               (strncmp(b->url->path, http->path, b->url->pathlen) == 0)
#endif
            )
            {
               freez(url->dbuf);
               freez(url->dvec);

               if (b->reject == 0) return(NULL);

               hostport = url_encode(http->hostport);
               path     = url_encode(http->path);

               if (csp->referrer)
               {
                  refer = url_encode(csp->referrer);
               }
               else
               {
                  refer = url_encode("undefined");
               }

               n  = strlen(CTRUST);
               n += strlen(hostport);
               n += strlen(path);
               n += strlen(refer);

               p = (char *)malloc(n);

               sprintf(p, CTRUST, hostport, path, refer);

               freez(hostport);
               freez(path);
               freez(refer);

               return(p);
            }
         }
      }
   }

   freez(url->dbuf);
   freez(url->dvec);

   if ((csp->referrer == NULL)|| (strlen(csp->referrer) <= 9))
   {
      /* no referrer was supplied */
      goto trust_url_not_trusted;
   }

   /* forge a URL from the referrer so we can use
    * convert_url() to parse it into its components.
    */

   p = NULL;
   p = strsav(p, "GET ");
   p = strsav(p, csp->referrer + 9);   /* skip over "Referer: " */
   p = strsav(p, " HTTP/1.0");

   parse_http_request(p, rhttp, csp);

   if (rhttp->cmd == NULL)
   {
      freez(p);
      goto trust_url_not_trusted;
   }

   freez(p);

   *url = dsplit(rhttp->host);

   /* if splitting the domain fails, punt */
   if (url->dbuf == NULL) goto trust_url_not_trusted;

   for (tl = csp->config->trust_list; (t = *tl) ; tl++)
   {
      if ((t->port == 0) || (t->port == rhttp->port))
      {
         if ((t->domain[0] == '\0') || domaincmp(t, url) == 0)
         {
            if ((t->path == NULL) ||
#ifdef REGEX
               (regexec(t->preg, rhttp->path, 0, NULL, 0) == 0)
#else
               (strncmp(t->path, rhttp->path, t->pathlen) == 0)
#endif
            )
            {
               /* if the URL's referrer is from a trusted referrer, then
                * add the target spec to the trustfile as an unblocked
                * domain and return NULL (which means it's OK).
                */

               FILE *fp;

               freez(url->dbuf);
               freez(url->dvec);

               if ((fp = fopen(csp->config->trustfile, "a")))
               {
                  h = NULL;

                  h = strsav(h, "~");
                  h = strsav(h, http->hostport);

                  p = http->path;
                  if ((*p++ == '/')
                  && (*p++ == '~'))
                  {
                  /* since this path points into a user's home space
                   * be sure to include this spec in the trustfile.
                   */
                     if ((p = strchr(p, '/')))
                     {
                        *p = '\0';
                        h = strsav(h, http->path);
                        h = strsav(h, "/");
                     }
                  }

                  free_http_request(rhttp);

                  fprintf(fp, "%s\n", h);
                  freez(h);
                  fclose(fp);
               }
               return(NULL);
            }
         }
      }
   }

trust_url_not_trusted:
   free_http_request(rhttp);

   hostport = url_encode(http->hostport);
   path     = url_encode(http->path);

   if (csp->referrer)
   {
      refer = url_encode(csp->referrer);
   }
   else
   {
      refer = url_encode("undefined");
   }

   n  = strlen(CTRUST);
   n += strlen(hostport);
   n += strlen(path);
   n += strlen(refer);

   p = (char *)malloc(n);
   sprintf(p, CTRUST, hostport, path, refer);

   freez(hostport);
   freez(path);
   freez(refer);

   return(p);

}
#endif /* def TRUST_FILES */


#ifdef PCRS
/*********************************************************************
 *
 * Function    :  re_process_buffer
 *
 * Description :  Apply all jobs from the joblist (aka. Perl regexp's) to
 *                the text buffer that's been accumulated in csp->iob->buf
 *                and set csp->content_length to the modified size.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  a pointer to the (newly allocated) modified buffer.
 *                
 *
 *********************************************************************/
char *re_process_buffer(struct client_state *csp)
{
   int hits=0;
   int size = csp->iob->eod - csp->iob->cur;
   char *old=csp->iob->cur, *new = NULL;
   pcrs_job *job, *joblist;

   struct file_list *fl;
   struct re_filterfile_spec *b;

   /* Sanity first ;-) */
   if (size <= 0)
   {
      return(strdup(""));
   }

   if ( ( NULL == (fl = csp->rlist) ) || ( NULL == (b = fl->f) ) )
   {
      log_error(LOG_LEVEL_ERROR, "Unable to get current state of regexp filtering.");
      return(strdup(""));
   }

   joblist = b->joblist;


   log_error(LOG_LEVEL_RE_FILTER, "re_filtering %s%s (size %d) ...",
              csp->http->hostport, csp->http->path, size);

   /* Apply all jobs from the joblist */
   for (job = joblist; NULL != job; job = job->next)
   {
      hits += pcrs_exec_substitution(job, old, size, &new, &size);
      if (old != csp->iob->cur) free(old);
      old=new;
   }

   log_error(LOG_LEVEL_RE_FILTER, " produced %d hits (new size %d).", hits, size);

   csp->content_length = size;

   /* fwiw, reset the iob */
   IOB_RESET(csp);
   return(new);

}
#endif /* def PCRS */


/*********************************************************************
 *
 * Function    :  url_actions
 *
 * Description :  Gets the actions for this URL.
 *
 * Parameters  :
 *          1  :  http = http_request request for blocked URLs
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void url_actions(struct http_request *http, 
                 struct client_state *csp)
{
   struct file_list *fl;
   struct url_actions *b;

   init_current_action(csp->action);

   if (((fl = csp->actions_list) == NULL) || ((b = fl->f) == NULL))
   {
      return;
   }

   apply_url_actions(csp->action, http, b);
}


/*********************************************************************
 *
 * Function    :  apply_url_actions
 *
 * Description :  Applies a list of URL actions.
 *
 * Parameters  :
 *          1  :  action = Destination.
 *          2  :  http = Current URL
 *          3  :  b = list of URL actions to apply
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void apply_url_actions(struct current_action_spec *action, 
                       struct http_request *http, 
                       struct url_actions *b)
{
   struct url_spec url[1];

   if (b == NULL)
   {
      /* Should never happen */
      return;
   }

   *url = dsplit(http->host);

   /* if splitting the domain fails, punt */
   if (url->dbuf == NULL)
   {
      return;
   }

   for (b = b->next; NULL != b; b = b->next)
   {
      if ((b->url->port == 0) || (b->url->port == http->port))
      {
         if ((b->url->domain[0] == '\0') || (domaincmp(b->url, url) == 0))
         {
            if ((b->url->path == NULL) ||
#ifdef REGEX
               (regexec(b->url->preg, http->path, 0, NULL, 0) == 0)
#else
               (strncmp(b->url->path, http->path, b->url->pathlen) == 0)
#endif
            )
            {
               merge_current_action(action, b->action);
            }
         }
      }
   }

   freez(url->dbuf);
   freez(url->dvec);
}


/*********************************************************************
 *
 * Function    :  forward_url
 *
 * Description :  Should we forward this to another proxy?
 *
 * Parameters  :
 *          1  :  http = http_request request for current URL
 *          2  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  Pointer to forwarding information.
 *
 *********************************************************************/
const struct forward_spec * forward_url(struct http_request *http,
                                        struct client_state *csp)
{
   static const struct forward_spec fwd_default[1] = { 0 }; /* All zeroes */
   struct forward_spec *fwd = csp->config->forward;
   struct url_spec url[1];

   if (fwd == NULL)
   {
      return(fwd_default);
   }

   *url = dsplit(http->host);

   /* if splitting the domain fails, punt */
   if (url->dbuf == NULL)
   {
      return(fwd_default);
   }

   while (fwd != NULL)
   {
      if ((fwd->url->port == 0) || (fwd->url->port == http->port))
      {
         if ((fwd->url->domain[0] == '\0') || (domaincmp(fwd->url, url) == 0))
         {
            if ((fwd->url->path == NULL) ||
#ifdef REGEX
               (regexec(fwd->url->preg, http->path, 0, NULL, 0) == 0)
#else
               (strncmp(fwd->url->path, http->path, fwd->url->pathlen) == 0)
#endif
            )
            {
               freez(url->dbuf);
               freez(url->dvec);
               return(fwd);
            }
         }
      }
      fwd = fwd->next;
   }

   freez(url->dbuf);
   freez(url->dvec);
   return(fwd_default);

}


/*********************************************************************
 *
 * Function    :  dsplit
 *
 * Description :  Takes a domain and returns a pointer to a url_spec
 *                structure populated with dbuf, dcnt and dvec.  The
 *                other fields in the structure that is returned are zero.
 *
 * Parameters  :
 *          1  :  domain = a URL address
 *
 * Returns     :  url_spec structure populated with dbuf, dcnt and dvec.
 *
 *********************************************************************/
struct url_spec dsplit(char *domain)
{
   struct url_spec ret[1];
   char *v[BUFFER_SIZE];
   int size;
   char *p;

   memset(ret, '\0', sizeof(*ret));

   if (domain[strlen(domain) - 1] == '.')
   {
	  ret->unanchored |= ANCHOR_RIGHT;
	}
	if (domain[0] == '.')
   {
	  ret->unanchored |= ANCHOR_LEFT;
	}

   ret->dbuf = strdup(domain);

   /* map to lower case */
   for (p = ret->dbuf; *p ; p++) *p = tolower(*p);

   /* split the domain name into components */
   ret->dcnt = ssplit(ret->dbuf, ".", v, SZ(v), 1, 1);

   if (ret->dcnt <= 0)
   {
      memset(ret, '\0', sizeof(ret));
      return(*ret);
   }

   /* save a copy of the pointers in dvec */
   size = ret->dcnt * sizeof(*ret->dvec);

   if ((ret->dvec = (char **)malloc(size)))
   {
      memcpy(ret->dvec, v, size);
   }


   return(*ret);

}


/*********************************************************************
 *
 * Function    :  simple_domaincmp
 *
 * Description :  Domain-wise Compare fqdn's.  The comparison is 
 *                both left- and right-anchored.  The individual
 *                domain names are compared with trivialmatch().
 *                This is only used by domaincmp.
 *
 * Parameters  :
 *          1  :  pv = array of patterns to compare
 *          2  :  fv = array of domain components to compare
 *          3  :  len = length of the arrays (both arrays are the
 *                      same length - if they weren't, it couldn't
 *                      possibly be a match).
 *
 * Returns     :  0 => domains are equivalent, else no match.
 *
 *********************************************************************/
static int simple_domaincmp(char **pv, char **fv, int len)
{
   int n;

   for (n = 0; n < len; n++)
   {
      if (simplematch(pv[n], fv[n]))
      {
         return 1;
      }
   }

   return 0;
}


/*********************************************************************
 *
 * Function    :  domaincmp
 *
 * Description :  Domain-wise Compare fqdn's. Governed by the bimap in
 *                pattern->unachored, the comparison is un-, left-,
 *                right-anchored, or both.
 *                The individual domain names are compared with
 *                trivialmatch().
 *
 * Parameters  :
 *          1  :  pattern = a domain that may contain a '*' as a wildcard.
 *          2  :  fqdn = domain name against which the patterns are compared.
 *
 * Returns     :  0 => domains are equivalent, else no match.
 *
 *********************************************************************/
int domaincmp(struct url_spec *pattern, struct url_spec *fqdn)
{
   char **pv, **fv;  /* vectors  */
   int    plen, flen;
   int unanchored = pattern->unanchored & (ANCHOR_RIGHT | ANCHOR_LEFT);

   plen = pattern->dcnt;
   flen = fqdn->dcnt;

   if (flen < plen)
   {
      /* fqdn is too short to match this pattern */
      return 1;
   }

   pv   = pattern->dvec;
   fv   = fqdn->dvec;

   if (unanchored == ANCHOR_LEFT)
   {
      /*
       * Right anchored.
       *
       * Convert this into a fully anchored pattern with
       * the fqdn and pattern the same length
       */
      fv += (flen - plen); /* flen - plen >= 0 due to check above */
      return simple_domaincmp(pv, fv, plen);
   }
   else if (unanchored == 0)
   {
      /* Fully anchored, check length */
      if (flen != plen)
      {
         return 1;
      }
      return simple_domaincmp(pv, fv, plen);
   }
   else if (unanchored == ANCHOR_RIGHT)
   {
      /* Left anchored, ignore all extra in fqdn */
      return simple_domaincmp(pv, fv, plen);
   }
   else
   {
      /* Unanchored */
      int n;
      int maxn = flen - plen;
      for (n = 0; n <= maxn; n++)
      {
         if (!simple_domaincmp(pv, fv, plen))
         {
            return 0;
         }
         /*
          * Doesn't match from start of fqdn
          * Try skipping first part of fqdn
          */
         fv++;
      }
      return 1;
   }
}



/*
  Local Variables:
  tab-width: 3
  end:
*/
