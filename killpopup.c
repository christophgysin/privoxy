const char killpopup_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Handles the filtering of popups.
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
 *    Revision 1.10  2001/09/22 16:34:44  jongfoster
 *    Removing unneeded #includes
 *
 *    Revision 1.9  2001/07/31 14:44:22  oes
 *    Deleted unused size parameter from filter_popups()
 *
 *    Revision 1.8  2001/07/30 22:08:36  jongfoster
 *    Tidying up #defines:
 *    - All feature #defines are now of the form FEATURE_xxx
 *    - Permanently turned off WIN_GUI_EDIT
 *    - Permanently turned on WEBDAV and SPLIT_PROXY_ARGS
 *
 *    Revision 1.7  2001/07/20 19:29:25  haroon
 *    - In v1.5 forgot to add that I implemented LOG_LEVEL_POPUPS in errlog.c,
 *      errlog.h and killpopup.c. In that case, it is superfluous to have define for
 *      POPUP_VERBOSE, so I removed the defines and logging is now done
 *      via log_error(LOG_LEVEL_POPUPS, ....)
 *
 *    Revision 1.6  2001/07/19 19:11:35  haroon
 *    - Implemented Guy's idea of replacing window.open( with 1;''.concat(
 *    - Implemented Guy's idea of replacing .resizeTo( with .scrollTo(
 *
 *    Revision 1.5  2001/07/18 15:02:52  haroon
 *    improved nuking of window.open
 *
 *    Revision 1.4  2001/06/29 13:29:55  oes
 *    Added FIXMEs (and didn't repair, hehe)
 *
 *    Revision 1.3  2001/05/22 18:56:28  oes
 *    CRLF -> LF
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
 *    Revision 1.1.1.1  2001/05/15 13:58:58  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "project.h"
#include "killpopup.h"
#include "errlog.h"

const char killpopup_h_rcs[] = KILLPOPUP_H_VERSION;

#ifdef FEATURE_KILL_POPUPS

/*********************************************************************
 *
 * Function    :  filter_popups
 *
 * Description :  Filter the block of data that's been read from the server.
 *                Caller is responsible for checking permissons list
 *                to determine if this function should be called.
 *                Remember not to change the content length (substitute char by char)
 *
 * Parameters  :
 *          1  :  buff = Buffer to scan and modify.  Null terminated.
 *
 * Returns     :  void
 *
 *********************************************************************/
void filter_popups(char *buff)
{
   char *popup = NULL;
   char *close = NULL;
   char *p     = NULL;

   while ((popup = strstr( buff, "window.open(" )) != NULL)
   {
      if ( popup )
      {
         /*
          * replace the window.open( with a harmless JavaScript replacement (notice the two single quotes)
          * Guy's idea (thanks)
          */
         strncpy(popup, "1;''.concat(", 12);
         log_error(LOG_LEVEL_POPUPS, "Blocked popup window open");
      }
   }
   
   while ((popup = strstr( buff, ".resizeTo(" )) != NULL)
   {
      if ( popup )
      {
         /*
          * replace the .resizeTo( with a harmless JavaScript replacement
          * Guy's idea (thanks)
          */
         strncpy(popup, ".scrollTo(", 10);
         log_error(LOG_LEVEL_POPUPS, "Blocked popup window resize");
      }
   }

   /* Filter onUnload and onExit */
   popup=strstr( buff, "<body");
   if (!popup) popup=strstr( buff, "<BODY");
   if (!popup) popup=strstr( buff, "<Body");
   if (!popup) popup=strstr( buff, "<BOdy");
   if (popup)
   {
      close=strchr(popup,'>');
      if (close)
      {
         /* we are now between <body and the ending > FIXME: No, we're anywhere! --oes */
         p=strstr(popup, "onUnload");
         if (p)
         {
            strncpy(p,"_nU_",4);
         }
         p=strstr(popup, "onExit");
         if (p)
         {
            strncpy(p,"_nE_",4);
         }
      }
   }

}

#endif /* def FEATURE_KILL_POPUPS */

/*
  Local Variables:
  tab-width: 3
  end:
*/
