const char cgisimple_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Simple CGIs to get information about JunkBuster's
 *                status.
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
 *    Revision 1.8  2001/11/13 00:14:07  jongfoster
 *    Fixing stupid bug now I've figured out what || means.
 *    (It always returns 0 or 1, not one of it's paramaters.)
 *
 *    Revision 1.7  2001/10/23 21:48:19  jongfoster
 *    Cleaning up error handling in CGI functions - they now send back
 *    a HTML error page and should never cause a FATAL error.  (Fixes one
 *    potential source of "denial of service" attacks).
 *
 *    CGI actions file editor that works and is actually useful.
 *
 *    Ability to toggle JunkBuster remotely using a CGI call.
 *
 *    You can turn off both the above features in the main configuration
 *    file, e.g. if you are running a multi-user proxy.
 *
 *    Revision 1.6  2001/10/14 22:00:32  jongfoster
 *    Adding support for a 404 error when an invalid CGI page is requested.
 *
 *    Revision 1.5  2001/10/07 15:30:41  oes
 *    Removed FEATURE_DENY_GZIP
 *
 *    Revision 1.4  2001/10/02 15:31:12  oes
 *    Introduced show-request cgi
 *
 *    Revision 1.3  2001/09/22 16:34:44  jongfoster
 *    Removing unneeded #includes
 *
 *    Revision 1.2  2001/09/19 18:01:11  oes
 *    Fixed comments; cosmetics
 *
 *    Revision 1.1  2001/09/16 17:08:54  jongfoster
 *    Moving simple CGI functions from cgi.c to new file cgisimple.c
 *
 *
 **********************************************************************/


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif /* def _WIN32 */

#include "project.h"
#include "cgi.h"
#include "cgisimple.h"
#include "list.h"
#include "encode.h"
#include "jcc.h"
#include "filters.h"
#include "actions.h"
#include "miscutil.h"
#include "loadcfg.h"
#include "parsers.h"

const char cgisimple_h_rcs[] = CGISIMPLE_H_VERSION;


static char *show_rcs(void);
static jb_err show_defines(struct map *exports);


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
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory
 *                (Problems other than out-of-memory should be
 *                handled by this routine - it should set the
 *                rsp appropriately and return "success")
 *
 *********************************************************************/
jb_err cgi_default(struct client_state *csp,
                   struct http_response *rsp,
                   const struct map *parameters)
{
   char *p;
   char *tmp;
   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, "")))
   {
      return JB_ERR_MEMORY;
   }

   /* If there were other parameters, export a dump as "cgi-parameters" */
   if (parameters->first)
   {
      if (NULL == (p = dump_map(parameters)))
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }
      tmp = strdup("<p>What made you think this cgi takes parameters?\n"
                   "Anyway, here they are, in case you're interested:</p>\n");
      string_append(&tmp, p);
      free(p);
      if (tmp == NULL)
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }
      if (map(exports, "cgi-parameters", 1, tmp, 0))
      {
         return JB_ERR_MEMORY;
      }
   }
   else
   {
      if (map(exports, "cgi-parameters", 1, "", 1))
      {
         return JB_ERR_MEMORY;
      }
   }

   return template_fill_for_cgi(csp, "default", exports, rsp);
}




