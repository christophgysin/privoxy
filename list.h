#ifndef _LIST_H
#define _LIST_H
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


extern void enlist(struct list *h, const char *s);
extern void destroy_list(struct list *h);
extern char *list_to_text(struct list *h);

void enlist_unique(struct list *header, const char *str);

int list_remove_item(struct list *header, const char *str);
int list_remove_list(struct list *header, const struct list *to_remove);

void list_duplicate(struct list *dest, const struct list *src);
void list_append_list_unique(struct list *dest, const struct list *src);

void destroy_list_share(struct list_share *h);
void enlist_share(struct list_share *header, const char *str);
void enlist_unique_share(struct list_share *header, const char *str);
int list_remove_item_share(struct list_share *header, const char *str);
int list_remove_list_share(struct list_share *dest, const struct list *src);
void list_duplicate_share(struct list_share *dest, const struct list *src);
void list_append_list_unique_share(struct list_share *dest, const struct list *src);

/* Revision control strings from this header and associated .c file */
extern const char list_rcs[];
extern const char list_h_rcs[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ndef _LIST_H */

/*
  Local Variables:
  tab-width: 3
  end:
*/
