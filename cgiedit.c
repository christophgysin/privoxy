const char cgiedit_rcs[] = "$Id$";
/*********************************************************************
 *
 * File        :  $Source$
 *
 * Purpose     :  CGI-based actionsfile editor.
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
 *    Revision 1.4  2001/10/23 21:48:19  jongfoster
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
 *    Revision 1.3  2001/10/14 22:12:49  jongfoster
 *    New version of CGI-based actionsfile editor.
 *    Major changes, including:
 *    - Completely new file parser and file output routines
 *    - edit-actions CGI renamed edit-actions-for-url
 *    - All CGIs now need a filename parameter, except for...
 *    - New CGI edit-actions which doesn't need a filename,
 *      to allow you to start the editor up.
 *    - edit-actions-submit now works, and now automatically
 *      redirects you back to the main edit-actions-list handler.
 *
 *    Revision 1.2  2001/09/16 17:05:14  jongfoster
 *    Removing unused #include showarg.h
 *
 *    Revision 1.1  2001/09/16 15:47:37  jongfoster
 *    First version of CGI-based edit interface.  This is very much a
 *    work-in-progress, and you can't actually use it to edit anything
 *    yet.  You must #define FEATURE_CGI_EDIT_ACTIONS for these changes
 *    to have any effect.
 *
 *
 **********************************************************************/


#include "config.h"

/*
 * FIXME: Following includes copied from cgi.c - which are actually needed?
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <sys/stat.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif /* def _WIN32 */

#include "project.h"
#include "cgi.h"
#include "cgiedit.h"
#include "cgisimple.h"
#include "list.h"
#include "encode.h"
#include "actions.h"
#include "miscutil.h"
#include "errlog.h"
#include "loadcfg.h"
/* loadcfg.h is for g_bToggleIJB only */

const char cgiedit_h_rcs[] = CGIEDIT_H_VERSION;


#ifdef FEATURE_CGI_EDIT_ACTIONS

struct file_line
{
   struct file_line * next;
   char * raw;
   char * prefix;
   char * unprocessed;
   int type;
   
   union
   {
      struct action_spec action[1];

      struct
      {
         char * name;
         char * svalue;
         int ivalue;
      } setting;

      /* Add more data types here... e.g.


      struct url_spec url[1];

      struct
      {
         struct action_spec action[1];
         const char * name;
      } alias;

      */

   } data;
};

#define FILE_LINE_UNPROCESSED           1
#define FILE_LINE_BLANK                 2
#define FILE_LINE_ALIAS_HEADER          3
#define FILE_LINE_ALIAS_ENTRY           4
#define FILE_LINE_ACTION                5
#define FILE_LINE_URL                   6
#define FILE_LINE_SETTINGS_HEADER       7
#define FILE_LINE_SETTINGS_ENTRY        8
#define FILE_LINE_DESCRIPTION_HEADER    9
#define FILE_LINE_DESCRIPTION_ENTRY    10


struct editable_file
{
   struct file_line * lines;
   const char * filename;     /* Full pathname - e.g. "/etc/junkbuster/wibble.action" */
   const char * identifier;   /* Filename stub - e.g. "wibble".  Use for CGI param. */
   const char * version_str;  /* Last modification time, as a string.  For CGI param */
   unsigned version;          /* Last modification time - prevents chaos with
                               * the browser's "back" button.  Note that this is a
                               * time_t cast to an unsigned.  When comparing, always
                               * cast the time_t to an unsigned, and *NOT* vice-versa.
                               * This may lose the top few bits, but they're not
                               * significant anyway.
                               */
   struct file_line * parse_error; /* On parse error, this is the offending line. */
   const char * parse_error_text;  /* On parse error, this is the problem.  
                                    * (Statically allocated) */
};

/* FIXME: Following non-static functions should be prototyped in .h or made static */

/* Functions to read and write arbitrary config files */
jb_err edit_read_file(struct client_state *csp,
                      const struct map *parameters,
                      int require_version,
                      const char *suffix,
                      struct editable_file **pfile);
jb_err edit_write_file(struct editable_file * file);
void   edit_free_file(struct editable_file * file);

/* Functions to read and write actions files */
jb_err edit_parse_actions_file(struct editable_file * file);
jb_err edit_read_actions_file(struct client_state *csp,
                              struct http_response *rsp,
                              const struct map *parameters,
                              int require_version,
                              struct editable_file **pfile);

/* Error handlers */
jb_err cgi_error_modified(struct client_state *csp,
                          struct http_response *rsp,
                          const char *filename);
jb_err cgi_error_parse(struct client_state *csp,
                       struct http_response *rsp,
                       struct editable_file *file);
jb_err cgi_error_file(struct client_state *csp,
                      struct http_response *rsp,
                      const char *filename);

/* Internal arbitrary config file support functions */
static jb_err edit_read_file_lines(FILE *fp, struct file_line ** pfile);
static void edit_free_file_lines(struct file_line * first_line);
static jb_err simple_read_line(char **dest, FILE *fp);
static jb_err edit_read_line(FILE *fp, char **raw_out, char **prefix_out, char **data_out);

/* Internal actions file support functions */
static int match_actions_file_header_line(const char * line, const char * name);
static jb_err split_line_on_equals(const char * line, char ** pname, char ** pvalue);

/* Internal parameter parsing functions */
static jb_err get_file_name_param(struct client_state *csp,
                                  const struct map *parameters,
                                  const char *param_name,
                                  const char *suffix,
                                  char **pfilename,
                                  const char **pparam);
static jb_err get_number_param(struct client_state *csp,
                               const struct map *parameters,
                               char *name,
                               unsigned *pvalue);

/* Internal actionsfile <==> HTML conversion functions */
static jb_err map_radio(struct map * exports,
                        const char * optionname, 
                        const char * values,
                        char value);
static jb_err actions_to_radio(struct map * exports,
                               const struct action_spec *action);
static jb_err actions_from_radio(const struct map * parameters,
                                 struct action_spec *action);


/*********************************************************************
 *
 * Function    :  simple_read_line
 *
 * Description :  Read a single line from a file and return it.
 *                This is basically a version of fgets() that malloc()s
 *                it's own line buffer.  Note that the buffer will
 *                always be a multiple of BUFFER_SIZE bytes long.
 *                Therefore if you are going to keep the string for
 *                an extended period of time, you should probably
 *                strdup() it and free() the original, to save memory.
 *
 *
 * Parameters  :
 *          1  :  dest = destination for newly malloc'd pointer to
 *                line data.  Will be set to NULL on error.
 *          2  :  fp = File to read from
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_FILE   on EOF.
 *
 *********************************************************************/
static jb_err simple_read_line(char **dest, FILE *fp)
{
   int len;
   char * buf;
   char * newbuf;

   assert(fp);
   assert(dest);

   *dest = NULL;

   if (NULL == (buf = malloc(BUFFER_SIZE)))
   {
      return JB_ERR_MEMORY;
   }
   
   *buf = '\0';
   len = 0;

   while (FOREVER)
   {
      newbuf = buf + len;
      if ((!fgets(newbuf, BUFFER_SIZE, fp)) || (*newbuf == '\0'))
      {
         /* (*newbuf == '\0') should never happen unless fgets fails */
         if (*buf == '\0')
         {
            free(buf);
            return JB_ERR_FILE;
         }
         else
         {
            *dest = buf;
            return JB_ERR_OK;
         }
      }
      len = strlen(buf);
      if ((buf[len - 1] == '\n') || (buf[len - 1] == '\r'))
      {
         *dest = buf;
         return JB_ERR_OK;
      }
      
      if (NULL == (newbuf = realloc(buf, len + BUFFER_SIZE)))
      {
         free(buf);
         return JB_ERR_MEMORY;
      }
      buf = newbuf;
   }
}


/*********************************************************************
 *
 * Function    :  edit_read_line
 *
 * Description :  Read a single non-empty line from a file and return
 *                it.  Trims comments, leading and trailing whitespace
 *                and respects escaping of newline and comment char.
 *                Provides the line in 2 alternative forms: raw and
 *                preprocessed.
 *                - raw is the raw data read from the file.  If the 
 *                  line is not modified, then this should be written
 *                  to the new file.
 *                - prefix is any comments and blank lines that were
 *                  read from the file.  If the line is modified, then
 *                  this should be written out to the file followed
 *                  by the modified data.  (If this string is non-empty
 *                  then it will have a newline at the end).
 *                - data is the actual data that will be parsed
 *                  further by appropriate routines.
 *                On EOF, the 3 strings will all be set to NULL and
 *                0 will be returned.
 *
 * Parameters  :
 *          1  :  fp = File to read from
 *          2  :  raw_out = destination for newly malloc'd pointer to
 *                raw line data.  May be NULL if you don't want it.
 *          3  :  prefix_out = destination for newly malloc'd pointer to
 *                comments.  May be NULL if you don't want it.
 *          4  :  data_out = destination for newly malloc'd pointer to
 *                line data with comments and leading/trailing spaces
 *                removed, and line continuation performed.  May be
 *                NULL if you don't want it.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_FILE   on EOF.
 *
 *********************************************************************/
