#ifndef _LOADERS_H
#define _LOADERS_H
#define LOADERS_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Functions to load and unload the various
 *                configuration files.  Also contains code to manage
 *                the list of active loaders, and to automatically 
 *                unload files that are no longer in use.
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
 *    Revision 1.1.1.1  2001/05/15 13:59:00  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void sweep(void);
extern char *read_config_line(char *buf, int buflen, FILE *fp, struct file_list *fs);
extern int check_file_changed(const struct file_list * current,
                              const char * filename,
                              struct file_list ** newfl);

extern int load_blockfile(struct client_state *csp);
extern int load_permissions_file(struct client_state *csp);
extern int load_forwardfile(struct client_state *csp);
  
#ifdef ACL_FILES
extern int load_aclfile(struct client_state *csp);
#endif /* def ACL_FILES */

#ifdef USE_IMAGE_LIST
extern int load_imagefile(struct client_state *csp);
#endif /* def USE_IMAGE_LIST */
 
#ifdef TRUST_FILES
extern int load_trustfile(struct client_state *csp);
#endif /* def TRUST_FILES */

#ifdef PCRS
extern int load_re_filterfile(struct client_state *csp);
#endif /* def PCRS */

extern void add_loader(int (*loader)(struct client_state *), 
                       struct configuration_spec * config);
extern int run_loader(struct client_state *csp);

#ifdef PCRS
extern int load_re_filterfile(struct client_state *csp);
#endif /* def PCRS */

/* Revision control strings from this header and associated .c file */
extern const char loaders_rcs[];
extern const char loaders_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef _LOADERS_H */

/*
  Local Variables:
  tab-width: 3
  end:
*/
