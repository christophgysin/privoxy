const char loadcfg_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Loads settings from the configuration file into
 *                global variables.  This file contains both the
 *                routine to load the configuration and the global
 *                variables it writes to.
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
 *    Revision 1.29  2002/01/17 21:02:30  jongfoster
 *    Moving all our URL and URL pattern parsing code to urlmatch.c.
 *
 *    Renaming free_url to free_url_spec, since it frees a struct url_spec.
 *
 *    Revision 1.28  2001/12/30 14:07:32  steudten
 *    - Add signal handling (unix)
 *    - Add SIGHUP handler (unix)
 *    - Add creation of pidfile (unix)
 *    - Add action 'top' in rc file (RH)
 *    - Add entry 'SIGNALS' to manpage
 *    - Add exit message to logfile (unix)
 *
 *    Revision 1.27  2001/11/07 00:02:13  steudten
 *    Add line number in error output for lineparsing for
 *    actionsfile and configfile.
 *    Special handling for CLF added.
 *
 *    Revision 1.26  2001/11/05 21:41:43  steudten
 *    Add changes to be a real daemon just for unix os.
 *    (change cwd to /, detach from controlling tty, set
 *    process group and session leader to the own process.
 *    Add DBG() Macro.
 *    Add some fatal-error log message for failed malloc().
 *    Add '-d' if compiled with 'configure --with-debug' to
 *    enable debug output.
 *
 *    Revision 1.25  2001/10/25 03:40:48  david__schmidt
 *    Change in porting tactics: OS/2's EMX porting layer doesn't allow multiple
 *    threads to call select() simultaneously.  So, it's time to do a real, live,
 *    native OS/2 port.  See defines for __EMX__ (the porting layer) vs. __OS2__
 *    (native). Both versions will work, but using __OS2__ offers multi-threading.
 *
 *    Revision 1.24  2001/10/23 21:40:30  jongfoster
 *    Added support for enable-edit-actions and enable-remote-toggle config
 *    file options.
 *
 *    Revision 1.23  2001/10/07 15:36:00  oes
 *    Introduced new config option "buffer-limit"
 *
 *    Revision 1.22  2001/09/22 16:36:59  jongfoster
 *    Removing unused parameter fs from read_config_line()
 *
 *    Revision 1.21  2001/09/16 17:10:43  jongfoster
 *    Moving function savearg() here, since it was the only thing left in
 *    showargs.c.
 *
 *    Revision 1.20  2001/07/30 22:08:36  jongfoster
 *    Tidying up #defines:
 *    - All feature #defines are now of the form FEATURE_xxx
 *    - Permanently turned off WIN_GUI_EDIT
 *    - Permanently turned on WEBDAV and SPLIT_PROXY_ARGS
 *
 *    Revision 1.19  2001/07/15 17:45:16  jongfoster
 *    Removing some unused #includes
 *
 *    Revision 1.18  2001/07/13 14:01:14  oes
 *     - Removed all #ifdef PCRS
 *     - Removed vim-settings
 *
 *    Revision 1.17  2001/06/29 13:31:03  oes
 *    - Improved comments
 *    - Fixed (actionsfile) and sorted hashes
 *    - Introduced admin_address and proxy-info-url
 *      as config parameters
 *    - Renamed config->proxy_args_invocation (which didn't have
 *      the invocation but the options!) to config->proxy_args
 *    - Various adaptions
 *    - Removed logentry from cancelled commit
 *
 *    Revision 1.16  2001/06/09 10:55:28  jongfoster
 *    Changing BUFSIZ ==> BUFFER_SIZE
 *
 *    Revision 1.15  2001/06/07 23:13:40  jongfoster
 *    Merging ACL and forward files into config file.
 *    Cosmetic: Sorting config file options alphabetically.
 *    Cosmetic: Adding brief syntax comments to config file options.
 *
 *    Revision 1.14  2001/06/07 14:46:25  joergs
 *    Missing make_path() added for re_filterfile.
 *
 *    Revision 1.13  2001/06/05 22:33:54  jongfoster
 *
 *    Fixed minor memory leak.
 *    Also now uses make_path to prepend the pathnames.
 *
 *    Revision 1.12  2001/06/05 20:04:09  jongfoster
 *    Now uses _snprintf() in place of snprintf() under Win32.
 *
 *    Revision 1.11  2001/06/04 18:31:58  swa
 *    files are now prefixed with either `confdir' or `logdir'.
 *    `make redhat-dist' replaces both entries confdir and logdir
 *    with redhat values
 *
 *    Revision 1.10  2001/06/03 19:11:54  oes
 *    introduced confdir option
 *
 *    Revision 1.9  2001/06/01 20:06:24  jongfoster
 *    Removed support for "tinygif" option - moved to actions file.
 *
 *    Revision 1.8  2001/05/31 21:27:13  jongfoster
 *    Removed many options from the config file and into the
 *    "actions" file: add_forwarded, suppress_vanilla_wafer,
 *    wafer, add_header, user_agent, referer, from
 *    Also globally replaced "permission" with "action".
 *
 *    Revision 1.7  2001/05/29 09:50:24  jongfoster
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
 *    Revision 1.3  2001/05/20 01:21:20  jongfoster
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
 *    Revision 1.2  2001/05/17 23:01:01  oes
 *     - Cleaned CRLF's from the sources and related files
 *
 *    Revision 1.1.1.1  2001/05/15 13:58:58  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#ifdef _WIN32

# include <windows.h>

# include "win32.h"
# ifndef _WIN_CONSOLE
#  include "w32log.h"
# endif /* ndef _WIN_CONSOLE */

/* VC++ has "_snprintf", not "snprintf" */
#define snprintf _snprintf

#else /* ifndef _WIN32 */

#ifndef __OS2__
# include <unistd.h>
# include <sys/wait.h>
#endif
# include <sys/time.h>
# include <sys/stat.h>
# include <signal.h>

#endif

#include "loadcfg.h"
#include "list.h"
#include "jcc.h"
#include "filters.h"
#include "loaders.h"
#include "miscutil.h"
#include "errlog.h"
#include "ssplit.h"
#include "encode.h"
#include "urlmatch.h"

const char loadcfg_h_rcs[] = LOADCFG_H_VERSION;

/*
 * Fix a problem with Solaris.  There should be no effect on other
 * platforms.
 * Solaris's isspace() is a macro which uses it's argument directly
 * as an array index.  Therefore we need to make sure that high-bit
 * characters generate +ve values, and ideally we also want to make
 * the argument match the declared parameter type of "int".
 */
#define ijb_isupper(__X) isupper((int)(unsigned char)(__X))
#define ijb_tolower(__X) tolower((int)(unsigned char)(__X))

#ifdef FEATURE_TOGGLE
/* by haroon - indicates if ijb is enabled */
int g_bToggleIJB        = 1;   /* JunkBusters is enabled by default. */
#endif /* def FEATURE_TOGGLE */

/* The filename of the configfile */
const char *configfile  = NULL;

/*
 * CGI functions will later need access to the invocation args,
 * so we will make argc and argv global.
 */
int Argc = 0;
const char **Argv = NULL;

static struct file_list *current_configfile = NULL;


/*
 * This takes the "cryptic" hash of each keyword and aliases them to
 * something a little more readable.  This also makes changing the
 * hash values easier if they should change or the hash algorthm changes.
 * Use the included "hash" program to find out what the hash will be
 * for any string supplied on the command line.  (Or just put it in the
 * config file and read the number from the error message in the log).
 *
 * Please keep this list sorted alphabetically (but with the Windows
 * console and GUI specific options last).
 */

#define hash_actions_file              1196306641ul /* "actionsfile" */
#define hash_admin_address             4112573064ul /* "admin-address" */
#define hash_buffer_limit              1881726070ul /* "buffer-limit */
#define hash_confdir                      1978389ul /* "confdir" */
#define hash_debug                          78263ul /* "debug" */
#define hash_deny_access               1227333715ul /* "deny-access" */
#define hash_enable_edit_actions       2517097536ul /* "enable-edit-actions" */
#define hash_enable_remote_toggle      2979744683ul /* "enable-remote-toggle" */
#define hash_forward                      2029845ul /* "forward" */
#define hash_forward_socks4            3963965521ul /* "forward-socks4" */
#define hash_forward_socks4a           2639958518ul /* "forward-socks4a" */
#define hash_jarfile                      2046641ul /* "jarfile" */
#define hash_listen_address            1255650842ul /* "listen-address" */
#define hash_logdir                        422889ul /* "logdir" */
#define hash_logfile                      2114766ul /* "logfile" */
#define hash_permit_access             3587953268ul /* "permit-access" */
#define hash_proxy_info_url            3903079059ul /* "proxy-info-url" */
#define hash_re_filterfile             3877522444ul /* "re_filterfile" */
#define hash_single_threaded           4250084780ul /* "single-threaded" */
#define hash_suppress_blocklists       1948693308ul /* "suppress-blocklists" */
#define hash_toggle                        447966ul /* "toggle" */
#define hash_trust_info_url             430331967ul /* "trust-info-url" */
#define hash_trustfile                   56494766ul /* "trustfile" */

#define hash_activity_animation        1817904738ul /* "activity-animation" */
#define hash_close_button_minimizes    3651284693ul /* "close-button-minimizes" */
#define hash_hide_console              2048809870ul /* "hide-console" */
#define hash_log_buffer_size           2918070425ul /* "log-buffer-size" */
#define hash_log_font_name             2866730124ul /* "log-font-name" */
#define hash_log_font_size             2866731014ul /* "log-font-size" */
#define hash_log_highlight_messages    4032101240ul /* "log-highlight-messages" */
#define hash_log_max_lines             2868344173ul /* "log-max-lines" */
#define hash_log_messages              2291744899ul /* "log-messages" */
#define hash_show_on_task_bar           215410365ul /* "show-on-task-bar" */


static void savearg(char *c, char *o, struct configuration_spec * config);

/*********************************************************************
 *
 * Function    :  unload_configfile
 *
 * Description :  Free the config structure and all components.
 *
 * Parameters  :
 *          1  :  data: struct configuration_spec to unload
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void unload_configfile (void * data)
{
   struct configuration_spec * config = (struct configuration_spec *)data;
   struct forward_spec *cur_fwd = config->forward;
#ifdef FEATURE_ACL
   struct access_control_list *cur_acl = config->acl;

   while (cur_acl != NULL)
   {
      struct access_control_list * next_acl = cur_acl->next;
      free(cur_acl);
      cur_acl = next_acl;
   }
   config->acl = NULL;
#endif /* def FEATURE_ACL */

   while (cur_fwd != NULL)
   {
      struct forward_spec * next_fwd = cur_fwd->next;
      free_url_spec(cur_fwd->url);

      freez(cur_fwd->gateway_host);
      freez(cur_fwd->forward_host);
      free(cur_fwd);
      cur_fwd = next_fwd;
   }
   config->forward = NULL;

#ifdef FEATURE_COOKIE_JAR
   if ( NULL != config->jar )
   {
      fclose( config->jar );
      config->jar = NULL;
   }
#endif /* def FEATURE_COOKIE_JAR */

   freez(config->confdir);
   freez(config->logdir);

   freez(config->haddr);
   freez(config->logfile);

   freez(config->actions_file);
   freez(config->admin_address);
   freez(config->proxy_info_url);
   freez(config->proxy_args);

#ifdef FEATURE_COOKIE_JAR
   freez(config->jarfile);
#endif /* def FEATURE_COOKIE_JAR */

   freez(config->re_filterfile);

}


/*********************************************************************
 *
 * Function    :  load_config
 *
 * Description :  Load the config file and all parameters.
 *
 * Parameters  :
 *          1  :  csp = Client state (the config member will be
 *                filled in by this function).
 *
 * Returns     :  0 => Ok, everything else is an error.
 *
 *********************************************************************/
struct configuration_spec * load_config(void)
{
   char buf[BUFFER_SIZE];
   char *p, *q;
   FILE *configfp = NULL;
   struct configuration_spec * config = NULL;
   struct client_state * fake_csp;
   struct file_list *fs;
   unsigned long linenum = 0;

   DBG(1, ("load_config() entered..\n") );
   if ( !check_file_changed(current_configfile, configfile, &fs))
   {
      /* No need to load */
      return ((struct configuration_spec *)current_configfile->f);
   }
   if (!fs)
   {
      log_error(LOG_LEVEL_FATAL, "can't check configuration file '%s':  %E",
                configfile);
   }

   /*
   log_error(LOG_LEVEL_INFO, "loading configuration file '%s':", configfile);
   */

#ifdef FEATURE_TOGGLE
   g_bToggleIJB      = 1;
#endif /* def FEATURE_TOGGLE */

   fs->f = config = (struct configuration_spec *)zalloc(sizeof(*config));

   if (config==NULL)
   {
      freez(fs->filename);
      freez(fs);
      log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
      /* Never get here - LOG_LEVEL_FATAL causes program exit */
   }

   /*
    * This is backwards from how it's usually done.
    * Following the usual pattern, "fs" would be stored in a member
    * variable in "csp", and then we'd access "config" from "fs->f",
    * using a cast.  However, "config" is used so often that a
    * cast each time would be very ugly, and the extra indirection
    * would waste CPU cycles.  Therefore we store "config" in
    * "csp->config", and "fs" in "csp->config->config_file_list".
    */
   config->config_file_list = fs;

   /*
    * Set to defaults
    */

   config->multi_threaded    = 1;
   config->hport             = HADDR_PORT;
   config->buffer_limit      = 4096 * 1024;

   if ((configfp = fopen(configfile, "r")) == NULL)
   {
      log_error(LOG_LEVEL_FATAL, "can't open configuration file '%s':  %E",
              configfile);
      /* Never get here - LOG_LEVEL_FATAL causes program exit */
   }

   while (read_config_line(buf, sizeof(buf), configfp, &linenum) != NULL)
   {
      char cmd[BUFFER_SIZE];
      char arg[BUFFER_SIZE];
      char tmp[BUFFER_SIZE];
#ifdef FEATURE_ACL
      struct access_control_list *cur_acl;
#endif /* def FEATURE_ACL */
      struct forward_spec *cur_fwd;
      int vec_count;
      char *vec[3];

      strcpy(tmp, buf);

      /* Copy command (i.e. up to space or tab) into cmd */
      p = buf;
      q = cmd;
      while (*p && (*p != ' ') && (*p != '\t'))
      {
         *q++ = *p++;
      }
      *q = '\0';

      /* Skip over the whitespace in buf */
      while (*p && ((*p == ' ') || (*p == '\t')))
      {
         p++;
      }

      /* Copy the argument into arg */
      strcpy(arg, p);

      /* Should never happen, but check this anyway */
      if (*cmd == '\0')
      {
         continue;
      }

      /* Make sure the command field is lower case */
      for (p=cmd; *p; p++)
      {
         if (ijb_isupper(*p))
         {
            *p = ijb_tolower(*p);
         }
      }

      /* Save the argument for show-proxy-args */
      savearg(cmd, arg, config);


      switch( hash_string( cmd ) )
      {
/****************************************************************************
 * actionsfile actions-file-name
 * In confdir by default
 ****************************************************************************/
         case hash_actions_file :
            freez(config->actions_file);
            config->actions_file = make_path(config->confdir, arg);
            continue;

/****************************************************************************
 * admin-address email-address
 ****************************************************************************/
         case hash_admin_address :
            freez(config->admin_address);
            config->admin_address = strdup(arg);
            continue;

/****************************************************************************
 * buffer-limit n
 ****************************************************************************/
         case hash_buffer_limit :
            config->buffer_limit = (size_t) 1024 * atoi(arg);
            continue;

/****************************************************************************
 * confdir directory-name
 ****************************************************************************/
         case hash_confdir :
            freez(config->confdir);
            config->confdir = make_path( NULL, arg);
            continue;

/****************************************************************************
 * debug n
 * Specifies debug level, multiple values are ORed together.
 ****************************************************************************/
         case hash_debug :
            config->debug |= atoi(arg);
            continue;

/****************************************************************************
 * deny-access source-ip[/significant-bits] [dest-ip[/significant-bits]]
 ****************************************************************************/
#ifdef FEATURE_ACL
         case hash_deny_access:
            vec_count = ssplit(arg, " \t", vec, SZ(vec), 1, 1);

            if ((vec_count != 1) && (vec_count != 2))
            {
               log_error(LOG_LEVEL_ERROR, "Wrong number of parameters for "
                     "deny-access directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Wrong number of parameters for "
                  "deny-access directive in configuration file.<br><br>\n");
               continue;
            }

            /* allocate a new node */
            cur_acl = (struct access_control_list *) zalloc(sizeof(*cur_acl));

            if (cur_acl == NULL)
            {
               log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
               /* Never get here - LOG_LEVEL_FATAL causes program exit */
               continue;
            }
            cur_acl->action = ACL_DENY;

            if (acl_addr(vec[0], cur_acl->src) < 0)
            {
               log_error(LOG_LEVEL_ERROR, "Invalid source IP for deny-access "
                     "directive in configuration file: \"%s\"", vec[0]);
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Invalid source IP for deny-access directive"
                  " in configuration file: \"");
               config->proxy_args = strsav( config->proxy_args,
                  vec[0]);
               config->proxy_args = strsav( config->proxy_args,
                  "\"<br><br>\n");
               freez(cur_acl);
               continue;
            }
            if (vec_count == 2)
            {
               if (acl_addr(vec[1], cur_acl->dst) < 0)
               {
                  log_error(LOG_LEVEL_ERROR, "Invalid destination IP for deny-access "
                        "directive in configuration file: \"%s\"", vec[0]);
                  config->proxy_args = strsav( config->proxy_args,
                     "<br>\nWARNING: Invalid destination IP for deny-access directive"
                     " in configuration file: \"");
                  config->proxy_args = strsav( config->proxy_args,
                     vec[0]);
                  config->proxy_args = strsav( config->proxy_args,
                     "\"<br><br>\n");
                  freez(cur_acl);
                  continue;
               }
            }

            /*
             * Add it to the list.  Note we reverse the list to get the
             * behaviour the user expects.  With both the ACL and
             * actions file, the last match wins.  However, the internal
             * implementations are different:  The actions file is stored
             * in the same order as the file, and scanned completely.
             * With the ACL, we reverse the order as we load it, then
             * when we scan it we stop as soon as we get a match.
             */
            cur_acl->next  = config->acl;
            config->acl = cur_acl;

            continue;
#endif /* def FEATURE_ACL */

/****************************************************************************
 * enable-edit-actions 0|1
 ****************************************************************************/
#ifdef FEATURE_CGI_EDIT_ACTIONS
         case hash_enable_edit_actions:
            if ((*arg != '\0') && (0 != atoi(arg)))
            {
               config->feature_flags |= RUNTIME_FEATURE_CGI_EDIT_ACTIONS;
            }
            else
            {
               config->feature_flags &= ~RUNTIME_FEATURE_CGI_EDIT_ACTIONS;
            }
            continue;
#endif /* def FEATURE_CGI_EDIT_ACTIONS */

/****************************************************************************
 * enable-remote-toggle 0|1
 ****************************************************************************/
#ifdef FEATURE_CGI_EDIT_ACTIONS
         case hash_enable_remote_toggle:
            if ((*arg != '\0') && (0 != atoi(arg)))
            {
               config->feature_flags |= RUNTIME_FEATURE_CGI_TOGGLE;
            }
            else
            {
               config->feature_flags &= ~RUNTIME_FEATURE_CGI_TOGGLE;
            }
            continue;
#endif /* def FEATURE_CGI_EDIT_ACTIONS */

/****************************************************************************
 * forward url-pattern (.|http-proxy-host[:port])
 ****************************************************************************/
         case hash_forward:
            vec_count = ssplit(arg, " \t", vec, SZ(vec), 1, 1);

            if (vec_count != 2)
            {
               log_error(LOG_LEVEL_ERROR, "Wrong number of parameters for forward "
                     "directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Wrong number of parameters for "
                  "forward directive in configuration file.");
               continue;
            }

            /* allocate a new node */
            cur_fwd = zalloc(sizeof(*cur_fwd));
            if (cur_fwd == NULL)
            {
               log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
               /* Never get here - LOG_LEVEL_FATAL causes program exit */
               continue;
            }

            cur_fwd->type = SOCKS_NONE;

            /* Save the URL pattern */
            if (create_url_spec(cur_fwd->url, vec[0]))
            {
               log_error(LOG_LEVEL_ERROR, "Bad URL specifier for forward "
                     "directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Bad URL specifier for "
                  "forward directive in configuration file.");
               continue;
            }

            /* Parse the parent HTTP proxy host:port */
            p = vec[1];

            if (strcmp(p, ".") != 0)
            {
               cur_fwd->forward_host = strdup(p);

               if ((p = strchr(cur_fwd->forward_host, ':')))
               {
                  *p++ = '\0';
                  cur_fwd->forward_port = atoi(p);
               }

               if (cur_fwd->forward_port <= 0)
               {
                  cur_fwd->forward_port = 8000;
               }
            }

            /* Add to list. */
            cur_fwd->next = config->forward;
            config->forward = cur_fwd;

            continue;

/****************************************************************************
 * forward-socks4 url-pattern socks-proxy[:port] (.|http-proxy[:port])
 ****************************************************************************/
         case hash_forward_socks4:
            vec_count = ssplit(arg, " \t", vec, SZ(vec), 1, 1);

            if (vec_count != 3)
            {
               log_error(LOG_LEVEL_ERROR, "Wrong number of parameters for "
                     "forward-socks4 directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Wrong number of parameters for "
                  "forward-socks4 directive in configuration file.");
               continue;
            }

            /* allocate a new node */
            cur_fwd = zalloc(sizeof(*cur_fwd));
            if (cur_fwd == NULL)
            {
               log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
               /* Never get here - LOG_LEVEL_FATAL causes program exit */
               continue;
            }

            cur_fwd->type = SOCKS_4;

            /* Save the URL pattern */
            if (create_url_spec(cur_fwd->url, vec[0]))
            {
               log_error(LOG_LEVEL_ERROR, "Bad URL specifier for forward-socks4 "
                     "directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Bad URL specifier for "
                  "forward-socks4 directive in configuration file.");
               continue;
            }

            /* Parse the SOCKS proxy host[:port] */
            p = vec[1];

            if (strcmp(p, ".") != 0)
            {
               cur_fwd->gateway_host = strdup(p);

               if ((p = strchr(cur_fwd->gateway_host, ':')))
               {
                  *p++ = '\0';
                  cur_fwd->gateway_port = atoi(p);
               }
               if (cur_fwd->gateway_port <= 0)
               {
                  cur_fwd->gateway_port = 1080;
               }
            }

            /* Parse the parent HTTP proxy host[:port] */
            p = vec[2];

            if (strcmp(p, ".") != 0)
            {
               cur_fwd->forward_host = strdup(p);

               if ((p = strchr(cur_fwd->forward_host, ':')))
               {
                  *p++ = '\0';
                  cur_fwd->forward_port = atoi(p);
               }

               if (cur_fwd->forward_port <= 0)
               {
                  cur_fwd->forward_port = 8000;
               }
            }

            /* Add to list. */
            cur_fwd->next = config->forward;
            config->forward = cur_fwd;

            continue;

/****************************************************************************
 * forward-socks4a url-pattern socks-proxy[:port] (.|http-proxy[:port])
 ****************************************************************************/
         case hash_forward_socks4a:
            vec_count = ssplit(arg, " \t", vec, SZ(vec), 1, 1);

            if (vec_count != 3)
            {
               log_error(LOG_LEVEL_ERROR, "Wrong number of parameters for "
                     "forward-socks4a directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Wrong number of parameters for "
                  "forward-socks4a directive in configuration file.");
               continue;
            }

            /* allocate a new node */
            cur_fwd = zalloc(sizeof(*cur_fwd));
            if (cur_fwd == NULL)
            {
               log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
               /* Never get here - LOG_LEVEL_FATAL causes program exit */
               continue;
            }

            cur_fwd->type = SOCKS_4A;

            /* Save the URL pattern */
            if (create_url_spec(cur_fwd->url, vec[0]))
            {
               log_error(LOG_LEVEL_ERROR, "Bad URL specifier for forward-socks4a "
                     "directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Bad URL specifier for "
                  "forward-socks4a directive in configuration file.");
               continue;
            }

            /* Parse the SOCKS proxy host[:port] */
            p = vec[1];

            cur_fwd->gateway_host = strdup(p);

            if ((p = strchr(cur_fwd->gateway_host, ':')))
            {
               *p++ = '\0';
               cur_fwd->gateway_port = atoi(p);
            }
            if (cur_fwd->gateway_port <= 0)
            {
               cur_fwd->gateway_port = 1080;
            }

            /* Parse the parent HTTP proxy host[:port] */
            p = vec[2];

            if (strcmp(p, ".") != 0)
            {
               cur_fwd->forward_host = strdup(p);

               if ((p = strchr(cur_fwd->forward_host, ':')))
               {
                  *p++ = '\0';
                  cur_fwd->forward_port = atoi(p);
               }

               if (cur_fwd->forward_port <= 0)
               {
                  cur_fwd->forward_port = 8000;
               }
            }

            /* Add to list. */
            cur_fwd->next = config->forward;
            config->forward = cur_fwd;

            continue;

/****************************************************************************
 * jarfile jar-file-name
 * In logdir by default
 ****************************************************************************/
#ifdef FEATURE_COOKIE_JAR
         case hash_jarfile :
            freez(config->jarfile);
            config->jarfile = make_path(config->logdir, arg);
            continue;
#endif /* def FEATURE_COOKIE_JAR */

/****************************************************************************
 * listen-address [ip][:port]
 ****************************************************************************/
         case hash_listen_address :
            freez(config->haddr);
            config->haddr = strdup(arg);
            continue;

/****************************************************************************
 * logdir directory-name
 ****************************************************************************/
         case hash_logdir :
            freez(config->logdir);
            config->logdir = make_path(NULL, arg);
            continue;

/****************************************************************************
 * logfile log-file-name
 * In logdir by default
 ****************************************************************************/
         case hash_logfile :
            freez(config->logfile);
            config->logfile = make_path(config->logdir, arg);
            continue;

/****************************************************************************
 * permit-access source-ip[/significant-bits] [dest-ip[/significant-bits]]
 ****************************************************************************/
#ifdef FEATURE_ACL
         case hash_permit_access:
            vec_count = ssplit(arg, " \t", vec, SZ(vec), 1, 1);

            if ((vec_count != 1) && (vec_count != 2))
            {
               log_error(LOG_LEVEL_ERROR, "Wrong number of parameters for "
                     "permit-access directive in configuration file.");
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Wrong number of parameters for "
                  "permit-access directive in configuration file.<br><br>\n");

               continue;
            }

            /* allocate a new node */
            cur_acl = (struct access_control_list *) zalloc(sizeof(*cur_acl));

            if (cur_acl == NULL)
            {
               log_error(LOG_LEVEL_FATAL, "can't allocate memory for configuration");
               /* Never get here - LOG_LEVEL_FATAL causes program exit */
               continue;
            }
            cur_acl->action = ACL_PERMIT;

            if (acl_addr(vec[0], cur_acl->src) < 0)
            {
               log_error(LOG_LEVEL_ERROR, "Invalid source IP for permit-access "
                     "directive in configuration file: \"%s\"", vec[0]);
               config->proxy_args = strsav( config->proxy_args,
                  "<br>\nWARNING: Invalid source IP for permit-access directive"
                  " in configuration file: \"");
               config->proxy_args = strsav( config->proxy_args,
                  vec[0]);
               config->proxy_args = strsav( config->proxy_args,
                  "\"<br><br>\n");
               freez(cur_acl);
               continue;
            }
            if (vec_count == 2)
            {
               if (acl_addr(vec[1], cur_acl->dst) < 0)
               {
                  log_error(LOG_LEVEL_ERROR, "Invalid destination IP for "
                        "permit-access directive in configuration file: \"%s\"",
                        vec[0]);
                  config->proxy_args = strsav( config->proxy_args,
                     "<br>\nWARNING: Invalid destination IP for permit-access directive"
                     " in configuration file: \"");
                  config->proxy_args = strsav( config->proxy_args,
                     vec[0]);
                  config->proxy_args = strsav( config->proxy_args,
                     "\"<br><br>\n");
                  freez(cur_acl);
                  continue;
               }
            }

            /*
             * Add it to the list.  Note we reverse the list to get the
             * behaviour the user expects.  With both the ACL and
             * actions file, the last match wins.  However, the internal
             * implementations are different:  The actions file is stored
             * in the same order as the file, and scanned completely.
             * With the ACL, we reverse the order as we load it, then
             * when we scan it we stop as soon as we get a match.
             */
            cur_acl->next  = config->acl;
            config->acl = cur_acl;

            continue;
#endif /* def FEATURE_ACL */

/****************************************************************************
 * proxy-info-url url
 ****************************************************************************/
         case hash_proxy_info_url :
            freez(config->proxy_info_url);
            config->proxy_info_url = strdup(arg);
            continue;

/****************************************************************************
 * re_filterfile file-name
 * In confdir by default.
 ****************************************************************************/
         case hash_re_filterfile :
            freez(config->re_filterfile);
            config->re_filterfile = make_path(config->confdir, arg);
            continue;

/****************************************************************************
 * single-threaded
 ****************************************************************************/
         case hash_single_threaded :
            config->multi_threaded = 0;
            continue;

/****************************************************************************
 * toggle (0|1)
 ****************************************************************************/
#ifdef FEATURE_TOGGLE
         case hash_toggle :
            g_bToggleIJB = atoi(arg);
            continue;
#endif /* def FEATURE_TOGGLE */

/****************************************************************************
 * trust-info-url url
 ****************************************************************************/
#ifdef FEATURE_TRUST
         case hash_trust_info_url :
            enlist(config->trust_info, arg);
            continue;
#endif /* def FEATURE_TRUST */

/****************************************************************************
 * trustfile filename
 * (In confdir by default.)
 ****************************************************************************/
#ifdef FEATURE_TRUST
         case hash_trustfile :
            freez(config->trustfile);
            config->trustfile = make_path(config->confdir, arg);
            continue;
#endif /* def FEATURE_TRUST */


/****************************************************************************
 * Win32 Console options:
 ****************************************************************************/

/****************************************************************************
 * hide-console
 ****************************************************************************/
#ifdef _WIN_CONSOLE
         case hash_hide_console :
            hideConsole = 1;
            continue;
#endif /*def _WIN_CONSOLE*/


/****************************************************************************
 * Win32 GUI options:
 ****************************************************************************/

#if defined(_WIN32) && ! defined(_WIN_CONSOLE)
/****************************************************************************
 * activity-animation (0|1)
 ****************************************************************************/
         case hash_activity_animation :
            g_bShowActivityAnimation = atoi(arg);
            continue;

/****************************************************************************
 *  close-button-minimizes (0|1)
 ****************************************************************************/
         case hash_close_button_minimizes :
            g_bCloseHidesWindow = atoi(arg);
            continue;

/****************************************************************************
 * log-buffer-size (0|1)
 ****************************************************************************/
         case hash_log_buffer_size :
            g_bLimitBufferSize = atoi(arg);
            continue;

/****************************************************************************
 * log-font-name fontnane
 ****************************************************************************/
         case hash_log_font_name :
            strcpy( g_szFontFaceName, arg );
            continue;

/****************************************************************************
 * log-font-size n
 ****************************************************************************/
         case hash_log_font_size :
            g_nFontSize = atoi(arg);
            continue;

/****************************************************************************
 * log-highlight-messages (0|1)
 ****************************************************************************/
         case hash_log_highlight_messages :
            g_bHighlightMessages = atoi(arg);
            continue;

/****************************************************************************
 * log-max-lines n
 ****************************************************************************/
         case hash_log_max_lines :
            g_nMaxBufferLines = atoi(arg);
            continue;

/****************************************************************************
 * log-messages (0|1)
 ****************************************************************************/
         case hash_log_messages :
            g_bLogMessages = atoi(arg);
            continue;

/****************************************************************************
 * show-on-task-bar (0|1)
 ****************************************************************************/
         case hash_show_on_task_bar :
            g_bShowOnTaskBar = atoi(arg);
            continue;

#endif /* defined(_WIN32) && ! defined(_WIN_CONSOLE) */


/****************************************************************************/
/* Warnings about unsupported features                                      */
/****************************************************************************/
#ifndef FEATURE_ACL
         case hash_deny_access:
#endif /* ndef FEATURE_ACL */
#ifndef FEATURE_CGI_EDIT_ACTIONS
         case hash_enable_edit_actions:
         case hash_enable_remote_toggle:
#endif /* def FEATURE_CGI_EDIT_ACTIONS */
#ifndef FEATURE_COOKIE_JAR
         case hash_jarfile :
#endif /* ndef FEATURE_COOKIE_JAR */
#ifndef FEATURE_ACL
         case hash_permit_access:
#endif /* ndef FEATURE_ACL */
#ifndef FEATURE_TOGGLE
         case hash_toggle :
#endif /* ndef FEATURE_TOGGLE */
#ifndef FEATURE_TRUST
         case hash_trustfile :
         case hash_trust_info_url :
#endif /* ndef FEATURE_TRUST */

#ifndef _WIN_CONSOLE
         case hash_hide_console :
#endif /* ndef _WIN_CONSOLE */

#if defined(_WIN_CONSOLE) || ! defined(_WIN32)
         case hash_activity_animation :
         case hash_close_button_minimizes :
         case hash_log_buffer_size :
         case hash_log_font_name :
         case hash_log_font_size :
         case hash_log_highlight_messages :
         case hash_log_max_lines :
         case hash_log_messages :
         case hash_show_on_task_bar :
#endif /* defined(_WIN_CONSOLE) || ! defined(_WIN32) */
            /* These warnings are annoying - so hide them. -- Jon */
            /* log_error(LOG_LEVEL_INFO, "Unsupported directive \"%s\" ignored.", cmd); */
            continue;

/****************************************************************************/
         default :
/****************************************************************************/
            /*
             * I decided that I liked this better as a warning than an
             * error.  To change back to an error, just change log level
             * to LOG_LEVEL_FATAL.
             */
            log_error(LOG_LEVEL_ERROR, "Unrecognized directive '%s' in line %lu in "
                  "configuration file (%s).",  buf, linenum, configfile);
            /* log_error(LOG_LEVEL_ERROR, "Unrecognized directive (%luul) in "
                  "configuration file: \"%s\"", hash_string( cmd ), buf);
 	    */
            config->proxy_args = strsav( config->proxy_args, "<br>\nWARNING: unrecognized directive : ");
            config->proxy_args = strsav( config->proxy_args, buf);
            config->proxy_args = strsav( config->proxy_args, "<br><br>\n");
            continue;

/****************************************************************************/
      } /* end switch( hash_string(cmd) ) */
   } /* end while ( read_config_line(...) ) */

   fclose(configfp);

   init_error_log(Argv[0], config->logfile, config->debug);

   if (config->actions_file)
   {
      add_loader(load_actions_file, config);
   }

   if (config->re_filterfile)
   {
      add_loader(load_re_filterfile, config);
   }

#ifdef FEATURE_TRUST
   if (config->trustfile)
   {
      add_loader(load_trustfile, config);
   }
#endif /* def FEATURE_TRUST */

#ifdef FEATURE_COOKIE_JAR
   if ( NULL != config->jarfile )
   {
      if ( NULL == (config->jar = fopen(config->jarfile, "a")) )
      {
         log_error(LOG_LEVEL_FATAL, "can't open jarfile '%s': %E", config->jarfile);
         /* Never get here - LOG_LEVEL_FATAL causes program exit */
      }
      setbuf(config->jar, NULL);
   }
#endif /* def FEATURE_COOKIE_JAR */

   if ( NULL == config->haddr )
   {
      config->haddr = strdup( HADDR_DEFAULT );
   }

   if ( NULL != config->haddr )
   {
      if ((p = strchr(config->haddr, ':')))
      {
         *p++ = '\0';
         if (*p)
         {
            config->hport = atoi(p);
         }
      }

      if (config->hport <= 0)
      {
         *--p = ':';
         log_error(LOG_LEVEL_FATAL, "invalid bind port spec %s", config->haddr);
         /* Never get here - LOG_LEVEL_FATAL causes program exit */
      }
      if (*config->haddr == '\0')
      {
         config->haddr = NULL;
      }
   }

   /*
    * Want to run all the loaders once now.
    *
    * Need to set up a fake csp, so they can get to the config.
    */
   fake_csp = (struct client_state *) zalloc (sizeof(*fake_csp));
   fake_csp->config = config;

   if (run_loader(fake_csp))
   {
      freez(fake_csp);
      log_error(LOG_LEVEL_FATAL, "A loader failed while loading config file. Exiting.");
      /* Never get here - LOG_LEVEL_FATAL causes program exit */
   }
   freez(fake_csp);

/* FIXME: this is a kludge for win32 */
#if defined(_WIN32) && !defined (_WIN_CONSOLE)

   g_actions_file     = config->actions_file;
   g_re_filterfile    = config->re_filterfile;

#ifdef FEATURE_TRUST
   g_trustfile        = config->trustfile;
#endif /* def FEATURE_TRUST */


#endif /* defined(_WIN32) && !defined (_WIN_CONSOLE) */
/* FIXME: end kludge */


   config->need_bind = 1;

   if (current_configfile)
   {
      struct configuration_spec * oldcfg = (struct configuration_spec *)
                                           current_configfile->f;
      /*
       * Check if config->haddr,hport == oldcfg->haddr,hport
       *
       * The following could be written more compactly as a single,
       * (unreadably long) if statement.
       */
      config->need_bind = 0;
      if (config->hport != oldcfg->hport)
      {
         config->need_bind = 1;
      }
      else if (config->haddr == NULL)
      {
         if (oldcfg->haddr != NULL)
         {
            config->need_bind = 1;
         }
      }
      else if (oldcfg->haddr == NULL)
      {
         config->need_bind = 1;
      }
      else if (0 != strcmp(config->haddr, oldcfg->haddr))
      {
         config->need_bind = 1;
      }

      current_configfile->unloader = unload_configfile;
   }

   fs->next = files->next;
   files->next = fs;

   current_configfile = fs;
   MustReload = 0;

   return (config);
}


/*********************************************************************
 *
 * Function    :  savearg
 *
 * Description :  Called from `load_config'.  It saves each non-empty
 *                and non-comment line from config into a list.  This
 *                list is used to create the show-proxy-args page.
 *
 * Parameters  :
 *          1  :  c = config setting that was found
 *          2  :  o = the setting's argument (if any)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void savearg(char *c, char *o, struct configuration_spec * config)
{
   char buf[BUFFER_SIZE];

   *buf = '\0';

   if ( ( NULL != c ) && ( '\0' != *c ) )
   {
      if ((c = html_encode(c)))
      {
         sprintf(buf, "<a href=\"" REDIRECT_URL "option#%s\">%s</a> ", c, c);
      }
      freez(c);
   }
   if ( ( NULL != o ) && ( '\0' != *o ) )
   {
      if ((o = html_encode(o)))
      {
         if (strncmpic(o, "http://", 7) == 0)
         {
            strcat(buf, "<a href=\"");
            strcat(buf, o);
            strcat(buf, "\">");
            strcat(buf, o);
            strcat(buf, "</a>");
         }
         else
         {
            strcat(buf, o);
         }
      }
      freez(o);
   }

   strcat(buf, "<br>\n");

   config->proxy_args = strsav(config->proxy_args, buf);

}


/*
  Local Variables:
  tab-width: 3
  end:
*/