static jb_err edit_read_line(FILE *fp, char **raw_out, char **prefix_out, char **data_out)
{
   char *p;          /* Temporary pointer   */
   char *linebuf;    /* Line read from file */
   char *linestart;  /* Start of linebuf, usually first non-whitespace char */
   char newline[3];  /* Used to store the newline - "\n", "\r", or "\r\n"   */
   int contflag = 0; /* Nonzero for line continuation - i.e. line ends '\'  */
   char *raw;        /* String to be stored in raw_out    */
   char *prefix;     /* String to be stored in prefix_out */
   char *data;       /* String to be stored in data_out   */
   jb_err rval = JB_ERR_OK;

   assert(fp);

   /* Set output parameters to NULL */
   if (raw_out)
   {
      *raw_out    = NULL;
   }
   if (prefix_out)
   {
      *prefix_out = NULL;
   }
   if (data_out)
   {
      *data_out   = NULL;
   }

   /* Set string variables to new, empty strings. */

   raw    = malloc(1);
   prefix = malloc(1);
   data   = malloc(1);

   if ((raw == NULL) || (prefix == NULL) || (data == NULL))
   {
      freez(raw);
      freez(prefix);
      freez(data);
      return JB_ERR_MEMORY;
   }

   *raw    = '\0';
   *prefix = '\0';
   *data   = '\0';

   /* Main loop.  Loop while we need more data & it's not EOF. */

   while ( (contflag || (*data == '\0'))
        && (JB_ERR_OK == (rval = simple_read_line(&linebuf, fp))))
   {
      if (string_append(&raw,linebuf))
      {
         free(prefix);
         free(data);
         free(linebuf);
         return JB_ERR_MEMORY;
      }
      
      /* Trim off newline */
      p = linebuf + strlen(linebuf);
      if ((p != linebuf) && ((p[-1] == '\r') || (p[-1] == '\n')))
      {
         p--;
         if ((p != linebuf) && ((p[-1] == '\r') || (p[-1] == '\n')))
         {
            p--;
         }
      }
      strcpy(newline, p);
      *p = '\0';

      /* Line continuation? Trim escape and set flag. */
      contflag = ((p != linebuf) && (*--p == '\\'));
      if (contflag)
      {
         *p = '\0';
      }

      /* Trim leading spaces if we're at the start of the line */
      linestart = linebuf;
      if (*data == '\0')
      {
         /* Trim leading spaces */
         while (*linestart && isspace((int)(unsigned char)*linestart))
         {
            linestart++;
         }
      }

      /* Handle comment characters. */
      p = linestart;
      while ((p = strchr(p, '#')) != NULL)
      {
         /* Found a comment char.. */
         if ((p != linebuf) && (*(p-1) == '\\'))
         {
            /* ..and it's escaped, left-shift the line over the escape. */
            char *q = p - 1;
            while ((*q = *(q + 1)) != '\0')
            {
               q++;
            }
            /* Now scan from just after the "#". */
         }
         else
         {
            /* Real comment.  Save it... */
            if (p == linestart)
            {
               /* Special case:  Line only contains a comment, so all the
                * previous whitespace is considered part of the comment.
                * Undo the whitespace skipping, if any.
                */
               linestart = linebuf;
               p = linestart;
            }
            string_append(&prefix,p);
            if (string_append(&prefix,newline))
            {
               free(raw);
               free(data);
               free(linebuf);
               return JB_ERR_MEMORY;
            }
            *newline = '\0';

            /* ... and chop off the rest of the line */
            *p = '\0';
         }
      } /* END while (there's a # character) */

      /* Write to the buffer */
      if (*linestart)
      {
         if (string_append(&data, linestart))
         {
            free(raw);
            free(prefix);
            free(linebuf);
            return JB_ERR_MEMORY;
         }
      }

      free(linebuf);
   } /* END while(we need more data) */

   /* Handle simple_read_line() errors - ignore EOF */
   if ((rval != JB_ERR_OK) && (rval != JB_ERR_FILE))
   {
      free(raw);
      free(prefix);
      free(data);
      return rval;
   }


   if (*raw)
   {
      /* Got at least some data */

      /* Remove trailing whitespace */         
      chomp(data);

      if (raw_out)
      {
         *raw_out    = raw;
      }
      else
      {
         free(raw);
      }
      if (prefix_out)
      {
         *prefix_out = prefix;
      }
      else
      {
         free(prefix);
      }
      if (data_out)
      {
         *data_out   = data;
      }
      else
      {
         free(data);
      }
      return JB_ERR_OK;
   }
   else
   {
      /* EOF and no data there. */

      free(raw);
      free(prefix);
      free(data);

      return JB_ERR_FILE;
   }
}


/*********************************************************************
 *
 * Function    :  edit_write_file
 *
 * Description :  Write a complete file to disk.
 *
 * Parameters  :
 *          1  :  filename = File to write to.
 *          2  :  file = Data structure to write.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_FILE   on error writing to file.
 *                JB_ERR_MEMORY on out of memory
 *
 *********************************************************************/
jb_err edit_write_file(struct editable_file * file)
{
   FILE * fp;
   struct file_line * cur_line;
   struct stat statbuf[1];
   char version_buf[22]; /* 22 = ceil(log10(2^64)) + 2 = max number of
                            digits in time_t, assuming this is a 64-bit
                            machine, plus null terminator, plus one
                            for paranoia */

   assert(file);
   assert(file->filename);

   if (NULL == (fp = fopen(file->filename, "wt")))
   {
      return JB_ERR_FILE;
   }

   cur_line = file->lines;
   while (cur_line != NULL)
   {
      if (cur_line->raw)
      {
         if (fputs(cur_line->raw, fp) < 0)
         {
            fclose(fp);
            return JB_ERR_FILE;
         }
      }
      else
      {
         if (cur_line->prefix)
         {
            if (fputs(cur_line->prefix, fp) < 0)
            {
               fclose(fp);
               return JB_ERR_FILE;
            }
         }
         if (cur_line->unprocessed)
         {
            if (fputs(cur_line->unprocessed, fp) < 0)
            {
               fclose(fp);
               return JB_ERR_FILE;
            }
            if (fputs("\n", fp) < 0)
            {
               fclose(fp);
               return JB_ERR_FILE;
            }
         }
         else
         {
            /* FIXME: Write data from file->data->whatever */
            assert(0);
         }
      }
      cur_line = cur_line->next;
   }

   fclose(fp);


   /* Update the version stamp in the file structure, since we just
    * wrote to the file & changed it's date.
    */
   if (stat(file->filename, statbuf) < 0)
   {
      /* Error, probably file not found. */
      return JB_ERR_FILE;
   }
   file->version = (unsigned)statbuf->st_mtime;

   /* Correct file->version_str */
   freez((char *)file->version_str);
   snprintf(version_buf, 22, "%u", file->version);
   version_buf[21] = '\0';
   file->version_str = strdup(version_buf);
   if (version_buf == NULL)
   {
      return JB_ERR_MEMORY;
   }

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  edit_free_file
 *
 * Description :  Free a complete file in memory.  
 *
 * Parameters  :
 *          1  :  file = Data structure to free.
 *
 * Returns     :  N/A
 *
 *********************************************************************/
void edit_free_file(struct editable_file * file)
{
   if (!file)
   {
      /* Silently ignore NULL pointer */
      return;
   }

   edit_free_file_lines(file->lines);
   freez((char *)file->filename);
   freez((char *)file->identifier);
   freez((char *)file->version_str);
   file->version = 0;
   file->parse_error_text = NULL; /* Statically allocated */
   file->parse_error = NULL;

   free(file);
}


/*********************************************************************
 *
 * Function    :  edit_free_file
 *
 * Description :  Free an entire linked list of file lines.  
 *
 * Parameters  :
 *          1  :  first_line = Data structure to free.
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void edit_free_file_lines(struct file_line * first_line)
{
   struct file_line * next_line;

   while (first_line != NULL)
   {
      next_line = first_line->next;
      first_line->next = NULL;
      freez(first_line->raw);
      freez(first_line->prefix);
      freez(first_line->unprocessed);
      switch(first_line->type)
      {
         case 0: /* special case if memory zeroed */
         case FILE_LINE_UNPROCESSED:
         case FILE_LINE_BLANK:
         case FILE_LINE_ALIAS_HEADER:
         case FILE_LINE_SETTINGS_HEADER:
         case FILE_LINE_DESCRIPTION_HEADER:
         case FILE_LINE_DESCRIPTION_ENTRY:
         case FILE_LINE_ALIAS_ENTRY:
         case FILE_LINE_URL:
            /* No data is stored for these */
            break;

         case FILE_LINE_ACTION:
            free_action(first_line->data.action);
            break;

         case FILE_LINE_SETTINGS_ENTRY:
            freez(first_line->data.setting.name);
            freez(first_line->data.setting.svalue);
            break;
         default:
            /* Should never happen */
            assert(0);
            break;
      }
      first_line->type = 0; /* paranoia */
      free(first_line);
      first_line = next_line;
   }
}


/*********************************************************************
 *
 * Function    :  match_actions_file_header_line
 *
 * Description :  Match an actions file {{header}} line 
 *
 * Parameters  :
 *          1  :  line - String from file
 *          2  :  name - Header to match against
 *
 * Returns     :  0 iff they match.
 *
 *********************************************************************/
static int match_actions_file_header_line(const char * line, const char * name)
{
   int len;

   assert(line);
   assert(name);

   /* Look for "{{" */
   if ((line[0] != '{') || (line[1] != '{'))
   {
      return 1;
   }
   line += 2;

   /* Look for optional whitespace */
   while ( (*line == ' ') || (*line == '\t') )
   {
      line++;
   }

   /* Look for the specified name (case-insensitive) */
   len = strlen(name);
   if (0 != strncmpic(line, name, len))
   {
      return 1;
   }
   line += len;

   /* Look for optional whitespace */
   while ( (*line == ' ') || (*line == '\t') )
   {
      line++;
   }

   /* Look for "}}" and end of string*/
   if ((line[0] != '}') || (line[1] != '}') || (line[2] != '\0'))
   {
      return 1;
   }

   /* It matched!! */
   return 0;
}


