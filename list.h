#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED
#define LIST_H_VERSION "$Id$"
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  Declares functions to handle lists.
 *                Functions declared include:
 *                   `destroy_list', `enlist' and `list_to_text'
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
 *    Revision 1.8  2001/09/16 17:30:24  jongfoster
 *    Fixing a compiler warning.
 *
 *    Revision 1.7  2001/09/16 13:20:29  jongfoster
 *    Rewrite of list library.  Now has seperate header and list_entry
 *    structures.  Also added a large sprinking of assert()s to the list
 *    code.
 *
 *    Revision 1.6  2001/08/05 16:06:20  jongfoster
 *    Modifiying "struct map" so that there are now separate header and
 *    "map_entry" structures.  This means that functions which modify a
 *    map no longer need to return a pointer to the modified map.
 *    Also, it no longer reverses the order of the entries (which may be
 *    important with some advanced template substitutions).
 *
 *    Revision 1.5  2001/07/29 18:43:08  jongfoster
 *    Changing #ifdef _FILENAME_H to FILENAME_H_INCLUDED, to conform to
 *    ANSI C rules.
 *
 *    Revision 1.4  2001/06/29 13:30:37  oes
 *    - Introduced enlist_unique_header()
 *    - Removed logentry from cancelled commit
 *
 *    Revision 1.3  2001/06/03 11:03:48  oes
 *    introduced functions for new list type "map": map(), lookup(),
 *    free_map(), and extended enlist_unique
 *
 *    Revision 1.2  2001/06/01 18:49:17  jongfoster
 *    Replaced "list_share" with "list" - the tiny memory gain was not
 *    worth the extra complexity.
 *
 *    Revision 1.1  2001/05/31 21:11:53  jongfoster
 *    - Moved linked list support to new "list.c" file.
 *      Structure definitions are still in project.h,
 *      function prototypes are now in "list.h".
 *    - Added support for "struct list_share", which is identical
 *      to "struct list" except it saves memory by not duplicating
 *      the strings.  Obviously, this only works if there is some
 *      other way of managing the memory used by the strings.
 *      (These list_share lists are used for lists which last
 *      for only 1 request, and where all the list entries are
 *      just coming directly from entries in the actionsfile.)
 *      Note that you still need to destroy list_share lists
 *      properly to free the nodes - it's only the strings
 *      which are shared.
 *
 *
 *********************************************************************/


#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * struct list
 *
 * A linked list class.
 */

extern void init_list    (struct list *the_list);
extern void destroy_list (struct list *the_list);

extern int  enlist                 (struct list *the_list, const char *str);
extern int  enlist_unique          (struct list *the_list, const char *str, int num_significant_chars);
extern int  enlist_unique_header   (struct list *the_list, const char *name, const char *value);
extern int  enlist_first           (struct list *the_list, const char *str);
extern int  list_append_list_unique(struct list *dest,     const struct list *src);
extern int  list_duplicate         (struct list *dest,     const struct list *src);

extern int  list_remove_item(struct list *the_list, const char *str);
extern int  list_remove_list(struct list *dest,     const struct list *src);
extern void list_remove_all (struct list *the_list);

extern int  list_is_empty(const struct list *the_list);

extern char * list_to_text(const struct list *the_list);


/*
 * struct map
 *
 * A class which maps names to values.
 *
 * Note: You must allocate this through new_map() and free it
 * through free_map().
 */

extern struct map * new_map  (void);
extern void         free_map (struct map * the_map);

extern int          map      (struct map * the_map,
                              const char * name, int name_needs_copying,
                              const char * value, int value_needs_copying);
extern const char * lookup   (const struct map * the_map, const char * name);


/* Revision control strings from this header and associated .c file */
extern const char list_rcs[];
extern const char list_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef LIST_H_INCLUDED */

/*
  Local Variables:
  tab-width: 3
  end:
*/