/*********************************************************************
 *
 * Function    :  cgi_error_404
 *
 * Description :  CGI function that is called if an unknown action was
 *                given.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_error_404(struct client_state *csp,
                     struct http_response *rsp,
                     const struct map *parameters)
{
   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("404 JunkBuster configuration page not found");
   if (rsp->status == NULL)
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   return template_fill_for_cgi(csp, "cgi-error-404", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_show_request
 *
 * Description :  Show the client's request and what sed() would have
 *                made of it.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_show_request(struct client_state *csp,
                        struct http_response *rsp,
                        const struct map *parameters)
{
   char *p;
   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, "show-request")))
   {
      return JB_ERR_MEMORY;
   }
   
   /*
    * Repair the damage done to the IOB by get_header()
    */
   for (p = csp->iob->buf; p < csp->iob->eod; p++)
   {
      if (*p == '\0') *p = '\n';
   }

   /*
    * Export the original client's request and the one we would
    * be sending to the server if this wasn't a CGI call
    */

   if (map(exports, "client-request", 1, csp->iob->buf, 1))
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   if (map(exports, "processed-request", 1, sed(client_patterns, add_client_headers, csp), 0))
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }
   
   return template_fill_for_cgi(csp, "show-request", exports, rsp);
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
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_send_banner(struct client_state *csp,
                       struct http_response *rsp,
                       const struct map *parameters)
{
   if (strcmp(lookup(parameters, "type"), "trans"))
   {
      rsp->body = bindup(image_junkbuster_gif_data, image_junkbuster_gif_length);
      rsp->content_length = image_junkbuster_gif_length;
   }
   else
   {
      rsp->body = bindup(image_blank_gif_data, image_blank_gif_length);
      rsp->content_length = image_blank_gif_length;
   }   

   if (rsp->body == NULL)
   {
      return JB_ERR_MEMORY;
   }

   if (enlist(rsp->headers, "Content-Type: image/gif"))
   {
      return JB_ERR_MEMORY;
   }

   rsp->is_static = 1;

   return JB_ERR_OK;

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
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_show_version(struct client_state *csp,
                        struct http_response *rsp,
                        const struct map *parameters)
{
   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, "show-version")))
   {
      return JB_ERR_MEMORY;
   }

   if (map(exports, "sourceversions", 1, show_rcs(), 0))
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   return template_fill_for_cgi(csp, "show-version", exports, rsp);
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
 *        file :  Which file to show.  Only first letter is checked,
 *                valid values are:
 *                - "p"ermissions (actions) file
 *                - "r"egex
 *                - "t"rust
 *                Default is to show menu and other information.
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_show_status(struct client_state *csp,
                       struct http_response *rsp,
                       const struct map *parameters)
{
   char *s = NULL;
   int i;

   FILE * fp;
   char buf[BUFFER_SIZE];
   char * p;
   const char * filename = NULL;
   char * file_description = NULL;
#ifdef FEATURE_STATISTICS
   float perc_rej;   /* Percentage of http requests rejected */
   int local_urls_read;
   int local_urls_rejected;
#endif /* ndef FEATURE_STATISTICS */
   jb_err err;

   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, "show-status")))
   {
      return JB_ERR_MEMORY;
   }

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
      if ( map(exports, "file-description", 1, file_description, 1)
        || map(exports, "filepath", 1, html_encode(filename), 0) )
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      if ((fp = fopen(filename, "r")) == NULL)
      {
         if (map(exports, "content", 1, "<h1>ERROR OPENING FILE!</h1>", 1))
         {
            free_map(exports);
            return JB_ERR_MEMORY;
         }
      }
      else
      {
         s = strdup("");
         while ((s != NULL) && fgets(buf, sizeof(buf), fp))
         {
            p = html_encode(buf);
            if (p)
            {
               string_append(&s, p);
               freez(p);
               string_append(&s, "<br>");
            }
         }
         fclose(fp);

         if (map(exports, "contents", 1, s, 0))
         {
            free_map(exports);
            return JB_ERR_MEMORY;
         }
      }

      return template_fill_for_cgi(csp, "show-status-file", exports, rsp);
   }

   if (map(exports, "redirect-url", 1, REDIRECT_URL, 1))
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }
   
   s = strdup("");
   for (i=0; i < Argc; i++)
   {
      string_append(&s, Argv[i]);
      string_append(&s, " ");
   }
   if (map(exports, "invocation", 1, s, 0))
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   err = map(exports, "options", 1, csp->config->proxy_args, 1);
   if (!err) err = show_defines(exports);