/*********************************************************************
 *
 * Function    :  match_actions_file_header_line
 *
 * Description :  Match an actions file {{header}} line 
 *
 * Parameters  :
 *          1  :  line - String from file.  Must not start with
 *                       whitespace (else infinite loop!)
 *          2  :  name - Destination for name
 *          2  :  name - Destination for value
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_PARSE  if there's no "=" sign, or if there's
 *                              nothing before the "=" sign (but empty
 *                              values *after* the "=" sign are legal).
 *
 *********************************************************************/
static jb_err split_line_on_equals(const char * line, char ** pname, char ** pvalue)
{
   const char * name_end;
   const char * value_start;
   int name_len;

   assert(line);
   assert(pname);
   assert(pvalue);
   assert(*line != ' ');
   assert(*line != '\t');

   *pname = NULL;
   *pvalue = NULL;

   value_start = strchr(line, '=');
   if ((value_start == NULL) || (value_start == line))
   {
      return JB_ERR_PARSE;
   }

   name_end = value_start - 1;

   /* Eat any whitespace before the '=' */
   while ((*name_end == ' ') || (*name_end == '\t'))
   {
      /*
       * we already know we must have at least 1 non-ws char
       * at start of buf - no need to check
       */
      name_end--;
   }

   name_len = name_end - line + 1; /* Length excluding \0 */
   if (NULL == (*pname = (char *) malloc(name_len + 1)))
   {
      return JB_ERR_MEMORY;
   }
   strncpy(*pname, line, name_len);
   (*pname)[name_len] = '\0';

   /* Eat any the whitespace after the '=' */
   value_start++;
   while ((*value_start == ' ') || (*value_start == '\t'))
   {
      value_start++;
   }

   if (NULL == (*pvalue = strdup(value_start)))
   {
      free(*pname);
      *pname = NULL;
      return JB_ERR_MEMORY;
   }

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  edit_parse_actions_file
 *
 * Description :  Parse an actions file in memory.  
 *
 *                Passed linked list must have the "data" member
 *                zeroed, and must contain valid "next" and
 *                "unprocessed" fields.  The "raw" and "prefix"
 *                fields are ignored, and "type" is just overwritten.
 *
 *                Note that on error the file may have been
 *                partially parsed.
 *
 * Parameters  :
 *          1  :  file = Actions file to be parsed in-place.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_PARSE  on error
 *
 *********************************************************************/
jb_err edit_parse_actions_file(struct editable_file * file)
{
   struct file_line * cur_line;
   int len;
   const char * text; /* Text from a line */
   char * name;  /* For lines of the form name=value */
   char * value; /* For lines of the form name=value */
   struct action_alias * alias_list = NULL;
   jb_err err = JB_ERR_OK;

   /* alias_list contains the aliases defined in this file.
    * It might be better to use the "file_line.data" fields
    * in the relavent places instead.
    */

   cur_line = file->lines;

   /* A note about blank line support: Blank lines should only 
    * ever occur as the last line in the file.  This function
    * is more forgiving than that - FILE_LINE_BLANK can occur
    * anywhere.
    */

   /* Skip leading blanks.  Should only happen if file is
    * empty (which is valid, but pointless).
    */
   while ( (cur_line != NULL)
        && (cur_line->unprocessed[0] == '\0') )
   {
      /* Blank line */
      cur_line->type = FILE_LINE_BLANK;
      cur_line = cur_line->next;
   }

   if ( (cur_line != NULL)
     && (cur_line->unprocessed[0] != '{') )
   {
      /* File doesn't start with a header */
      file->parse_error = cur_line;
      file->parse_error_text = "First (non-comment) line of the file must contain a header.";
      return JB_ERR_PARSE;
   }

   if ( (cur_line != NULL) && (0 ==
      match_actions_file_header_line(cur_line->unprocessed, "settings") ) )
   {
      cur_line->type = FILE_LINE_SETTINGS_HEADER;

      cur_line = cur_line->next;
      while ((cur_line != NULL) && (cur_line->unprocessed[0] != '{'))
      {
         if (cur_line->unprocessed[0])
         {
            cur_line->type = FILE_LINE_SETTINGS_ENTRY;

            err = split_line_on_equals(cur_line->unprocessed,
                     &cur_line->data.setting.name,
                     &cur_line->data.setting.svalue);
            if (err == JB_ERR_MEMORY)
            {
               return err;
            }
            else if (err != JB_ERR_OK)
            {
               /* Line does not contain a name=value pair */
               file->parse_error = cur_line;
               file->parse_error_text = "Expected a name=value pair on this {{description}} line, but couldn't find one.";
               return JB_ERR_PARSE;
            }
         }
         else
         {
            cur_line->type = FILE_LINE_BLANK;
         }
         cur_line = cur_line->next;
      }
   }

   if ( (cur_line != NULL) && (0 ==
      match_actions_file_header_line(cur_line->unprocessed, "description") ) )
   {
      cur_line->type = FILE_LINE_DESCRIPTION_HEADER;

      cur_line = cur_line->next;
      while ((cur_line != NULL) && (cur_line->unprocessed[0] != '{'))
      {
         if (cur_line->unprocessed[0])
         {
            cur_line->type = FILE_LINE_DESCRIPTION_ENTRY;
         }
         else
         {
            cur_line->type = FILE_LINE_BLANK;
         }
         cur_line = cur_line->next;
      }
   }

   if ( (cur_line != NULL) && (0 ==
      match_actions_file_header_line(cur_line->unprocessed, "alias") ) )
   {
      cur_line->type = FILE_LINE_ALIAS_HEADER;

      cur_line = cur_line->next;
      while ((cur_line != NULL) && (cur_line->unprocessed[0] != '{'))
      {
         if (cur_line->unprocessed[0])
         {
            /* define an alias */
            struct action_alias * new_alias;

            cur_line->type = FILE_LINE_ALIAS_ENTRY;

            err = split_line_on_equals(cur_line->unprocessed, &name, &value);
            if (err == JB_ERR_MEMORY)
            {
               return err;
            }
            else if (err != JB_ERR_OK)
            {
               /* Line does not contain a name=value pair */
               file->parse_error = cur_line;
               file->parse_error_text = "Expected a name=value pair on this {{alias}} line, but couldn't find one.";
               return JB_ERR_PARSE;
            }

            if ((new_alias = zalloc(sizeof(*new_alias))) == NULL)
            {
               /* Out of memory */
               free(name);
               free(value);
               free_alias_list(alias_list);
               return JB_ERR_MEMORY;
            }

            err = get_actions(value, alias_list, new_alias->action);
            if (err)
            {
               /* Invalid action or out of memory */
               free(name);
               free(value);
               free(new_alias);
               free_alias_list(alias_list);
               if (err == JB_ERR_MEMORY)
               {
                  return err;
               }
               else
               {
                  /* Line does not contain a name=value pair */
                  file->parse_error = cur_line;
                  file->parse_error_text = "This alias does not specify a valid set of actions.";
                  return JB_ERR_PARSE;
               }
            }

            free(value);

            new_alias->name = name;

            /* add to list */
            new_alias->next = alias_list;
            alias_list = new_alias;
         }
         else
         {
            cur_line->type = FILE_LINE_BLANK;
         }
         cur_line = cur_line->next;
      }
   }

   /* Header done, process the main part of the file */
   while (cur_line != NULL)
   {
      /* At this point, (cur_line->unprocessed[0] == '{') */
      assert(cur_line->unprocessed[0] == '{');
      text = cur_line->unprocessed + 1;
      len = strlen(text) - 1;
      if (text[len] != '}')
      {
         /* No closing } on header */
         free_alias_list(alias_list);
         file->parse_error = cur_line;
         file->parse_error_text = "Headers starting with '{' must have a "
            "closing bracket ('}').  Headers starting with two brackets ('{{') "
            "must close with two brackets ('}}').";
         return JB_ERR_PARSE;
      }

      if (text[0] == '{')
      {
         /* An invalid {{ header.  */
         free_alias_list(alias_list);
         file->parse_error = cur_line;
         file->parse_error_text = "Unknown or unexpected two-bracket header.  "
            "Please remember that the system (two-bracket) headers must "
            "appear in the order {{settings}}, {{description}}, {{alias}}, "
            "and must appear before any actions (one-bracket) headers.  "
            "Also note that system headers may not be repeated.";
         return JB_ERR_PARSE;
      }

      while ( (*text == ' ') || (*text == '\t') )
      {
         text++;
         len--;
      }
      while ( (len > 0)
           && ( (text[len - 1] == ' ')
             || (text[len - 1] == '\t') ) )
      {
         len--;
      }

      cur_line->type = FILE_LINE_ACTION;

      /* Remove {} and make copy */
      if (NULL == (value = (char *) malloc(len + 1)))
      {
         /* Out of memory */
         free_alias_list(alias_list);
         return JB_ERR_MEMORY;
      }
      strncpy(value, text, len);
      value[len] = '\0';

      /* Get actions */
      err = get_actions(value, alias_list, cur_line->data.action);
      if (err)
      {
         /* Invalid action or out of memory */
         free(value);
         free_alias_list(alias_list);
         if (err == JB_ERR_MEMORY)
         {
            return err;
         }
         else
         {
            /* Line does not contain a name=value pair */
            file->parse_error = cur_line;
            file->parse_error_text = "This header does not specify a valid set of actions.";
            return JB_ERR_PARSE;
         }
      }

      /* Done with string - it was clobbered anyway */
      free(value);

      /* Process next line */
      cur_line = cur_line->next;

      /* Loop processing URL patterns */
      while ((cur_line != NULL) && (cur_line->unprocessed[0] != '{'))
      {
         if (cur_line->unprocessed[0])
         {
            /* Could parse URL here, but this isn't currently needed */

            cur_line->type = FILE_LINE_URL;
         }
         else
         {
            cur_line->type = FILE_LINE_BLANK;
         }
         cur_line = cur_line->next;
      }
   } /* End main while(cur_line != NULL) loop */

   free_alias_list(alias_list);

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  edit_read_file_lines
 *
 * Description :  Read all the lines of a file into memory.  
 *                Handles whitespace, comments and line continuation.
 *
 * Parameters  :
 *          1  :  fp = File to read from.  On return, this will be
 *                     at EOF but it will not have been closed.
 *          2  :  pfile = Destination for a linked list of file_lines.
 *                        Will be set to NULL on error.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *
 *********************************************************************/
jb_err edit_read_file_lines(FILE *fp, struct file_line ** pfile)
{
   struct file_line * first_line; /* Keep for return value or to free */
   struct file_line * cur_line;   /* Current line */
   struct file_line * prev_line;  /* Entry with prev_line->next = cur_line */
   jb_err rval;

   assert(fp);
   assert(pfile);

   *pfile = NULL;

   cur_line = first_line = zalloc(sizeof(struct file_line));
   if (cur_line == NULL)
   {
      return JB_ERR_MEMORY;
   }

   cur_line->type = FILE_LINE_UNPROCESSED;

   rval = edit_read_line(fp, &cur_line->raw, &cur_line->prefix, &cur_line->unprocessed);
   if (rval)
   {
      /* Out of memory or empty file. */
      /* Note that empty file is not an error we propogate up */
      free(cur_line);
      return ((rval == JB_ERR_FILE) ? JB_ERR_OK : rval);
   }

   do
   {
      prev_line = cur_line;
      cur_line = prev_line->next = zalloc(sizeof(struct file_line));
      if (cur_line == NULL)
      {
         /* Out of memory */
         edit_free_file_lines(first_line);
         return JB_ERR_MEMORY;
      }

      cur_line->type = FILE_LINE_UNPROCESSED;

      rval = edit_read_line(fp, &cur_line->raw, &cur_line->prefix, &cur_line->unprocessed);
      if ((rval != JB_ERR_OK) && (rval != JB_ERR_FILE))
      {
         /* Out of memory */
         edit_free_file_lines(first_line);
         return JB_ERR_MEMORY;
      }

   }
   while (rval != JB_ERR_FILE);

   /* EOF */

   /* We allocated one too many - free it */
   prev_line->next = NULL;
   free(cur_line);

   *pfile = first_line;
   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  edit_read_file
 *
 * Description :  Read a complete file into memory.
 *                Handles CGI parameter parsing.  If requested, also
 *                checks the file's modification timestamp.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  parameters = map of cgi parameters.
 *          3  :  require_version = true to check "ver" parameter.
 *          4  :  suffix = File extension, e.g. ".action".
 *          5  :  pfile = Destination for the file.  Will be set
 *                        to NULL on error.
 *
 * CGI Parameters :
 *    filename :  The name of the file to read, without the
 *                path or ".action" extension.
 *         ver :  (Only if require_version is nonzero)
 *                Timestamp of the actions file.  If wrong, this
 *                function fails with JB_ERR_MODIFIED.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if "filename" was not specified
 *                                  or is not valid.
 *                JB_ERR_FILE   if the file cannot be opened or
 *                              contains no data
 *                JB_ERR_MODIFIED if version checking was requested and
 *                                failed - the file was modified outside
 *                                of this CGI editor instance.
 *
 *********************************************************************/
jb_err edit_read_file(struct client_state *csp,
                      const struct map *parameters,
                      int require_version,
                      const char *suffix,
                      struct editable_file **pfile)
{
   struct file_line * lines;
   FILE * fp;
   jb_err err;
   char * filename;
   const char * identifier;
   struct editable_file * file;
   unsigned version = 0;
   struct stat statbuf[1];
   char version_buf[22];

   assert(csp);
   assert(parameters);
   assert(pfile);

   *pfile = NULL;

   err = get_file_name_param(csp, parameters, "filename", suffix,
                             &filename, &identifier);
   if (err)
   {
      return err;
   }

   if (stat(filename, statbuf) < 0)
   {
      /* Error, probably file not found. */
      free(filename);
      return JB_ERR_FILE;
   }
   version = (unsigned) statbuf->st_mtime;

   if (require_version)
   {
      unsigned specified_version;
      err = get_number_param(csp, parameters, "ver", &specified_version);
      if (err)
      {
         free(filename);
         return err;
      }

      if (version != specified_version)
      {
         return JB_ERR_MODIFIED;
      }
   }

   if (NULL == (fp = fopen(filename,"rt")))
   {
      free(filename);
      return JB_ERR_FILE;
   }

   err = edit_read_file_lines(fp, &lines);

   fclose(fp);

   if (err)
   {
      free(filename);
      return err;
   }

   file = (struct editable_file *) zalloc(sizeof(*file));
   if (err)
   {
      free(filename);
      edit_free_file_lines(lines);
      return err;
   }

   file->lines = lines;
   file->filename = filename;
   file->version = version;
   file->identifier = strdup(identifier);

   if (file->identifier == NULL)
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   /* Correct file->version_str */
   freez((char *)file->version_str);
   snprintf(version_buf, 22, "%u", file->version);
   version_buf[21] = '\0';
   file->version_str = strdup(version_buf);
   if (version_buf == NULL)
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   *pfile = file;
   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  edit_read_actions_file
 *
 * Description :  Read a complete actions file into memory.
 *                Handles CGI parameter parsing.  If requested, also
 *                checks the file's modification timestamp.
 *
 *                If this function detects an error in the categories
 *                JB_ERR_FILE, JB_ERR_MODIFIED, or JB_ERR_PARSE,
 *                then it handles it by filling in the specified
 *                response structure and returning JB_ERR_FILE.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  rsp = HTTP response.  Only filled in on error.
 *          2  :  parameters = map of cgi parameters.
 *          3  :  require_version = true to check "ver" parameter.
 *          4  :  pfile = Destination for the file.  Will be set
 *                        to NULL on error.
 *
 * CGI Parameters :
 *    filename :  The name of the actions file to read, without the
 *                path or ".action" extension.
 *         ver :  (Only if require_version is nonzero)
 *                Timestamp of the actions file.  If wrong, this
 *                function fails with JB_ERR_MODIFIED.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if "filename" was not specified
 *                                  or is not valid.
 *                JB_ERR_FILE  if the file does not contain valid data,
 *                             or if file cannot be opened or
 *                             contains no data, or if version
 *                             checking was requested and failed.
 *
 *********************************************************************/
jb_err edit_read_actions_file(struct client_state *csp,
                              struct http_response *rsp,
                              const struct map *parameters,
                              int require_version,
                              struct editable_file **pfile)
{
   jb_err err;
   struct editable_file *file;

   assert(csp);
   assert(parameters);
   assert(pfile);

   *pfile = NULL;

   err = edit_read_file(csp, parameters, require_version, ".action", &file);
   if (err)
   {
      /* Try to handle if possible */
      if (err == JB_ERR_FILE)
      {
         err = cgi_error_file(csp, rsp, lookup(parameters, "filename"));
      }
      else if (err == JB_ERR_MODIFIED)
      {
         err = cgi_error_modified(csp, rsp, lookup(parameters, "filename"));
      }
      if (err == JB_ERR_OK)
      {
         /*
          * Signal to higher-level CGI code that there was a problem but we
          * handled it, they should just return JB_ERR_OK.
          */
         err = JB_ERR_FILE;
      }
      return err;
   }

   err = edit_parse_actions_file(file);
   if (err)
   {
      if (err == JB_ERR_PARSE)
      {
         err = cgi_error_parse(csp, rsp, file);
         if (err == JB_ERR_OK)
         {
            /*
             * Signal to higher-level CGI code that there was a problem but we
             * handled it, they should just return JB_ERR_OK.
             */
            err = JB_ERR_FILE;
         }
      }
      edit_free_file(file);
      return err;
   }

   *pfile = file;
   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  get_file_name_param
 *
 * Description :  Get the name of the file to edit from the parameters
 *                passed to a CGI function.  This function handles
 *                security checks such as blocking urls containing
 *                "/" or ".", prepending the config file directory,
 *                and adding the specified suffix.
 *
 *                (This is an essential security check, otherwise
 *                users may be able to pass "../../../etc/passwd"
 *                and overwrite the password file [linux], "prn:"
 *                and print random data [Windows], etc...)
 *
 *                This function only allows filenames contining the
 *                characters '-', '_', 'A'-'Z', 'a'-'z', and '0'-'9'.
 *                That's probably too restrictive but at least it's
 *                secure.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  parameters = map of cgi parameters
 *           3 :  suffix = File extension, e.g. ".actions"
 *           4 :  pfilename = destination for full filename.  Caller
 *                free()s.  Set to NULL on error.
 *           5 :  pparam = destination for partial filename,
 *                suitable for use in another URL.  Allocated as part
 *                of the map "parameters", so don't free it.
 *                Set to NULL if not specified.
 *
 * Returns     :  JB_ERR_OK         on success
 *                JB_ERR_MEMORY     on out-of-memory
 *                JB_ERR_CGI_PARAMS if "filename" was not specified
 *                                  or is not valid.
 *
 *********************************************************************/
static jb_err get_file_name_param(struct client_state *csp,
                                  const struct map *parameters,
                                  const char *param_name,
                                  const char *suffix,
                                  char **pfilename,
                                  const char **pparam)
{
   const char *param;
   const char *s;
   char *name;
   char *fullpath;
   char ch;
   int len;

   assert(csp);
   assert(parameters);
   assert(suffix);
   assert(pfilename);
   assert(pparam);

   *pfilename = NULL;
   *pparam = NULL;

   param = lookup(parameters, param_name);
   if (!*param)
   {
      return JB_ERR_CGI_PARAMS;
   }

   *pparam = param;

   len = strlen(param);
   if (len >= FILENAME_MAX)
   {
      /* Too long. */
      return JB_ERR_CGI_PARAMS;
   }

   /* Check every character to see if it's legal */
   s = param;
   while ((ch = *s++) != '\0')
   {
      if ( ((ch < 'A') || (ch > 'Z'))
        && ((ch < 'a') || (ch > 'z'))
        && ((ch < '0') || (ch > '9'))
        && (ch != '-')
        && (ch != '_') )
      {
         /* Probable hack attempt. */
         return JB_ERR_CGI_PARAMS;
      }
   }

   /* Append extension */
   name = malloc(len + strlen(suffix) + 1);
   if (name == NULL)
   {
      return JB_ERR_MEMORY;
   }
   strcpy(name, param);
   strcpy(name + len, suffix);

   /* Prepend path */
   fullpath = make_path(csp->config->confdir, name);
   free(name);
   if (fullpath == NULL)
   {
      return JB_ERR_MEMORY;
   }

   /* Success */
   *pfilename = fullpath;

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  get_number_param
 *
 * Description :  Get a non-negative integer from the parameters
 *                passed to a CGI function.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  parameters = map of cgi parameters
 *           3 :  name = Name of CGI parameter to read
 *           4 :  pvalue = destination for value.
 *                         Set to -1 on error.
 *
 * Returns     :  JB_ERR_OK         on success
 *                JB_ERR_MEMORY     on out-of-memory
 *                JB_ERR_CGI_PARAMS if the parameter was not specified
 *                                  or is not valid.
 *
 *********************************************************************/
static jb_err get_number_param(struct client_state *csp,
                               const struct map *parameters,
                               char *name,
                               unsigned *pvalue)
{
   const char *param;
   char ch;
   unsigned value;

   assert(csp);
   assert(parameters);
   assert(name);
   assert(pvalue);

   *pvalue = -1;

   param = lookup(parameters, name);
   if (!*param)
   {
      return JB_ERR_CGI_PARAMS;
   }

   /* We don't use atoi because I want to check this carefully... */

   value = 0;
   while ((ch = *param++) != '\0')
   {
      if ((ch < '0') || (ch > '9'))
      {
         return JB_ERR_CGI_PARAMS;
      }

      ch -= '0';

      /* Note:
       *
       * <limits.h> defines UINT_MAX
       *
       * (UINT_MAX - ch) / 10 is the largest number that 
       *     can be safely multiplied by 10 then have ch added.
       */
      if (value > ((UINT_MAX - (unsigned)ch) / 10U))
      {
         return JB_ERR_CGI_PARAMS;
      }

      value = value * 10 + ch;
   }

   /* Success */
   *pvalue = value;

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  map_radio
 *
 * Description :  Map a set of radio button values.  E.g. if you have
 *                3 radio buttons, declare them as:
 *                  <option type="radio" name="xyz" @xyz-a@>
 *                  <option type="radio" name="xyz" @xyz-b@>
 *                  <option type="radio" name="xyz" @xyz-c@>
 *                Then map one of the @xyz-?@ variables to "checked"
 *                and all the others to empty by calling:
 *                map_radio(exports, "xyz", "abc", sel)
 *                Where 'sel' is 'a', 'b', or 'c'.
 *
 * Parameters  :
 *           1 :  exports = Exports map to modify.
 *           2 :  optionname = name for map
 *           3 :  values = null-terminated list of values;
 *           4 :  value = Selected value.
 *
 * CGI Parameters : None
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *
 *********************************************************************/
static jb_err map_radio(struct map * exports,
                        const char * optionname, 
                        const char * values,
                        char value)
{
   int len;
   char * buf;
   char * p;
   char c;
   
   assert(exports);
   assert(optionname);
   assert(values);

   len = strlen(optionname);
   buf = malloc(len + 3);
   if (buf == NULL)
   {
      return JB_ERR_MEMORY;
   }

   strcpy(buf, optionname);
   p = buf + len;
   *p++ = '-';
   p[1] = '\0';

   while ((c = *values++) != '\0')
   {
      if (c != value)
      {
         *p = c;
         if (map(exports, buf, 1, "", 1))
         {
            free(buf);
            return JB_ERR_MEMORY;
         }
      }
   }

   *p = value;
   if (map(exports, buf, 0, "checked", 1))
   {
      free(buf);
      return JB_ERR_MEMORY;
   }

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  actions_to_radio
 *
 * Description :  Converts a actionsfile entry into settings for
 *                radio buttons and edit boxes on a HTML form.
 *
 * Parameters  :
 *          1  :  exports = List of substitutions to add to.
 *          2  :  action  = Action to read
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *
 *********************************************************************/
static jb_err actions_to_radio(struct map * exports,
                               const struct action_spec *action)
{
   unsigned mask = action->mask;
   unsigned add  = action->add;
   int mapped_param;
   int checked;
   char current_mode;

   assert(exports);
   assert(action);

   mask = action->mask;
   add  = action->add;

   /* sanity - prevents "-feature +feature" */
   mask |= add;


#define DEFINE_ACTION_BOOL(name, bit)                 \
   if (!(mask & bit))                                 \
   {                                                  \
      current_mode = 'n';                             \
   }                                                  \
   else if (add & bit)                                \
   {                                                  \
      current_mode = 'y';                             \
   }                                                  \
   else                                               \
   {                                                  \
      current_mode = 'x';                             \
   }                                                  \
   if (map_radio(exports, name, "ynx", current_mode)) \
   {                                                  \
      return JB_ERR_MEMORY;                           \
   }

#define DEFINE_ACTION_STRING(name, bit, index)        \
   DEFINE_ACTION_BOOL(name, bit);                     \
   mapped_param = 0;

#define DEFINE_CGI_PARAM_RADIO(name, bit, index, value, is_default)  \
   if (add & bit)                                                    \
   {                                                                 \
      checked = !strcmp(action->string[index], value);               \
   }                                                                 \
   else                                                              \
   {                                                                 \
      checked = is_default;                                          \
   }                                                                 \
   mapped_param |= checked;                                          \
   if (map(exports, name "-param-" value, 1, (checked ? "checked" : ""), 1)) \
   {                                                                 \
      return JB_ERR_MEMORY;                                          \
   }

#define DEFINE_CGI_PARAM_CUSTOM(name, bit, index, default_val)       \
   if (map(exports, name "-param-custom", 1,                         \
           ((!mapped_param) ? "checked" : ""), 1))                   \
   {                                                                 \
      return JB_ERR_MEMORY;                                          \
   }                                                                 \
   if (map(exports, name "-param", 1,                                \
           (((add & bit) && !mapped_param) ?                         \
           action->string[index] : default_val), 1))                 \
   {                                                                 \
      return JB_ERR_MEMORY;                                          \
   }

#define DEFINE_CGI_PARAM_NO_RADIO(name, bit, index, default_val)     \
   if (map(exports, name "-param", 1,                                \
           ((add & bit) ? action->string[index] : default_val), 1))  \
   {                                                                 \
      return JB_ERR_MEMORY;                                          \
   }

#define DEFINE_ACTION_MULTI(name, index)              \
   if (action->multi_add[index]->first)               \
   {                                                  \
      current_mode = 'y';                             \
   }                                                  \
   else if (action->multi_remove_all[index])          \
   {                                                  \
      current_mode = 'n';                             \
   }                                                  \
   else if (action->multi_remove[index]->first)       \
   {                                                  \
      current_mode = 'y';                             \
   }                                                  \
   else                                               \
   {                                                  \
      current_mode = 'x';                             \
   }                                                  \
   if (map_radio(exports, name, "ynx", current_mode)) \
   {                                                  \
      return JB_ERR_MEMORY;                           \
   }

#define DEFINE_ACTION_ALIAS 0 /* No aliases for output */

#include "actionlist.h"

#undef DEFINE_ACTION_MULTI
#undef DEFINE_ACTION_STRING
#undef DEFINE_ACTION_BOOL
#undef DEFINE_ACTION_ALIAS
#undef DEFINE_CGI_PARAM_CUSTOM
#undef DEFINE_CGI_PARAM_RADIO
#undef DEFINE_CGI_PARAM_NO_RADIO

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  actions_from_radio
 *
 * Description :  Converts a map of parameters passed to a CGI function
 *                into an actionsfile entry.
 *
 * Parameters  :
 *          1  :  parameters = parameters to the CGI call
 *          2  :  action  = Action to change.  Must be valid before
 *                          the call, actions not specified will be
 *                          left unchanged.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *
 *********************************************************************/
static jb_err actions_from_radio(const struct map * parameters,
                              struct action_spec *action)
{
   const char * param;
   char * param_dup;
   char ch;

   assert(parameters);
   assert(action);

#define DEFINE_ACTION_BOOL(name, bit)                 \
   if (NULL != (param = lookup(parameters, name)))    \
   {                                                  \
      ch = toupper((int)param[0]);                    \
      if (ch == 'Y')                                  \
      {                                               \
         action->add  |= bit;                         \
         action->mask |= bit;                         \
      }                                               \
      else if (ch == 'N')                             \
      {                                               \
         action->add  &= ~bit;                        \
         action->mask &= ~bit;                        \
      }                                               \
      else if (ch == 'X')                             \
      {                                               \
         action->add  &= ~bit;                        \
         action->mask |= bit;                         \
      }                                               \
   }

#define DEFINE_ACTION_STRING(name, bit, index)                    \
   if (NULL != (param = lookup(parameters, name)))                \
   {                                                              \
      ch = toupper((int)param[0]);                                \
      if (ch == 'Y')                                              \
      {                                                           \
         param = lookup(parameters, name "-mode");                \
         if ((*param == '\0') || (0 == strcmp(param, "CUSTOM")))  \
         {                                                        \
            param = lookup(parameters, name "-param");            \
         }                                                        \
         if (*param != '\0')                                      \
         {                                                        \
            if (NULL == (param_dup = strdup(param)))              \
            {                                                     \
               return JB_ERR_MEMORY;                              \
            }                                                     \
            freez(action->string[index]);                         \
            action->add  |= bit;                                  \
            action->mask |= bit;                                  \
            action->string[index] = param_dup;                    \
         }                                                        \
      }                                                           \
      else if (ch == 'N')                                         \
      {                                                           \
         if (action->add & bit)                                   \
         {                                                        \
            freez(action->string[index]);                         \
         }                                                        \
         action->add  &= ~bit;                                    \
         action->mask &= ~bit;                                    \
      }                                                           \
      else if (ch == 'X')                                         \
      {                                                           \
         if (action->add & bit)                                   \
         {                                                        \
            freez(action->string[index]);                         \
         }                                                        \
         action->add  &= ~bit;                                    \
         action->mask |= bit;                                     \
      }                                                           \
   }

#define DEFINE_ACTION_MULTI(name, index)                          \
   if (NULL != (param = lookup(parameters, name)))                \
   {                                                              \
      ch = toupper((int)param[0]);                                \
      if (ch == 'Y')                                              \
      {                                                           \
         /* FIXME */                                              \
      }                                                           \
      else if (ch == 'N')                                         \
      {                                                           \
         list_remove_all(action->multi_add[index]);               \
         list_remove_all(action->multi_remove[index]);            \
         action->multi_remove_all[index] = 1;                     \
      }                                                           \
      else if (ch == 'X')                                         \
      {                                                           \
         list_remove_all(action->multi_add[index]);               \
         list_remove_all(action->multi_remove[index]);            \
         action->multi_remove_all[index] = 0;                     \
      }                                                           \
   }

#define DEFINE_CGI_PARAM_CUSTOM(name, bit, index, default_val)
#define DEFINE_CGI_PARAM_RADIO(name, bit, index, value, is_default)
#define DEFINE_CGI_PARAM_NO_RADIO(name, bit, index, default_val)

#define DEFINE_ACTION_ALIAS 0 /* No aliases for URL parsing */

#include "actionlist.h"

#undef DEFINE_ACTION_MULTI
#undef DEFINE_ACTION_STRING
#undef DEFINE_ACTION_BOOL
#undef DEFINE_ACTION_ALIAS
#undef DEFINE_CGI_PARAM_CUSTOM
#undef DEFINE_CGI_PARAM_RADIO
#undef DEFINE_CGI_PARAM_NO_RADIO

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  cgi_error_modified
 *
 * Description :  CGI function that is called when a file is modified
 *                outside the CGI editor.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  filename = The file that was modified.
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_error_modified(struct client_state *csp,
                          struct http_response *rsp,
                          const char *filename)
{
   struct map *exports;
   jb_err err;

   assert(csp);
   assert(rsp);
   assert(filename);

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      return JB_ERR_MEMORY;
   }

   err = map(exports, "filename", 1, filename, 1);
   if (err)
   {
      free_map(exports);
      return err;
   }

   return template_fill_for_cgi(csp, "cgi-error-modified", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_error_parse
 *
 * Description :  CGI function that is called when a file cannot
 *                be parsed by the CGI editor.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  file = The file that was modified.
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_error_parse(struct client_state *csp,
                       struct http_response *rsp,
                       struct editable_file *file)
{
   struct map *exports;
   jb_err err;
   struct file_line *cur_line;

   assert(csp);
   assert(rsp);
   assert(file);

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      return JB_ERR_MEMORY;
   }

   err = map(exports, "filename", 1, file->identifier, 1);
   err = err || map(exports, "parse-error", 1, file->parse_error_text, 1);

   cur_line = file->parse_error;
   assert(cur_line);

   err = err || map(exports, "line-raw", 1, html_encode(cur_line->raw), 0);
   err = err || map(exports, "line-data", 1, html_encode(cur_line->unprocessed), 0);

   if (err)
   {
      free_map(exports);
      return err;
   }

   return template_fill_for_cgi(csp, "cgi-error-parse", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_error_file
 *
 * Description :  CGI function that is called when a file cannot be
 *                opened by the CGI editor.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  filename = The file that was modified.
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_error_file(struct client_state *csp,
                      struct http_response *rsp,
                      const char *filename)
{
   struct map *exports;
   jb_err err;

   assert(csp);
   assert(rsp);
   assert(filename);

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      return JB_ERR_MEMORY;
   }

   err = map(exports, "filename", 1, filename, 1);
   if (err)
   {
      free_map(exports);
      return err;
   }

   return template_fill_for_cgi(csp, "cgi-error-file", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_error_bad_param
 *
 * Description :  CGI function that is called if the parameters
 *                (query string) for a CGI were wrong.
 *               
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *
 * CGI Parameters : none
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error.  
 *
 *********************************************************************/
jb_err cgi_error_disabled(struct client_state *csp,
                          struct http_response *rsp)
{
   struct map *exports;

   assert(csp);
   assert(rsp);

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      return JB_ERR_MEMORY;
   }

   return template_fill_for_cgi(csp, "cgi-error-disabled", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions
 *
 * Description :  CGI function that allows the user to choose which
 *                actions file to edit.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : None
 *
 * Returns     :  JB_ERR_OK on success
 *                JB_ERR_MEMORY on out-of-memory error
 *
 *********************************************************************/
jb_err cgi_edit_actions(struct client_state *csp,
                        struct http_response *rsp,
                        const struct map *parameters)
{

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   /* FIXME: Incomplete */
   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      return JB_ERR_MEMORY;
   }
   if (enlist_unique_header(rsp->headers, "Location", "http://ijbswa.sourceforge.net/config/edit-actions-list?filename=edit"))
   {
      free(rsp->status);
      rsp->status = NULL;
      return JB_ERR_MEMORY;
   }

   return JB_ERR_OK;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_list
 *
 * Description :  CGI function that edits the actions list.
 *                FIXME: This function shouldn't FATAL ever.
 *                FIXME: This function doesn't check the retval of map()
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : filename
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_FILE   if the file cannot be opened or
 *                              contains no data
 *                JB_ERR_CGI_PARAMS if "filename" was not specified
 *                                  or is not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_list(struct client_state *csp,
                             struct http_response *rsp,
                             const struct map *parameters)
{
   char * section_template;
   char * url_template;
   char * sections;
   char * urls;
   char buf[50];
   char * s;
   struct map * exports;
   struct map * section_exports;
   struct map * url_exports;
   struct editable_file * file;
   struct file_line * cur_line;
   int line_number = 0;
   int url_1_2;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = edit_read_actions_file(csp, rsp, parameters, 0, &file);
   if (err)
   {
      /* No filename specified, can't read file, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   err = map(exports, "filename", 1, file->identifier, 1);
   err = err || map(exports, "ver", 1, file->version_str, 1);
   if (err)
   {
      edit_free_file(file);
      free_map(exports);
      return err;
   }

   /* Should do all global exports above this point */

   err = template_load(csp, &section_template, "edit-actions-list-section");
   if (err)
   {
      edit_free_file(file);
      free_map(exports);
      if (err == JB_ERR_FILE)
      {
         return cgi_error_no_template(csp, rsp, "edit-actions-list-section");
      }
      return err;
   }
   
   err = template_load(csp, &url_template, "edit-actions-list-url");
   if (err)
   {
      free(section_template);
      edit_free_file(file);
      free_map(exports);
      if (err == JB_ERR_FILE)
      {
         return cgi_error_no_template(csp, rsp, "edit-actions-list-url");
      }
      return err;
   }

   err = template_fill(&section_template, exports);
   if (err)
   {
      free(url_template);
      edit_free_file(file);
      free_map(exports);
      free(url_template);
      return err;
   }

   err = template_fill(&url_template, exports);
   if (err)
   {
      free(section_template);
      edit_free_file(file);
      free_map(exports);
      return err;
   }

   /* Find start of actions in file */
   cur_line = file->lines;
   line_number = 1;
   while ((cur_line != NULL) && (cur_line->type != FILE_LINE_ACTION))
   {
      cur_line = cur_line->next;
      line_number++;
   }

   if (NULL == (sections = strdup("")))
   {
      free(section_template);
      free(url_template);
      edit_free_file(file);
      free_map(exports);
      return JB_ERR_MEMORY;
   }

   while ((cur_line != NULL) && (cur_line->type == FILE_LINE_ACTION))
   {
      if (NULL == (section_exports = new_map()))
      {
         free(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         return JB_ERR_MEMORY;
      }

      snprintf(buf, 50, "%d", line_number);
      err = map(section_exports, "sectionid", 1, buf, 1);

      err = err || map(section_exports, "actions", 1, 
                       actions_to_html(cur_line->data.action), 0);

      if ((cur_line->next != NULL) && (cur_line->next->type == FILE_LINE_URL))
      {
         /* This section contains at least one URL, don't allow delete */
         err = err || map_block_killer(section_exports, "empty-section");
      }

      if (err)
      {
         free(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         free_map(section_exports);
         return err;
      }

      /* Should do all section-specific exports above this point */

      if (NULL == (urls = strdup("")))
      {
         free(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         free_map(section_exports);
         return JB_ERR_MEMORY;
      }

      url_1_2 = 2;

      cur_line = cur_line->next;
      line_number++;

      while ((cur_line != NULL) && (cur_line->type == FILE_LINE_URL))
      {
         if (NULL == (url_exports = new_map()))
         {
            free(urls);
            free(sections);
            free(section_template);
            free(url_template);
            edit_free_file(file);
            free_map(exports);
            free_map(section_exports);
            return JB_ERR_MEMORY;
         }

         snprintf(buf, 50, "%d", line_number);
         err = map(url_exports, "urlid", 1, buf, 1);

         snprintf(buf, 50, "%d", url_1_2);
         err = err || map(url_exports, "url-1-2", 1, buf, 1);

         err = err || map(url_exports, "url", 1, 
                          html_encode(cur_line->unprocessed), 0);

         if (err)
         {
            free(urls);
            free(sections);
            free(section_template);
            free(url_template);
            edit_free_file(file);
            free_map(exports);
            free_map(section_exports);
            free_map(url_exports);
            return err;
         }

         if (NULL == (s = strdup(url_template)))
         {
            free(urls);
            free(sections);
            free(section_template);
            free(url_template);
            edit_free_file(file);
            free_map(exports);
            free_map(section_exports);
            free_map(url_exports);
            return JB_ERR_MEMORY;
         }

         err =        template_fill(&s, section_exports);
         err = err || template_fill(&s, url_exports);
         err = err || string_append(&urls, s);
         free_map(url_exports);
         freez(s);

         if (err)
         {
            freez(urls);
            free(sections);
            free(section_template);
            free(url_template);
            edit_free_file(file);
            free_map(exports);
            free_map(section_exports);
            return err;
         }

         url_1_2 = 3 - url_1_2;

         cur_line = cur_line->next;
         line_number++;
      }

      err = map(section_exports, "urls", 1, urls, 0);

      if (err)
      {
         free(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         free_map(section_exports);
         return err;
      }

      /* Could also do section-specific exports here, but it wouldn't be as fast */

      if (NULL == (s = strdup(section_template)))
      {
         free(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         free_map(section_exports);
         return JB_ERR_MEMORY;
      }

      err = template_fill(&s, section_exports);
      err = err || string_append(&sections, s);
      freez(s);
      free_map(section_exports);

      if (err)
      {
         freez(sections);
         free(section_template);
         free(url_template);
         edit_free_file(file);
         free_map(exports);
         return err;
      }
   }

   edit_free_file(file);
   free(section_template);
   free(url_template);

   err = map(exports, "sections", 1, sections, 0);
   if (err)
   {
      free_map(exports);
      return err;
   }

   /* Could also do global exports here, but it wouldn't be as fast */

   return template_fill_for_cgi(csp, "edit-actions-list", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions
 *
 * Description :  CGI function that edits the Actions list.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : None
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_for_url(struct client_state *csp,
                                struct http_response *rsp,
                                const struct map *parameters)
{
   struct map * exports;
   unsigned sectionid;
   struct editable_file * file;
   struct file_line * cur_line;
   unsigned line_number;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   if (err)
   {
      return err;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   cur_line = file->lines;

   for (line_number = 1; (cur_line != NULL) && (line_number < sectionid); line_number++)
   {
      cur_line = cur_line->next;
   }

   if ( (cur_line == NULL)
     || (line_number != sectionid)
     || (sectionid < 1)
     || (cur_line->type != FILE_LINE_ACTION))
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   if (NULL == (exports = default_exports(csp, NULL)))
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   err = map(exports, "filename", 1, file->identifier, 1);
   err = err || map(exports, "ver", 1, file->version_str, 1);
   err = err || map(exports, "section", 1, lookup(parameters, "section"), 1);

   err = err || actions_to_radio(exports, cur_line->data.action);

   edit_free_file(file);

   if (err)
   {
      free_map(exports);
      return err;
   }

   return template_fill_for_cgi(csp, "edit-actions-for-url", exports, rsp);
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_submit
 *
 * Description :  CGI function that actually edits the Actions list.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters : None
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_submit(struct client_state *csp,
                               struct http_response *rsp,
                               const struct map *parameters)
{
   int sectionid;
   char * actiontext;
   char * newtext;
   int len;
   struct editable_file * file;
   struct file_line * cur_line;
   int line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   if (err)
   {
      return err;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   cur_line = file->lines;

   for (line_number = 1; (cur_line != NULL) && (line_number < sectionid); line_number++)
   {
      cur_line = cur_line->next;
   }

   if ( (cur_line == NULL)
     || (line_number != sectionid)
     || (sectionid < 1)
     || (cur_line->type != FILE_LINE_ACTION))
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   err = actions_from_radio(parameters, cur_line->data.action);
   if(err)
   {
      /* Out of memory */
      edit_free_file(file);
      return err;
   }

   if (NULL == (actiontext = actions_to_text(cur_line->data.action)))
   {
      /* Out of memory */
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   len = strlen(actiontext);
   if (len == 0)
   {
      /*
       * Empty action - must special-case this.
       * Simply setting len to 1 is sufficient...
       */
      len = 1;
   }

   if (NULL == (newtext = malloc(len + 2)))
   {
      /* Out of memory */
      free(actiontext);
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }
   strcpy(newtext, actiontext);
   free(actiontext);
   newtext[0]       = '{';
   newtext[len]     = '}';
   newtext[len + 1] = '\0';

   freez(cur_line->raw);
   freez(cur_line->unprocessed);
   cur_line->unprocessed = newtext;

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_url
 *
 * Description :  CGI function that actually edits a URL pattern in
 *                an actions file.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *    filename : Identifies the file to edit
 *         ver : File's last-modified time
 *     section : Line number of section to edit
 *     pattern : Line number of pattern to edit
 *      newval : New value for pattern
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_url(struct client_state *csp,
                            struct http_response *rsp,
                            const struct map *parameters)
{
   unsigned sectionid;
   unsigned patternid;
   const char * newval;
   char * new_pattern;
   struct editable_file * file;
   struct file_line * cur_line;
   unsigned line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   err = err || get_number_param(csp, parameters, "pattern", &patternid);
   if (err)
   {
      return err;
   }

   newval = lookup(parameters, "newval");

   if ((*newval == '\0') || (sectionid < 1U) || (patternid < 1U))
   {
      return JB_ERR_CGI_PARAMS;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   line_number = 1;
   cur_line = file->lines;

   while ((cur_line != NULL) && (line_number < sectionid))
   {
      cur_line = cur_line->next;
      line_number++;
   }

   if ( (cur_line == NULL)
     || (cur_line->type != FILE_LINE_ACTION))
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   while (line_number < patternid)
   {
      cur_line = cur_line->next;
      line_number++;

      if ( (cur_line == NULL)
        || ( (cur_line->type != FILE_LINE_URL)
          && (cur_line->type != FILE_LINE_BLANK) ) )
      {
         /* Invalid "patternid" parameter */
         edit_free_file(file);
         return JB_ERR_CGI_PARAMS;
      }
   }

   if (cur_line->type != FILE_LINE_URL)
   {
      /* Invalid "patternid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   /* At this point, the line to edit is in cur_line */

   new_pattern = strdup(newval);
   if (NULL == new_pattern)
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   freez(cur_line->raw);
   freez(cur_line->unprocessed);
   cur_line->unprocessed = new_pattern;

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_add_url
 *
 * Description :  CGI function that actually adds a URL pattern to
 *                an actions file.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *    filename : Identifies the file to edit
 *         ver : File's last-modified time
 *     section : Line number of section to edit
 *      newval : New pattern
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_add_url(struct client_state *csp,
                                struct http_response *rsp,
                                const struct map *parameters)
{
   unsigned sectionid;
   unsigned patternid;
   const char * newval;
   char * new_pattern;
   struct file_line * new_line;
   struct editable_file * file;
   struct file_line * cur_line;
   unsigned line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   if (err)
   {
      return err;
   }

   newval = lookup(parameters, "newval");

   if ((*newval == '\0') || (sectionid < 1U) || (patternid < 1U))
   {
      return JB_ERR_CGI_PARAMS;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   line_number = 1;
   cur_line = file->lines;

   while ((cur_line != NULL) && (line_number < sectionid))
   {
      cur_line = cur_line->next;
      line_number++;
   }

   if ( (cur_line == NULL)
     || (cur_line->type != FILE_LINE_ACTION))
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   /* At this point, the section header is in cur_line - add after this. */

   new_pattern = strdup(newval);
   if (NULL == new_pattern)
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   /* Allocate the new line */
   new_line = (struct file_line *)zalloc(sizeof(*new_line));
   if (new_line == NULL)
   {
      free(new_pattern);
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   /* Fill in the data members of the new line */
   new_line->raw = NULL;
   new_line->prefix = NULL;
   new_line->unprocessed = new_pattern;
   new_line->type = FILE_LINE_URL;

   /* Link new_line into the list, after cur_line */
   new_line->next = cur_line->next;
   cur_line->next = new_line;

   /* Done making changes, now commit */

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_remove_url
 *
 * Description :  CGI function that actually removes a URL pattern from
 *                the actions file.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *    filename : Identifies the file to edit
 *         ver : File's last-modified time
 *     section : Line number of section to edit
 *     pattern : Line number of pattern to edit
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_remove_url(struct client_state *csp,
                                   struct http_response *rsp,
                                   const struct map *parameters)
{
   unsigned sectionid;
   unsigned patternid;
   struct editable_file * file;
   struct file_line * cur_line;
   struct file_line * prev_line;
   unsigned line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   err = err || get_number_param(csp, parameters, "pattern", &patternid);
   if (err)
   {
      return err;
   }


   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   line_number = 1;
   cur_line = file->lines;

   while ((cur_line != NULL) && (line_number < sectionid))
   {
      cur_line = cur_line->next;
      line_number++;
   }

   if ( (cur_line == NULL)
     || (cur_line->type != FILE_LINE_ACTION))
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   prev_line = NULL;
   while (line_number < patternid)
   {
      prev_line = cur_line;
      cur_line = cur_line->next;
      line_number++;

      if ( (cur_line == NULL)
        || ( (cur_line->type != FILE_LINE_URL)
          && (cur_line->type != FILE_LINE_BLANK) ) )
      {
         /* Invalid "patternid" parameter */
         edit_free_file(file);
         return JB_ERR_CGI_PARAMS;
      }
   }

   if (cur_line->type != FILE_LINE_URL)
   {
      /* Invalid "patternid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   assert(prev_line);

   /* At this point, the line to remove is in cur_line, and the previous
    * one is in prev_line
    */

   /* Unlink cur_line */
   prev_line->next = cur_line->next;
   cur_line->next = NULL;

   /* Free cur_line */
   edit_free_file_lines(cur_line);

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_section_remove
 *
 * Description :  CGI function that actually removes a whole section from
 *                the actions file.  The section must be empty first
 *                (else JB_ERR_CGI_PARAMS).
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *    filename : Identifies the file to edit
 *         ver : File's last-modified time
 *     section : Line number of section to edit
 *     pattern : Line number of pattern to edit
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_section_remove(struct client_state *csp,
                                       struct http_response *rsp,
                                       const struct map *parameters)
{
   unsigned sectionid;
   struct editable_file * file;
   struct file_line * cur_line;
   struct file_line * prev_line;
   unsigned line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   if (err)
   {
      return err;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   line_number = 1;
   cur_line = file->lines;

   prev_line = NULL;
   while ((cur_line != NULL) && (line_number < sectionid))
   {
      prev_line = cur_line;
      cur_line = cur_line->next;
      line_number++;
   }

   if ( (cur_line == NULL)
     || (cur_line->type != FILE_LINE_ACTION) )
   {
      /* Invalid "sectionid" parameter */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   if ( (cur_line->next != NULL)
     && (cur_line->next->type == FILE_LINE_URL) )
   {
      /* Section not empty. */
      edit_free_file(file);
      return JB_ERR_CGI_PARAMS;
   }

   /* At this point, the line to remove is in cur_line, and the previous
    * one is in prev_line
    */

   /* Unlink cur_line */
   if (prev_line == NULL)
   {
      /* Removing the first line from the file */
      file->lines = cur_line->next;
   }
   else
   {
      prev_line->next = cur_line->next;
   }
   cur_line->next = NULL;

   /* Free cur_line */
   edit_free_file_lines(cur_line);

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_edit_actions_section_add
 *
 * Description :  CGI function that adds a new empty section to
 *                an actions file.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *    filename : Identifies the file to edit
 *         ver : File's last-modified time
 *     section : Line number of section to add after, 0 for start
 *               of file.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *                JB_ERR_CGI_PARAMS if the CGI parameters are not
 *                                  specified or not valid.
 *
 *********************************************************************/
jb_err cgi_edit_actions_section_add(struct client_state *csp,
                                    struct http_response *rsp,
                                    const struct map *parameters)
{
   unsigned sectionid;
   struct file_line * new_line;
   char * new_text;
   struct editable_file * file;
   struct file_line * cur_line;
   unsigned line_number;
   char * target;
   jb_err err;

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_EDIT_ACTIONS))
   {
      return cgi_error_disabled(csp, rsp);
   }

   err = get_number_param(csp, parameters, "section", &sectionid);
   if (err)
   {
      return err;
   }

   err = edit_read_actions_file(csp, rsp, parameters, 1, &file);
   if (err)
   {
      /* No filename specified, can't read file, modified, or out of memory. */
      return (err == JB_ERR_FILE ? JB_ERR_OK : err);
   }

   line_number = 1;
   cur_line = file->lines;

   if (sectionid < 1U)
   {
      /* Add to start of file */
      if (cur_line != NULL)
      {
         /* There's something in the file, find the line before the first
          * action.
          */
         while ( (cur_line->next != NULL)
              && (cur_line->next->type != FILE_LINE_ACTION) )
         {
            cur_line = cur_line->next;
            line_number++;
         }
      }
   }
   else
   {
      /* Add after stated section. */
      while ((cur_line != NULL) && (line_number < sectionid))
      {
         cur_line = cur_line->next;
         line_number++;
      }

      if ( (cur_line == NULL)
        || (cur_line->type != FILE_LINE_ACTION))
      {
         /* Invalid "sectionid" parameter */
         edit_free_file(file);
         return JB_ERR_CGI_PARAMS;
      }

      /* Skip through the section to find the last line in it. */
      while ( (cur_line->next != NULL)
           && (cur_line->next->type != FILE_LINE_ACTION) )
      {
         cur_line = cur_line->next;
         line_number++;
      }
   }

   /* At this point, the last line in the previous section is in cur_line
    * - add after this.  (Or if we need to add as the first line, cur_line
    * will be NULL).
    */

   new_text = strdup("{}");
   if (NULL == new_text)
   {
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   /* Allocate the new line */
   new_line = (struct file_line *)zalloc(sizeof(*new_line));
   if (new_line == NULL)
   {
      free(new_text);
      edit_free_file(file);
      return JB_ERR_MEMORY;
   }

   /* Fill in the data members of the new line */
   new_line->raw = NULL;
   new_line->prefix = NULL;
   new_line->unprocessed = new_text;
   new_line->type = FILE_LINE_ACTION;

   if (cur_line != NULL)
   {
      /* Link new_line into the list, after cur_line */
      new_line->next = cur_line->next;
      cur_line->next = new_line;
   }
   else
   {
      /* Link new_line into the list, as first line */
      new_line->next = file->lines;
      file->lines = new_line;
   }

   /* Done making changes, now commit */

   err = edit_write_file(file);
   if (err)
   {
      /* Error writing file */
      edit_free_file(file);
      return err;
   }

   target = strdup("http://ijbswa.sourceforge.net/config/edit-actions-list?filename=");
   string_append(&target, file->identifier);

   edit_free_file(file);

   if (target == NULL)
   {
      /* Out of memory */
      return JB_ERR_MEMORY;
   }

   rsp->status = strdup("302 Local Redirect from Junkbuster");
   if (rsp->status == NULL)
   {
      free(target);
      return JB_ERR_MEMORY;
   }
   err = enlist_unique_header(rsp->headers, "Location", target);
   free(target);

   return err;
}


/*********************************************************************
 *
 * Function    :  cgi_toggle
 *
 * Description :  CGI function that adds a new empty section to
 *                an actions file.
 *
 * Parameters  :
 *           1 :  csp = Current client state (buffers, headers, etc...)
 *           2 :  rsp = http_response data structure for output
 *           3 :  parameters = map of cgi parameters
 *
 * CGI Parameters :
 *         set : If present, how to change toggle setting:
 *               "enable", "disable", "toggle", or none (default).
 *        mini : If present, use mini reply template.
 *
 * Returns     :  JB_ERR_OK     on success
 *                JB_ERR_MEMORY on out-of-memory
 *
 *********************************************************************/
jb_err cgi_toggle(struct client_state *csp,
                  struct http_response *rsp,
                  const struct map *parameters)
{
   struct map *exports;
   char mode;
   const char *template_name;
   jb_err err;

   assert(csp);
   assert(rsp);
   assert(parameters);

   if (0 == (csp->config->feature_flags & RUNTIME_FEATURE_CGI_TOGGLE))
   {
      return cgi_error_disabled(csp, rsp);
   }

   if (NULL == (exports = default_exports(csp, "toggle")))
   {
      return JB_ERR_MEMORY;
   }

   mode = *(lookup(parameters, "set"));

   if (mode == 'e')
   {
      /* Enable */
      g_bToggleIJB = 1;
   }
   else if (mode == 'd')
   {
      /* Disable */
      g_bToggleIJB = 0;
   }
   else if (mode == 't')
   {
      /* Toggle */
      g_bToggleIJB = !g_bToggleIJB;
   }

   err = map_conditional(exports, "enabled", g_bToggleIJB);
   if (err)
   {
      free_map(exports);
      return err;
   }

   template_name = (*(lookup(parameters, "mini"))
                 ? "toggle-mini"
                 : "toggle");

   return template_fill_for_cgi(csp, template_name, exports, rsp);
}
#endif /* def FEATURE_CGI_EDIT_ACTIONS */


/*
  Local Variables:
  tab-width: 3
  end:
*/
