const char cgi_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Declares functions to intercept request, generate
 *                html or gif answers, and to compose HTTP resonses.
 *                
 *                Functions declared include:
 * 
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
 *    Revision 1.19  2001/09/13 23:31:25  jongfoster
 *    Moving image data to cgi.c rather than cgi.h.
 *
 *    Revision 1.18  2001/08/05 16:06:20  jongfoster
 *    Modifiying "struct map" so that there are now separate header and
 *    "map_entry" structures.  This means that functions which modify a
 *    map no longer need to return a pointer to the modified map.
 *    Also, it no longer reverses the order of the entries (which may be
 *    important with some advanced template substitutions).
 *
 *    Revision 1.17  2001/08/05 15:57:38  oes
 *    Adapted finish_http_response to new list_to_text
 *
 *    Revision 1.16  2001/08/01 21:33:18  jongfoster
 *    Changes to fill_template() that reduce memory usage without having
 *    an impact on performance.  I also renamed some variables so as not
 *    to clash with the C++ keywords "new" and "template".
 *
 *    Revision 1.15  2001/08/01 21:19:22  jongfoster
 *    Moving file version information to a separate CGI page.
 *
 *    Revision 1.14  2001/08/01 00:19:03  jongfoster
 *    New function: map_conditional() for an if-then-else syntax.
 *    Changing to use new version of show_defines()
 *
 *    Revision 1.13  2001/07/30 22:08:36  jongfoster
 *    Tidying up #defines:
 *    - All feature #defines are now of the form FEATURE_xxx
 *    - Permanently turned off WIN_GUI_EDIT
 *    - Permanently turned on WEBDAV and SPLIT_PROXY_ARGS
 *
 *    Revision 1.12  2001/07/29 18:47:05  jongfoster
 *    Adding missing #include "loadcfg.h"
 *
 *    Revision 1.11  2001/07/18 17:24:37  oes
 *    Changed to conform to new pcrs interface
 *
 *    Revision 1.10  2001/07/13 13:53:13  oes
 *    Removed all #ifdef PCRS and related code
 *
 *    Revision 1.9  2001/06/29 21:45:41  oes
 *    Indentation, CRLF->LF, Tab-> Space
 *
 *    Revision 1.8  2001/06/29 13:21:46  oes
 *    - Cosmetics: renamed and reordered functions, variables,
 *      texts, improved comments  etc
 *
 *    - Removed ij_untrusted_url() The relevant
 *      info is now part of the "untrusted" page,
 *      which is generated by filters.c:trust_url()
 *
 *    - Generators of content now call finish_http_response()
 *      themselves, making jcc.c:chat() a little less
 *      cluttered
 *
 *    - Removed obsolete "Pragma: no-cache" from our headers
 *
 *    - http_responses now know their head length
 *
 *    - fill_template now uses the new interface to pcrs, so that
 *       - long jobs (like whole files) no longer have to be assembled
 *         in a fixed size buffer
 *       - the new T (trivial) option is used, and the replacement may
 *         contain Perl syntax backrefs without confusing pcrs
 *
 *    - Introduced default_exports() which generates a set of exports
 *      common to all CGIs and other content generators
 *
 *    - Introduced convenience function map_block_killer()
 *
 *    - Introduced convenience function make_menu()
 *
 *    - Introduced CGI-like function error_response() which generates
 *      the "No such domain" and "Connect failed" messages using the
 *      CGI platform
 *
 *    - cgi_show_url_info:
 *      - adapted to new CGI features
 *      - form and answers now generated from same template
 *      - http:// prefix in URL now OK
 *
 *    - cgi_show_status:
 *      - adapted to new CGI features
 *      - no longer uses csp->init_proxy_args
 *
 *    - cgi_default:
 *      - moved menu generation to make_menu()
 *
 *    - add_stats now writes single export map entries instead
 *      of a fixed string
 *
 *    - Moved redirect_url() to filters.c
 *
 *    - Fixed mem leak in free_http_response(), map_block_killer(),
 *
 *    - Removed logentry from cancelled commit
 *
 *    Revision 1.7  2001/06/09 10:51:58  jongfoster
 *    Changing "show URL info" handler to new style.
 *    Changing BUFSIZ ==> BUFFER_SIZE
 *
 *    Revision 1.6  2001/06/07 23:05:19  jongfoster
 *    Removing code related to old forward and ACL files.
 *
 *    Revision 1.5  2001/06/05 19:59:16  jongfoster
 *    Fixing multiline character string (a GCC-only "feature"), and snprintf (it's _snprintf under VC++).
 *
 *    Revision 1.4  2001/06/04 10:41:52  swa
 *    show version string of cgi.h and cgi.c
 *
 *    Revision 1.3  2001/06/03 19:12:16  oes
 *    introduced new cgi handling
 *
 *    No revisions before 1.3
 *
 **********************************************************************/


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif /* def _WIN32 */

#include "project.h"
#include "cgi.h"
#include "list.h"
#include "encode.h"
#include "ssplit.h"
#include "jcc.h"
#include "filters.h"
#include "actions.h"
#include "errlog.h"
#include "miscutil.h"
#include "showargs.h"
#include "loadcfg.h"

const char cgi_h_rcs[] = CGI_H_VERSION;

const struct cgi_dispatcher cgi_dispatcher[] = {
   { "show-status", 
         11, cgi_show_status,  
         "Show information about the current configuration" }, 
   { "show-url-info",
         13, cgi_show_url_info, 
         "Show which actions apply to a URL and why"  },
   { "show-version", 
         12, cgi_show_version,  
         "Show the source code version numbers" }, 
   { "send-banner",
         11, cgi_send_banner, 
         "HIDE Send the transparent or \"Junkbuster\" gif" },
   { "",
         0, cgi_default,
         "Junkbuster main page" },
   { NULL, 0, NULL, NULL }
};


/*
 * Some images
 *
 * Hint: You can encode your own GIFs like that:
 * perl -e 'while (read STDIN, $c, 1) { printf("\\%.3o,", unpack("C", $c)); }'
 */

const char image_junkbuster_gif_data[] =
   "GIF89aD\000\013\000\360\000\000\000\000\000\377\377\377!"
   "\371\004\001\000\000\001\000,\000\000\000\000D\000\013\000"
   "\000\002a\214\217\251\313\355\277\000\200G&K\025\316hC\037"
   "\200\234\230Y\2309\235S\230\266\206\372J\253<\3131\253\271"
   "\270\215\342\254\013\203\371\202\264\334P\207\332\020o\266"
   "N\215I\332=\211\312\3513\266:\026AK)\364\370\365aobr\305"
   "\372\003S\275\274k2\354\254z\347?\335\274x\306^9\374\276"
   "\037Q\000\000;";

const int image_junkbuster_gif_length = sizeof(image_junkbuster_gif_data) - 1;


const char image_blank_gif_data[] =
   "GIF89a\001\000\001\000\200\000\000\377\377\377\000\000"
   "\000!\371\004\001\000\000\000\000,\000\000\000\000\001"
   "\000\001\000\000\002\002D\001\000;";

const int image_blank_gif_length = sizeof(image_blank_gif_data) - 1;


/*********************************************************************
 * 
 * Function    :  dispatch_cgi
 *
 * Description :  Checks if a request URL has either the magical hostname
 *                i.j.b or matches HOME_PAGE_URL/config/. If so, it parses
 *                the (rest of the) path as a cgi name plus query string,
 *                prepares a map that maps CGI parameter names to their values,
 *                initializes the http_response struct, and calls the 
 *                relevant CGI handler function.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  http_response if match, NULL if nonmatch or handler fail
 *
 *********************************************************************/
struct http_response *dispatch_cgi(struct client_state *csp)
{
   char *argstring = NULL;
   const struct cgi_dispatcher *d;
   struct map *param_list;
   struct http_response *rsp;

   /*
    * Should we intercept ?
    */

   /* Either the host matches CGI_PREFIX_HOST ..*/
   if (0 == strcmpic(csp->http->host, CGI_PREFIX_HOST))
   {
      /* ..then the path will all be for us */
      argstring = csp->http->path;
   }
   /* Or it's the host part HOME_PAGE_URL, and the path /config ? */
   else if (   (0 == strcmpic(csp->http->host, HOME_PAGE_URL + 7 ))
            && (0 == strncmpic(csp->http->path,"/config", 7))
            && ((csp->http->path[7] == '/') || (csp->http->path[7] == '\0')))
   {
      /* then it's everything following "/config" */
      argstring = csp->http->path + 7;
   }
   else
   {
      return NULL;
   }

   /* 
    * This is a CGI call.
    */

   /* Get mem for response or fail*/
   if (NULL == ( rsp = zalloc(sizeof(*rsp))))
   {
      return NULL;
   }

   /* Remove leading slash */
   if (*argstring == '/')
   {
      argstring++;
   }

   log_error(LOG_LEVEL_GPC, "%s%s cgi call", csp->http->hostport, csp->http->path);
   log_error(LOG_LEVEL_CLF, "%s - - [%T] \"%s\" 200 3", 
                            csp->ip_addr_str, csp->http->cmd); 

   /* Find and start the right CGI function*/
   for (d = cgi_dispatcher; d->handler; d++)
   {
      if (strncmp(argstring, d->name, d->name_length) == 0)
      {
         param_list = parse_cgi_parameters(argstring + d->name_length);
         if ((d->handler)(csp, rsp, param_list))
         {
            freez(rsp);
         }

         free_map(param_list);
         return(finish_http_response(rsp));
      }
   }

   /* Can't get here, since cgi_default will match all requests */
   freez(rsp);
   return(NULL);

}


/*********************************************************************
 *
 * Function    :  parse_cgi_parameters
 *
 * Description :  Parse a URL-encoded argument string into name/value
 *                pairs and store them in a struct map list.
 *
 * Parameters  :
 *          1  :  string = string to be parsed 
 *
 * Returns     :  pointer to param list
 *
 *********************************************************************/
struct map *parse_cgi_parameters(char *argstring)
{
   char *tmp, *p;
   char *vector[BUFFER_SIZE];
   int pairs, i;
   struct map *cgi_params = new_map();

   if(*argstring == '?')
   {
      argstring++;
   }
   tmp = strdup(argstring);

   pairs = ssplit(tmp, "&", vector, SZ(vector), 1, 1);

   for (i = 0; i < pairs; i++)
   {
      if ((NULL != (p = strchr(vector[i], '='))) && (*(p+1) != '\0'))
      {
         *p = '\0';
         map(cgi_params, url_decode(vector[i]), 0, url_decode(++p), 0);
      }
   }

   free(tmp);
   return(cgi_params);

}


/*********************************************************************
 *
 * Function    :  cgi_default
 *
 * Description :  CGI function that is called if no action was given.
 *                Lists menu of available unhidden CGIs.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * Returns     :  0
 *
 *********************************************************************/
int cgi_default(struct client_state *csp, struct http_response *rsp,
                struct map *parameters)
{
   char *p;
   char *tmp = NULL;
   struct map * exports = default_exports(csp, "");

   /* If there were other parameters, export a dump as "cgi-parameters" */
   if(parameters)
   {
      p = dump_map(parameters);
      tmp = strsav(tmp, "<p>What made you think this cgi takes parameters?\n"
                        "Anyway, here they are, in case you're interested:</p>\n");
      tmp = strsav(tmp, p);
      map(exports, "cgi-parameters", 1, tmp, 0);
      free(p);
   }
   else
   {
      map(exports, "cgi-parameters", 1, "", 1);
   }

   rsp->body = fill_template(csp, "default", exports);
   free_map(exports);
   return(0);

}


/*********************************************************************
 *
 * Function    :  cgi_send_banner
 *
 * Description :  CGI function that returns a banner. 
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *           type : Selects the type of banner between "trans" and "jb".
 *                  Defaults to "jb" if absent or != "trans".
 *
 * Returns     :  0
 *
 *********************************************************************/
int cgi_send_banner(struct client_state *csp, struct http_response *rsp,
                    struct map *parameters)
{
   if(strcmp(lookup(parameters, "type"), "trans"))
   {
      rsp->body = bindup(image_junkbuster_gif_data, image_junkbuster_gif_length);
      rsp->content_length = image_junkbuster_gif_length;
   }
   else
   {
      rsp->body = bindup(image_blank_gif_data, image_blank_gif_length);
      rsp->content_length = image_blank_gif_length;
   }   

   enlist(rsp->headers, "Content-Type: image/gif");

   return(0);

}


/*********************************************************************
 *
 * Function    :  cgi_show_version
 *
 * Description :  CGI function that returns a a web page describing the
 *                file versions of IJB.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *           type : Selects the type of banner between "trans" and "jb".
 *                  Defaults to "jb" if absent or != "trans".
 *
 * Returns     :  0
 *
 *********************************************************************/
int cgi_show_version(struct client_state *csp, struct http_response *rsp,
                     struct map *parameters)
{
   struct map * exports = default_exports(csp, "show-version");

   map(exports, "sourceversions", 1, show_rcs(), 0);  

   rsp->body = fill_template(csp, "show-version", exports);
   free_map(exports);
   return(0);

}

 
/*********************************************************************
 *
 * Function    :  cgi_show_status
 *
 * Description :  CGI function that returns a a web page describing the
 *                current status of IJB.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *           type : Selects the type of banner between "trans" and "jb".
 *                  Defaults to "jb" if absent or != "trans".
 *
 * Returns     :  0
 *
 *********************************************************************/
int cgi_show_status(struct client_state *csp, struct http_response *rsp,
                    struct map *parameters)
{
   char *s = NULL;
   int i;

   FILE * fp;
   char buf[BUFFER_SIZE];
   char * p;
   const char * filename = NULL;
   char * file_description = NULL;

   struct map * exports = default_exports(csp, "show-status");

   switch (*(lookup(parameters, "file")))
   {
   case 'p':
      if (csp->actions_list)
      {
         filename = csp->actions_list->filename;
         file_description = "Actions List";
      }
      break;

   case 'r':
      if (csp->rlist)
      {
         filename = csp->rlist->filename;
         file_description = "Regex Filter List";
      }
      break;

#ifdef FEATURE_TRUST
   case 't':
      if (csp->tlist)
      {
         filename = csp->tlist->filename;
         file_description = "Trust List";
      }
      break;
#endif /* def FEATURE_TRUST */
   }

   if (NULL != filename)
   {
      map(exports, "file-description", 1, file_description, 1);
      map(exports, "filepath", 1, html_encode(filename), 0);

      if ((fp = fopen(filename, "r")) == NULL)
      {
         map(exports, "content", 1, "<h1>ERROR OPENING FILE!</h1>", 1);
      }
      else
      {
         while (fgets(buf, sizeof(buf), fp))
         {
            p = html_encode(buf);
            if (p)
            {
               s = strsav(s, p);
               freez(p);
               s = strsav(s, "<br>");
            }
         }
         fclose(fp);
         map(exports, "contents", 1, s, 0);
      }
      rsp->body = fill_template(csp, "show-status-file", exports);
      free_map(exports);
      return(0);

   }

   map(exports, "redirect-url", 1, REDIRECT_URL, 1);
   
   s = NULL;
   for (i=0; i < Argc; i++)
   {
      s = strsav(s, Argv[i]);
      s = strsav(s, " ");
   }
   map(exports, "invocation", 1, s, 0);

   map(exports, "options", 1, csp->config->proxy_args, 1);
   show_defines(exports);

#ifdef FEATURE_STATISTICS
   add_stats(exports);
#else /* ndef FEATURE_STATISTICS */
   map_block_killer(exports, "statistics");
#endif /* ndef FEATURE_STATISTICS */

   if (csp->actions_list)
   {
      map(exports, "actions-filename", 1,  csp->actions_list->filename, 1);
   }
   else
   {
      map(exports, "actions-filename", 1, "None specified", 1);
   }

   if (csp->rlist)
   {
      map(exports, "re-filter-filename", 1,  csp->rlist->filename, 1);
   }
   else
   {
      map(exports, "re-filter-filename", 1, "None specified", 1);
   }

#ifdef FEATURE_TRUST
   if (csp->tlist)
   {
      map(exports, "trust-filename", 1,  csp->tlist->filename, 1);
   }
   else
   {
       map(exports, "trust-filename", 1, "None specified", 1);
   }
#else
   map_block_killer(exports, "trust-support");
#endif /* ndef FEATURE_TRUST */

   rsp->body = fill_template(csp, "show-status", exports);
   free_map(exports);
   return(0);

}

 
 /*********************************************************************
 *
 * Function    :  cgi_show_url_info
 *
 * Description :  CGI function that determines and shows which actions
 *                junkbuster will perform for a given url, and which
 *                matches starting from the defaults have lead to that.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *            url : The url whose actions are to be determined.
 *                  If url is unset, the url-given conditional will be
 *                  set, so that all but the form can be suppressed in
 *                  the template.
 *
 * Returns     :  0
 *
 *********************************************************************/
int cgi_show_url_info(struct client_state *csp, struct http_response *rsp,
                      struct map *parameters)
{
   char *url_param;
   char *host = NULL;
   struct map * exports = default_exports(csp, "show-url-info");

   if (NULL == (url_param = strdup(lookup(parameters, "url"))) || *url_param == '\0')
   {
      map_block_killer(exports, "url-given");
      map(exports, "url", 1, "", 1);
   }
   else
   {
      char *matches = NULL;
      char *path;
      char *s;
      int port = 80;
      int hits = 0;
      struct file_list *fl;
      struct url_actions *b;
      struct url_spec url[1];
      struct current_action_spec action[1];
      
      host = url_param;
      host += (strncmp(url_param, "http://", 7)) ? 0 : 7;

      map(exports, "url", 1, host, 1);
      map(exports, "url-html", 1, html_encode(host), 0);

      init_current_action(action);

      s = current_action_to_text(action);
      map(exports, "default", 1, s , 0);

      if (((fl = csp->actions_list) == NULL) || ((b = fl->f) == NULL))
      {
         map(exports, "matches", 1, "none" , 1);
         map(exports, "final", 1, lookup(exports, "default"), 1);

         freez(url_param);
         free_current_action(action);

         rsp->body = fill_template(csp, "show-url-info", exports);
         free_map(exports);

         return 0;
      }

      s = strchr(host, '/');
      if (s != NULL)
      {
         path = strdup(s);
         *s = '\0';
      }
      else
      {
         path = strdup("");
      }
      s = strchr(host, ':');
      if (s != NULL)
      {
         *s++ = '\0';
         port = atoi(s);
         s = NULL;
      }

      *url = dsplit(host);

      /* if splitting the domain fails, punt */
      if (url->dbuf == NULL)
      {
         map(exports, "matches", 1, "none" , 1);
         map(exports, "final", 1, lookup(exports, "default"), 1);

         freez(url_param);
         freez(path);
         free_current_action(action);

         rsp->body = fill_template(csp, "show-url-info", exports);
         free_map(exports);

         return 0;
      }

      for (b = b->next; NULL != b; b = b->next)
      {
         if ((b->url->port == 0) || (b->url->port == port))
         {
            if ((b->url->domain[0] == '\0') || (domaincmp(b->url, url) == 0))
            {
               if ((b->url->path == NULL) ||
#ifdef REGEX
                  (regexec(b->url->preg, path, 0, NULL, 0) == 0)
#else
                  (strncmp(b->url->path, path, b->url->pathlen) == 0)
#endif
               )
               {
                  s = actions_to_text(b->action);
                  matches = strsav(matches, "<b>{");
                  matches = strsav(matches, s);
                  matches = strsav(matches, " }</b><br>\n<code>");
                  matches = strsav(matches, b->url->spec);
                  matches = strsav(matches, "</code><br>\n<br>\n");
                  freez(s);

                  merge_current_action(action, b->action);
                  hits++;
               }
            }
         }
      }

      if (hits)
      {
         map(exports, "matches", 1, matches , 0);
      }
      else
      {
         map(exports, "matches", 1, "none", 1);
      }
      matches = NULL;

      freez(url->dbuf);
      freez(url->dvec);

      freez(url_param);
      freez(path);

      s = current_action_to_text(action);
      map(exports, "final", 1, s, 0);
      s = NULL;

      free_current_action(action);
   }

   rsp->body = fill_template(csp, "show-url-info", exports);
   free_map(exports);
   return 0;

}


/*********************************************************************
 *
 * Function    :  error_response
 *
 * Description :  returns an http_response that explains the reason
 *                why a request failed.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  templatename = Which template should be used for the answer
 *          3  :  errno = system error number
 *
 * Returns     :  NULL if no memory, else http_response
 *
 *********************************************************************/
struct http_response *error_response(struct client_state *csp, const char *templatename, int err)
{
   struct http_response *rsp;
   struct map * exports = default_exports(csp, NULL);

   if (NULL == ( rsp = (struct http_response *)zalloc(sizeof(*rsp))))
   {
      return NULL;
   }  

      map(exports, "host-html", 1, html_encode(csp->http->host), 0);
      map(exports, "hostport", 1, csp->http->hostport, 1);
      map(exports, "hostport-html", 1, html_encode(csp->http->hostport), 0);
      map(exports, "path", 1, csp->http->path, 1);
      map(exports, "path-html", 1, html_encode(csp->http->path), 0);
      map(exports, "error", 1, safe_strerror(err), 0);
      map(exports, "host-ip", 1, csp->http->host_ip_addr_str, 1);

      rsp->body = fill_template(csp, templatename, exports);
      free_map(exports);
      
      if (!strcmp(templatename, "no-such-domain"))
      {
         rsp->status = strdup("404 No such domain"); 
      }
      else if (!strcmp(templatename, "connect-failed"))
      {
         rsp->status = strdup("503 Connect failed");
      }

      return(finish_http_response(rsp));
}


/*********************************************************************
 *
 * Function    :  finish_http_response
 *
 * Description :  Fill in the missing headers in an http response,
 *                and flatten the headers to an http head.
 *
 * Parameters  :
 *          1  :  rsp = pointer to http_response to be processed
 *
 * Returns     :  http_response, or NULL on failiure
 *
 *********************************************************************/
struct http_response *finish_http_response(struct http_response *rsp)
{
   char buf[BUFFER_SIZE];

   /* 
    * Fill in the HTTP Status
    */
   sprintf(buf, "HTTP/1.0 %s", rsp->status ? rsp->status : "200 OK");
   enlist_first(rsp->headers, buf);

   /* 
    * Set the Content-Length
    */
   if (rsp->content_length == 0)
   {
      rsp->content_length = rsp->body ? strlen(rsp->body) : 0;
   }
   sprintf(buf, "Content-Length: %d", rsp->content_length);
   enlist(rsp->headers, buf);

   /* 
    * Fill in the default headers FIXME: Are these correct? sequence OK? check rfc!
    * FIXME: Should have:
    *   "JunkBuster" GIF: Last-Modified: any *fixed* date in the past (as now).
    *                     Expires: 5 minutes after the time when reply sent
    *   CGI, "blocked", & all other requests:
    *        Last-Modified: Time when reply sent
    *        Expires:       Time when reply sent
    *       "Cache-Control: no-cache"
    *        
    * See http://www.w3.org/Protocols/rfc2068/rfc2068
    */
   enlist_unique(rsp->headers, "Last-Modified: Thu Jul 31, 1997 07:42:22 pm GMT", 14);
   enlist_unique(rsp->headers, "Expires:       Thu Jul 31, 1997 07:42:22 pm GMT", 8);
   enlist_unique(rsp->headers, "Content-Type: text/html", 13);


   /* 
    * Write the head
    */
   if (NULL == (rsp->head = list_to_text(rsp->headers)))
   {
      free_http_response(rsp);
      return(NULL);
   }
   rsp->head_length = strlen(rsp->head);

   return(rsp);

}
  

/*********************************************************************
 *
 * Function    :  free_http_response
 *
 * Description :  Free the memory occupied by an http_response
 *                and its depandant structures.
 *
 * Parameters  :
 *          1  :  rsp = pointer to http_response to be freed
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void free_http_response(struct http_response *rsp)
{
   if(rsp)
   {
      freez(rsp->status);
      freez(rsp->head);
      freez(rsp->body);
      destroy_list(rsp->headers);
      freez(rsp);
   }

}


/*********************************************************************
 *
 * Function    :  fill_template
 *
 * Description :  CGI support function that loads a given HTML
 *                template from the confdir, and fills it in
 *                by replacing @name@ with value using pcrs,
 *                for each item in the output map.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           3 :  template = name of the HTML template to be used
 *           2 :  exports = map with fill in symbol -> name pairs
 *
 * Returns     :  char * with filled out form, or NULL if failiure
 *
 *********************************************************************/
char *fill_template(struct client_state *csp, const char *templatename, struct map *exports)
{
   struct map_entry *m;
   pcrs_job *job;
   char buf[BUFFER_SIZE];
   char *tmp_out_buffer;
   char *file_buffer = NULL;
   int size;
   int error;
   FILE *fp;


   /*
    * Open template file or fail
    */
   snprintf(buf, BUFFER_SIZE, "%s/templates/%s", csp->config->confdir, templatename);

   if(NULL == (fp = fopen(buf, "r")))
   {
      log_error(LOG_LEVEL_ERROR, "error loading template %s: %E", buf);
      return NULL;
   }
   

   /* 
    * Read the file, ignoring comments
    */
   while (fgets(buf, BUFFER_SIZE, fp))
   {
      /* skip lines starting with '#' */
      if(*buf == '#') continue;
   
      file_buffer = strsav(file_buffer, buf);
   }
   fclose(fp);


   /*
    * Execute the jobs
    */
   size = strlen(file_buffer) + 1;

   /* 
    * Assemble pcrs joblist from exports map
    */
   for (m = exports->first; m != NULL; m = m->next)
   {
      /* Enclose name in @@ */
      snprintf(buf, BUFFER_SIZE, "@%s@", m->name);

      /* Make and run job. */
      job = pcrs_compile(buf, m->value, "sigTU",  &error);
      if (job == NULL) 
      {
         log_error(LOG_LEVEL_ERROR, "Error compiling template fill job %s: %d", m->name, error);
      }
      else
      {
         pcrs_execute(job, file_buffer, size, &tmp_out_buffer, &size);
         if (file_buffer != tmp_out_buffer)
         {
            free(file_buffer);
            file_buffer = tmp_out_buffer;
         }
         pcrs_free_job(job);
      }
   }


   /*
    * Return
    */
   return(file_buffer);

}


/*********************************************************************
 *
 * Function    :  default_exports
 *
 * Description :  returns a struct map list that contains exports
 *                which are common to all CGI functions.
 *
 * Parameters  :
 *          1  :  exports = Structure to write output to.  This
 *                structure should be newly allocated and will be
 *                zeroed.
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  caller = name of CGI who calls us and which should
 *                         be excluded from the generated menu.
 * Returns     :  NULL if no memory, else map
 *
 *********************************************************************/
struct map * default_exports(const struct client_state *csp, const char *caller)
{
   char buf[20];
   struct map * exports = new_map();

   map(exports, "version", 1, VERSION, 1);
   map(exports, "my-ip-address", 1, csp->my_ip_addr_str ? csp->my_ip_addr_str : "unknown", 1);
   map(exports, "my-hostname", 1, csp->my_hostname ? csp->my_hostname : "unknown", 1);
   map(exports, "admin-address", 1, csp->config->admin_address ? csp->config->admin_address : "fill@me.in.please", 1);
   map(exports, "homepage", 1, HOME_PAGE_URL, 1);
   map(exports, "default-cgi", 1, HOME_PAGE_URL "/config", 1);
   map(exports, "menu", 1, make_menu(caller), 0);
   map(exports, "code-status", 1, CODE_STATUS, 1);

   snprintf(buf, 20, "%d", csp->config->hport);
   map(exports, "my-port", 1, buf, 1);

   if(!strcmp(CODE_STATUS, "stable"))
   {
      map_block_killer(exports, "unstable");
   }

   if(csp->config->proxy_info_url != NULL)
   {
      map(exports, "proxy-info-url", 1, csp->config->proxy_info_url, 1);
   }
   else
   {
      map_block_killer(exports, "have-proxy-info");
   }   

   return (exports);
}


/*********************************************************************
 *
 * Function    :  map_block_killer
 *
 * Description :  Convenience function.
 *                Adds a "killer" for the conditional HTML-template
 *                block <name>, i.e. a substitution of the regex
 *                "if-<name>-start.*if-<name>-end" to the given
 *                export list.
 *
 * Parameters  :  
 *          1  :  exports = map to extend
 *          2  :  name = name of conditional block
 *
 * Returns     :  extended map
 *
 *********************************************************************/
void map_block_killer(struct map *exports, const char *name)
{
   char buf[1000]; /* Will do, since the names are hardwired */

   snprintf(buf, 1000, "if-%s-start.*if-%s-end", name, name);
   map(exports, buf, 1, "", 1);
}


/*********************************************************************
 *
 * Function    :  map_conditional
 *
 * Description :  Convenience function.
 *                Adds an "if-then-else" for the conditional HTML-template
 *                block <name>, i.e. a substitution of the form:
 *                @if-<name>-then@
 *                   True text
 *                @else-not-<name>@
 *                   False text
 *                @endif-<name>@
 *
 *                The control structure and one of the alternatives
 *                will be hidden.
 *
 * Parameters  :  
 *          1  :  exports = map to extend
 *          2  :  name = name of conditional block
 *          3  :  choose_first = nonzero for first, zero for second.
 *
 * Returns     :  extended map
 *
 *********************************************************************/
void map_conditional(struct map *exports, const char *name, int choose_first)
{
   char buf[1000]; /* Will do, since the names are hardwired */

   snprintf(buf, 1000, (choose_first
      ? "else-not-%s@.*@endif-%s"
      : "if-%s-then@.*@else-not-%s"),
      name, name);
   map(exports, buf, 1, "", 1);

   snprintf(buf, 1000, (choose_first ? "if-%s-then" : "endif-%s"), name);
   map(exports, buf, 1, "", 1);
}


/*********************************************************************
 *
 * Function    :  make_menu
 *
 * Description :  Returns an HTML-formatted menu of the available 
 *                unhidden CGIs, excluding the one given in <self>.
 *
 * Parameters  :  self = name of CGI to leave out, can be NULL
 *
 * Returns     :  menu string
 *
 *********************************************************************/
char *make_menu(const char *self)
{
   const struct cgi_dispatcher *d;
   char buf[BUFFER_SIZE];
   char *result = NULL;

   if (self == NULL)
   {
      self = "NO-SUCH-CGI!";
   }

   /* List available unhidden CGI's and export as "other-cgis" */
   for (d = cgi_dispatcher; d->handler; d++)
   {
      if (strncmp(d->description, "HIDE", 4) && strcmp(d->name, self))
      {
         snprintf(buf, BUFFER_SIZE, "<li><a href=%s/config/%s>%s</a></li>\n",
   	       HOME_PAGE_URL, d->name, d->description);
         result = strsav(result, buf);
      }
   }
   return(result);

}


/*********************************************************************
 *
 * Function    :  dump_map
 *
 * Description :  HTML-dump a map for debugging
 *
 * Parameters  :
 *          1  :  the_map = map to dump
 *
 * Returns     :  string with HTML
 *
 *********************************************************************/
char *dump_map(const struct map *the_map)
{
   struct map_entry *cur_entry = the_map->first;
   char *ret = NULL;

   ret = strsav(ret, "<table>\n");

   while (cur_entry)
   {
      ret = strsav(ret, "<tr><td><b>");
      ret = strsav(ret, cur_entry->name);
      ret = strsav(ret, "</b></td><td>");
      ret = strsav(ret, cur_entry->value);
      ret = strsav(ret, "</td></tr>\n");
      cur_entry = cur_entry->next;
   }

   ret = strsav(ret, "</table>\n");
   return(ret);

}


#ifdef FEATURE_STATISTICS
/*********************************************************************
 *
 * Function    :  add_stats
 *
 * Description :  Add the blocking statistics to a given map.
 *
 * Parameters  :
 *          1  :  exports = map to write to.
 *
 * Returns     :  pointer to extended map
 *
 *********************************************************************/
struct map *add_stats(struct map *exports)
{
   float perc_rej;   /* Percentage of http requests rejected */
   char buf[1000];
   int local_urls_read     = urls_read;
   int local_urls_rejected = urls_rejected;

   /*
    * Need to alter the stats not to include the fetch of this
    * page.
    *
    * Can't do following thread safely! doh!
    *
    * urls_read--;
    * urls_rejected--; * This will be incremented subsequently *
    */

   if (local_urls_read == 0)
   {
      map_block_killer(exports, "have-stats");
   }
   else
   {
      map_block_killer(exports, "have-no-stats");

      perc_rej = (float)local_urls_rejected * 100.0F /
            (float)local_urls_read;

      sprintf(buf, "%d", local_urls_read);
      map(exports, "requests-received", 1, buf, 1);

      sprintf(buf, "%d", local_urls_rejected);
      map(exports, "requests-blocked", 1, buf, 1);

      sprintf(buf, "%6.2f", perc_rej);
      map(exports, "percent-blocked", 1, buf, 1);
   }

   return(exports);

}
#endif /* def FEATURE_STATISTICS */

/*
  Local Variables:
  tab-width: 3
  end:
*/
