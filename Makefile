# $Id$
#
# Written by and Copyright (C) 2001 the SourceForge
# Privoxy team. http://www.privoxy.org/
#
# Based on the Internet Junkbuster originally written
# by and Copyright (C) 1997 Anonymous Coders and 
# Junkbusters Corporation.  http://www.junkbusters.com
#
# This program is free software; you can redistribute it 
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public
# License for more details.
#
# The GNU General Public License should be included with
# this file.  If not, you can view it at
# http://www.gnu.org/copyleft/gpl.html
# or write to the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# $Log$
# Revision 1.10  2002/10/24 00:40:46  hal9
# Quote $$answer to avoid error on null value.
#
# Revision 1.9  2002/10/23 06:14:09  agotneja
# Added FreeBSD checks to stop people from using the default 'make'
#
# Revision 1.8  2002/10/23 05:41:45  agotneja
# Added checks for Solaris 'make' command, and more extensive checks that
# the user is running GNU make.
#
# Revision 1.7  2002/10/22 02:22:18  hal9
# Look for gmake first, and fall back to make. More Solaris trouble.
#
# Revision 1.6  2002/09/05 14:35:17  oes
# Change make to gmake to fix auto-build on Solaris
#
# Revision 1.5  2002/04/11 12:51:34  oes
# Bugfix
#
# Revision 1.4  2002/04/09 16:38:10  oes
# Added option to run the whole build process
#
# Revision 1.3  2002/03/26 22:29:54  swa
# we have a new homepage!
#
# Revision 1.2  2002/03/24 13:25:42  swa
# name change related issues
#
# Revision 1.1  2001/12/01 11:24:29  jongfoster
# Will display a warning if non-GNU make is used
#
#

#############################################################################

# This script will first try to use the GNU make command, then the standard
# make command, checking whether the command returns 'GNU' as part of its
# version string. Amend this to point to your GNU make command if it is
# not in your path.
# Further tests; 
#   GNU  'make' sets the MAKE_VERSION variable
#   Solaris 'make' sets the HOST_ARCH variable
#   FreeBSD 'make' sets the MACHINE_ARCH variable
# We check if this isn't GNU but matches one of the above we error out

GNU_MAKE_CMD = gmake
MAKE_CMD     = make

error:
	@if [ -f GNUmakefile ]; then \
	    echo "***"; \
	    echo "*** You are not using the GNU version of Make - maybe it's called gmake"; \
	    echo "*** or it's in a different PATH? Please read INSTALL." ; \
	    echo "***"; \
	    exit 1; \
	 elif test -n "$(HOST_ARCH)"  && test -z "$(MAKE_VERSION)" ; then \
	    echo "***"; \
	    echo "*** You are not using GNU Make on Solaris, please make sure you do" ; \
	    echo "*** and re-run 'make' "; \
	    echo "***"; \
	    exit 1 ; \
	 elif test -n "$(MACHINE_ARCH)"  && test -z "$(MAKE_VERSION)" ; then \
	    echo "***"; \
	    echo "*** You are not using GNU Make on FreeBSD, please make sure you do" ; \
	    echo "*** and re-run 'make' "; \
	    echo "***"; \
	    exit 1 ; \
	 else \
	    echo "***"; \
	    echo "*** To build this program, you must run"; \
	    echo "*** autoheader && autoconf && ./configure and then run GNU make."; \
	    echo "***"; \
	    echo -n "*** Shall I do this for you now? (y/n) "; \
	    read answer; \
	    if [ "$$answer" = "y" ]; then \
		autoheader && autoconf && ./configure || exit 1; \
	  	if $(GNU_MAKE_CMD) -v |grep GNU >/dev/null 2>/dev/null; then \
		   $(GNU_MAKE_CMD) ;\
		elif $(MAKE_CMD) -v |grep GNU >/dev/null 2>/dev/null; then \
		   $(MAKE_CMD) ;\
		else \
		   echo "Neither 'make' nor 'gmake' are GNU compatible!" ; \
		   echo "Please read INSTALL." ; \
		   exit 1 ; \
		fi ;\
	    fi; \
	 fi

.PHONY: error


#############################################################################

## Local Variables:
## tab-width: 3
## end:
