#ifndef JCC_H_INCLUDED
#define JCC_H_INCLUDED
#define JCC_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Main file.  Contains main() method, main loop, and 
 *                the main connection-handling function.
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
 *    Revision 1.5  2001/07/29 19:32:00  jongfoster
 *    Renaming _main() [mingw32 only] to real_main(), for ANSI compliance.
 *
 *    Revision 1.4  2001/07/29 18:58:15  jongfoster
 *    Removing nested #includes, adding forward declarations for needed
 *    structures, and changing the #define _FILENAME_H to FILENAME_H_INCLUDED.
 *
 *    Revision 1.3  2001/07/18 12:31:58  oes
 *    moved #define freez from jcc.h to project.h
 *
 *    Revision 1.2  2001/05/31 21:24:47  jongfoster
 *    Changed "permission" to "action" throughout.
 *    Removed DEFAULT_USER_AGENT - it must now be specified manually.
 *    Moved vanilla wafer check into chat(), since we must now
 *    decide whether or not to add it based on the URL.
 *
 *    Revision 1.1.1.1  2001/05/15 13:58:56  oes
 *    Initial import of version 2.9.3 source tree
 *
 *
 *********************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

struct client_state;
struct file_list;


/* Global variables */

#ifdef STATISTICS
extern int urls_read;
extern int urls_rejected;
#endif /*def STATISTICS*/

extern struct client_state clients[];

extern struct file_list    files[];


/* Functions */

#ifdef __MINGW32__
int real_main(int argc, const char *argv[]);
#else
int main(int argc, const char *argv[]);
#endif

/* Revision control strings from this header and associated .c file */
extern const char jcc_rcs[];
extern const char jcc_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef JCC_H_INCLUDED */

/*
  Local Variables:
  tab-width: 3
  end:
*/