#ifdef FEATURE_STATISTICS
   local_urls_read     = urls_read;
   local_urls_rejected = urls_rejected;

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
      if (!err) err = map_block_killer(exports, "have-stats");
   }
   else
   {
      if (!err) err = map_block_killer(exports, "have-no-stats");

      perc_rej = (float)local_urls_rejected * 100.0F /
            (float)local_urls_read;

      sprintf(buf, "%d", local_urls_read);
      if (!err) err = map(exports, "requests-received", 1, buf, 1);

      sprintf(buf, "%d", local_urls_rejected);
      if (!err) err = map(exports, "requests-blocked", 1, buf, 1);

      sprintf(buf, "%6.2f", perc_rej);
      if (!err) err = map(exports, "percent-blocked", 1, buf, 1);
   }

#else /* ndef FEATURE_STATISTICS */
   err = err || map_block_killer(exports, "statistics");
#endif /* ndef FEATURE_STATISTICS */

   if (csp->actions_list)
   {
      if (!err) err = map(exports, "actions-filename", 1,  csp->actions_list->filename, 1);
   }
   else
   {
      if (!err) err = map(exports, "actions-filename", 1, "None specified", 1);
   }

   if (csp->rlist)
   {
      if (!err) err = map(exports, "re-filter-filename", 1,  csp->rlist->filename, 1);
   }
   else
   {
      if (!err) err = map(exports, "re-filter-filename", 1, "None specified", 1);
   }

#ifdef FEATURE_TRUST
   if (csp->tlist)
   {
      if (!err) err = map(exports, "trust-filename", 1,  csp->tlist->filename, 1);
   }
   else
   {
      if (!err) err = map(exports, "trust-filename", 1, "None specified", 1);
   }
#else
   if (!err) err = map_block_killer(exports, "trust-support");
