#ifndef _W32RES_H
#define _W32RES_H
#define W32RES_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Identifiers for Windows GUI resources.
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
 *    Revision 1.3  2001/05/26 00:28:36  jongfoster
 *    Automatic reloading of config file.
 *    Removed obsolete SIGHUP support (Unix) and Reload menu option (Win32).
 *    Most of the global variables have been moved to a new
 *    struct configuration_spec, accessed through csp->config->globalname
 *    Most of the globals remaining are used by the Win32 GUI.
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
 *    Revision 1.1.1.1  2001/05/15 13:59:08  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/

#define IDS_NEW_BLOCKER                   1

#define ID_NEW_BLOCKER                    100
#define IDR_TRAYMENU                      101
#define IDI_IDLE                          102
#define IDR_LOGVIEW                       103
#define IDR_ACCELERATOR                   104
#define IDR_POPUP_SELECTION               105
#define IDD_RULES                         106
#define IDI_DENYRULE                      107
#define IDI_ALLOWRULE                     108

#define IDI_JUNKBUSTER                    200
#define IDI_JUNKBUSTER1                   201
#define IDI_JUNKBUSTER2                   202
#define IDI_JUNKBUSTER3                   203
#define IDI_JUNKBUSTER4                   204
#define IDI_JUNKBUSTER5                   205
#define IDI_JUNKBUSTER6                   206
#define IDI_JUNKBUSTER7                   207
#define IDI_JUNKBUSTER8                   208

#define IDC_NEW                           300
#define IDC_ACTION                        301
#define IDC_RULES                         302
#define IDC_CREATE                        303
#define IDC_MOVEUP                        304
#define IDC_MOVEDOWN                      305
#define IDC_DELETE                        306
#define IDC_SAVE                          307

#define ID_SHOWWINDOW                     4000
#define ID_HELP_ABOUTJUNKBUSTER           4001
#define ID_FILE_EXIT                      4002
#define ID_VIEW_CLEARLOG                  4003
#define ID_VIEW_LOGMESSAGES               4004
#define ID_VIEW_MESSAGEHIGHLIGHTING       4005
#define ID_VIEW_LIMITBUFFERSIZE           4006
#define ID_VIEW_ACTIVITYANIMATION         4007
#define ID_HELP_FAQ                       4008
#define ID_HELP_MANUAL                    4009
#define ID_HELP_GPL                       4010
#define ID_HELP_STATUS                    4011
#ifdef TOGGLE
#define ID_TOGGLE_IJB                     4012
#endif

/* Break these out so they are easier to extend, but keep consecutive */
#define ID_TOOLS_EDITJUNKBUSTER           5000
#define ID_TOOLS_EDITBLOCKERS             5001
#define ID_TOOLS_EDITPERMISSIONS          5002
#define ID_TOOLS_EDITFORWARD              5003

#ifdef ACL_FILES
#define ID_TOOLS_EDITACLS                 5005
#endif /* def ACL_FILES */

#ifdef USE_IMAGE_LIST
#define ID_TOOLS_EDITIMAGE                5006
#endif /* def USE_IMAGE_LIST */

#ifdef PCRS
#define ID_TOOLS_EDITPERLRE               5008
#endif /* def PCRS */

#ifdef TRUST_FILES
#define ID_TOOLS_EDITTRUST                5004
#endif /* def TRUST_FILES */

/*
 * The following symbols are declared in <afxres.h> in VC++.
 * However, mingw32 doesn't have that header.  Let's 
 * always declare them here, for consistency.
 * These are the VC++ values.
 */
#define IDC_STATIC      (-1)
#define ID_EDIT_COPY  30000


#endif /* ndef _W32RES_H */

/*
  Local Variables:
  tab-width: 3
  end:
*/