#endif /* ndef FEATURE_TRUST */

   if (err)
   {
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   return template_fill_for_cgi(csp, "show-status", exports, rsp);
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
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_show_url_info(struct client_state *csp,
                         struct http_response *rsp,
                         const struct map *parameters)
{
   const char *url_param_const;
   char *host = NULL;
   struct map *exports;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (NULL == (exports = default_exports(csp, "show-url-info")))
   {
      return JB_ERR_MEMORY;
   }

   url_param_const = lookup(parameters, "url");
   if (*url_param_const == '\0')
   {
      if (map_block_killer(exports, "url-given")
        || map(exports, "url", 1, "", 1))
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }
   }
   else
   {
      char *url_param;
      char *matches;
      char *path;
      char *s;
      int port = 80;
      int hits = 0;
      struct file_list *fl;
      struct url_actions *b;
      struct url_spec url[1];
      struct current_action_spec action[1];
      
      if (NULL == (url_param = strdup(url_param_const)))
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      host = url_param;
      host += (strncmp(url_param, "http://", 7)) ? 0 : 7;

      if (map(exports, "url", 1, host, 1)
       || map(exports, "url-html", 1, html_encode(host), 0))
      {
         free(url_param);
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      init_current_action(action);

      s = current_action_to_text(action);
      if (map(exports, "default", 1, s , 0))
      {
         free_current_action(action);
         free(url_param);
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      if (((fl = csp->actions_list) == NULL) || ((b = fl->f) == NULL))
      {
         jb_err err;

         err = map(exports, "matches", 1, "none" , 1)
            || map(exports, "final", 1, lookup(exports, "default"), 1);

         free_current_action(action);
         free(url_param);

         if (err)
         {
            free_map(exports);
            return JB_ERR_MEMORY;
         }

         return template_fill_for_cgi(csp, "show-url-info", exports, rsp);
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
      if (NULL == path)
      {
         free_current_action(action);
         free(url_param);
         free_map(exports);
         return JB_ERR_MEMORY;
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
         jb_err err;
         
         err = map(exports, "matches", 1, "none" , 1)
            || map(exports, "final", 1, lookup(exports, "default"), 1);

         freez(url_param);
         freez(path);
         free_current_action(action);

         if (err)
         {
            free_map(exports);
            return JB_ERR_MEMORY;
         }

         return template_fill_for_cgi(csp, "show-url-info", exports, rsp);
      }

      matches = strdup("");

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
                  if (s == NULL)
                  {
                     freez(url->dbuf);
                     freez(url->dvec);

                     free(url_param);
                     free(path);
                     free_current_action(action);
                     free_map(exports);
                     return JB_ERR_MEMORY;
                  }
                  string_append(&matches, "<b>{");
                  string_append(&matches, s);
                  string_append(&matches, " }</b><br>\n<code>");
                  string_append(&matches, b->url->spec);
                  string_append(&matches, "</code><br>\n<br>\n");
                  free(s);

                  merge_current_action(action, b->action); /* FIXME: Add error checking */
                  hits++;
               }
            }
         }
      }

      freez(url->dbuf);
      freez(url->dvec);

      free(url_param);
      free(path);

      if (matches == NULL)
      {
         free_current_action(action);
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      if (!hits)
      {
         free(matches);
         matches = strdup("none");
      }
      if (map(exports, "matches", 1, matches , 0))
      {
         free_current_action(action);
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      s = current_action_to_text(action);

      free_current_action(action);

      if (map(exports, "final", 1, s, 0))
      {
         free_map(exports);
         return JB_ERR_MEMORY;
      }
   }

   return template_fill_for_cgi(csp, "show-url-info", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_robots_txt
 *
 * Description :  CGI function to return "/robots.txt".
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : None
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_robots_txt(struct client_state *csp,
                      struct http_response *rsp,
                      const struct map *parameters)
{
   char buf[100];
   jb_err err;

   rsp->body = strdup(
      "# This is the Internet Junkbuster control interface.\n"
      "# It isn't very useful to index it, and you're likely to break stuff.\n"
      "# So go away!\n"
      "\n"
      "User-agent: *\n"
      "Disallow: /\n"
      "\n");
   if (rsp->body == NULL)
   {
      return JB_ERR_MEMORY;
   }

   err = enlist_unique(rsp->headers, "Content-Type: text/plain", 13);

   rsp->is_static = 1;

   get_http_time(7 * 24 * 60 * 60, buf); /* 7 days into future */
   if (!err) err = enlist_unique_header(rsp->headers, "Expires", buf);

   return (err ? JB_ERR_MEMORY : JB_ERR_OK);
}


/*********************************************************************
 *
 * Function    :  show_defines
 *
 * Description :  Add to a map the state od all conditional #defines
 *                used when building
 *
 * Parameters  :
 *           1 :  exports = map to extend
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
static jb_err show_defines(struct map *exports)
{
   jb_err err = JB_ERR_OK;

#ifdef FEATURE_ACL
   if (!err) err = map_conditional(exports, "FEATURE_ACL", 1);
#else /* ifndef FEATURE_ACL */
   if (!err) err = map_conditional(exports, "FEATURE_ACL", 0);
#endif /* ndef FEATURE_ACL */

#ifdef FEATURE_COOKIE_JAR
   if (!err) err = map_conditional(exports, "FEATURE_COOKIE_JAR", 1);
#else /* ifndef FEATURE_COOKIE_JAR */
   if (!err) err = map_conditional(exports, "FEATURE_COOKIE_JAR", 0);
#endif /* ndef FEATURE_COOKIE_JAR */

#ifdef FEATURE_FAST_REDIRECTS
   if (!err) err = map_conditional(exports, "FEATURE_FAST_REDIRECTS", 1);
#else /* ifndef FEATURE_FAST_REDIRECTS */
   if (!err) err = map_conditional(exports, "FEATURE_FAST_REDIRECTS", 0);
#endif /* ndef FEATURE_FAST_REDIRECTS */

#ifdef FEATURE_FORCE_LOAD
   if (!err) err = map_conditional(exports, "FEATURE_FORCE_LOAD", 1);
#else /* ifndef FEATURE_FORCE_LOAD */
   if (!err) err = map_conditional(exports, "FEATURE_FORCE_LOAD", 0);
#endif /* ndef FEATURE_FORCE_LOAD */

#ifdef FEATURE_IMAGE_BLOCKING
   if (!err) err = map_conditional(exports, "FEATURE_IMAGE_BLOCKING", 1);
#else /* ifndef FEATURE_IMAGE_BLOCKING */
   if (!err) err = map_conditional(exports, "FEATURE_IMAGE_BLOCKING", 0);
#endif /* ndef FEATURE_IMAGE_BLOCKING */

#ifdef FEATURE_IMAGE_DETECT_MSIE
   if (!err) err = map_conditional(exports, "FEATURE_IMAGE_DETECT_MSIE", 1);
#else /* ifndef FEATURE_IMAGE_DETECT_MSIE */
   if (!err) err = map_conditional(exports, "FEATURE_IMAGE_DETECT_MSIE", 0);
#endif /* ndef FEATURE_IMAGE_DETECT_MSIE */

#ifdef FEATURE_KILL_POPUPS
   if (!err) err = map_conditional(exports, "FEATURE_KILL_POPUPS", 1);
#else /* ifndef FEATURE_KILL_POPUPS */
   if (!err) err = map_conditional(exports, "FEATURE_KILL_POPUPS", 0);
#endif /* ndef FEATURE_KILL_POPUPS */

#ifdef FEATURE_PTHREAD
   if (!err) err = map_conditional(exports, "FEATURE_PTHREAD", 1);
#else /* ifndef FEATURE_PTHREAD */
   if (!err) err = map_conditional(exports, "FEATURE_PTHREAD", 0);
#endif /* ndef FEATURE_PTHREAD */

#ifdef FEATURE_STATISTICS
   if (!err) err = map_conditional(exports, "FEATURE_STATISTICS", 1);
#else /* ifndef FEATURE_STATISTICS */
   if (!err) err = map_conditional(exports, "FEATURE_STATISTICS", 0);
#endif /* ndef FEATURE_STATISTICS */

#ifdef FEATURE_TOGGLE
   if (!err) err = map_conditional(exports, "FEATURE_TOGGLE", 1);
#else /* ifndef FEATURE_TOGGLE */
   if (!err) err = map_conditional(exports, "FEATURE_TOGGLE", 0);
#endif /* ndef FEATURE_TOGGLE */

#ifdef FEATURE_TRUST
   if (!err) err = map_conditional(exports, "FEATURE_TRUST", 1);
#else /* ifndef FEATURE_TRUST */
   if (!err) err = map_conditional(exports, "FEATURE_TRUST", 0);
#endif /* ndef FEATURE_TRUST */

#ifdef REGEX_GNU
   if (!err) err = map_conditional(exports, "REGEX_GNU", 1);
#else /* ifndef REGEX_GNU */
   if (!err) err = map_conditional(exports, "REGEX_GNU", 0);
#endif /* def REGEX_GNU */

#ifdef REGEX_PCRE
   if (!err) err = map_conditional(exports, "REGEX_PCRE", 1);
#else /* ifndef REGEX_PCRE */
   if (!err) err = map_conditional(exports, "REGEX_PCRE", 0);
#endif /* def REGEX_PCRE */

#ifdef STATIC_PCRE
   if (!err) err = map_conditional(exports, "STATIC_PCRE", 1);
#else /* ifndef STATIC_PCRE */
   if (!err) err = map_conditional(exports, "STATIC_PCRE", 0);
#endif /* ndef STATIC_PCRE */

#ifdef STATIC_PCRS
   if (!err) err = map_conditional(exports, "STATIC_PCRS", 1);
#else /* ifndef STATIC_PCRS */
   if (!err) err = map_conditional(exports, "STATIC_PCRS", 0);
#endif /* ndef STATIC_PCRS */

   if (!err) err = map(exports, "FORCE_PREFIX", 1, FORCE_PREFIX, 1);

   return err;
}


/*********************************************************************
 *
 * Function    :  show_rcs
 *
 * Description :  Create a string with the rcs info for all sourcefiles
 *
 * Parameters  :  None
 *
 * Returns     :  A string, or NULL on out-of-memory.
 *
 *********************************************************************/
static char *show_rcs(void)
{
   char *result = strdup("");
   char buf[BUFFER_SIZE];

   /* Instead of including *all* dot h's in the project (thus creating a
    * tremendous amount of dependencies), I will concede to declaring them
    * as extern's.  This forces the developer to add to this list, but oh well.
    */

#define SHOW_RCS(__x)              \
   {                               \
      extern const char __x[];     \
      sprintf(buf, "%s\n", __x);   \
      string_append(&result, buf); \
   }

   /* In alphabetical order */
   SHOW_RCS(actions_h_rcs)
   SHOW_RCS(actions_rcs)
   SHOW_RCS(cgi_h_rcs)
   SHOW_RCS(cgi_rcs)
#ifdef FEATURE_CGI_EDIT_ACTIONS
   SHOW_RCS(cgiedit_h_rcs)
   SHOW_RCS(cgiedit_rcs)
#endif /* def FEATURE_CGI_EDIT_ACTIONS */
   SHOW_RCS(cgisimple_h_rcs)
   SHOW_RCS(cgisimple_rcs)
#ifdef __MINGW32__
   SHOW_RCS(cygwin_h_rcs)
#endif
   SHOW_RCS(deanimate_h_rcs)
   SHOW_RCS(deanimate_rcs)
   SHOW_RCS(encode_h_rcs)
   SHOW_RCS(encode_rcs)
   SHOW_RCS(errlog_h_rcs)
   SHOW_RCS(errlog_rcs)
   SHOW_RCS(filters_h_rcs)
   SHOW_RCS(filters_rcs)
   SHOW_RCS(gateway_h_rcs)
   SHOW_RCS(gateway_rcs)
#ifdef GNU_REGEX
   SHOW_RCS(gnu_regex_h_rcs)
   SHOW_RCS(gnu_regex_rcs)
#endif /* def GNU_REGEX */
   SHOW_RCS(jbsockets_h_rcs)
   SHOW_RCS(jbsockets_rcs)
   SHOW_RCS(jcc_h_rcs)
   SHOW_RCS(jcc_rcs)
#ifdef FEATURE_KILL_POPUPS
   SHOW_RCS(killpopup_h_rcs)
   SHOW_RCS(killpopup_rcs)
#endif /* def FEATURE_KILL_POPUPS */
   SHOW_RCS(list_h_rcs)
   SHOW_RCS(list_rcs)
   SHOW_RCS(loadcfg_h_rcs)
   SHOW_RCS(loadcfg_rcs)
   SHOW_RCS(loaders_h_rcs)
   SHOW_RCS(loaders_rcs)
   SHOW_RCS(miscutil_h_rcs)
   SHOW_RCS(miscutil_rcs)
   SHOW_RCS(parsers_h_rcs)
   SHOW_RCS(parsers_rcs)
   SHOW_RCS(pcrs_rcs)
   SHOW_RCS(pcrs_h_rcs)
   SHOW_RCS(project_h_rcs)
   SHOW_RCS(ssplit_h_rcs)
   SHOW_RCS(ssplit_rcs)
#ifdef _WIN32
#ifndef _WIN_CONSOLE
   SHOW_RCS(w32log_h_rcs)
   SHOW_RCS(w32log_rcs)
   SHOW_RCS(w32res_h_rcs)
   SHOW_RCS(w32taskbar_h_rcs)
   SHOW_RCS(w32taskbar_rcs)
#endif /* ndef _WIN_CONSOLE */
   SHOW_RCS(win32_h_rcs)
   SHOW_RCS(win32_rcs)
#endif /* def _WIN32 */

#undef SHOW_RCS

   return result;

}


/*
  Local Variables:
  tab-width: 3
  end:
*/
