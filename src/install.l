!*===========================================================================*
!									     *
!   install.l	Re-build Libero on Unix box				     *
!		Generates shell script to compile, link, and install	     *
!									     *
!   Written:	95/03/10  Pieter Hintjens <ph@imatix.com>		     *
!   Revised:	96/12/29						     *
!									     *
!   FSM Code Generator.  Copyright (c) 1991-97 iMatix.			     *
!									     *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 2 of the License, or	     *
!   (at your option) any later version. 				     *
!									     *
!   This program is distributed in the hope that it will be useful,	     *
!   but WITHOUT ANY WARRANTY; without even the implied warranty of	     *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	     *
!   GNU General Public License for more details.			     *
!									     *
!   You should have received a copy of the GNU General Public License	     *
!   along with this program; if not, write to the Free Software 	     *
!   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     *
!*===========================================================================*
-schema=lrschema.ksh

After-Init:
    (--) Ok				    -> Have-Package
	  + What-System-Are-We-On
	  + What-Package-Do-We-Have

Have-Package:
    (--) Source 			    -> Before-Action
	  + Get-Source-Install-Action
    (--) Binary 			    -> Before-Action
	  + Get-Binary-Install-Action
    (--) Error				    ->
	  + Return-Error-Feedback
	  + Terminate-The-Program

Before-Action:
    (--) Build				    -> Have-Install-Directory
	  + Check-All-Files-Exist
	  + Compile-All-Subroutines
	  + Link-Main-Programs
	  + Get-Directory-Name
    (--) Install			    -> Have-Install-Directory
	  + Get-Directory-Name
    (--) Delete 			    -> Have-Delete-Directory
	  + Get-Directory-Name
    (--) Cleanup			    -> Have-Package
	  + Clean-Up-Directory
	  + What-Package-Do-We-Have
    (--) Error				    -> Have-Package
	  + What-Package-Do-We-Have

Have-Install-Directory:
    (--) Ok				    -> Have-Package
	  + Check-Directory-Writable
	  + Install-Product-Files
	  + What-Package-Do-We-Have
    (--) Current			    -> Have-Package
	  + What-Package-Do-We-Have
    (--) Error				    -> Have-Install-Directory
	  + Get-Directory-Name

Have-Delete-Directory:
    (--) Ok				    -> Have-Package
	  + Check-Directory-Writable
	  + Delete-Product-Files
	  + What-Package-Do-We-Have
    (--) Error				    -> Have-Delete-Directory
	  + Get-Directory-Name

Defaults:
    (--) Error				    ->
	  + Return-Error-Feedback
	  + Terminate-The-Program
    (--) Quit				    ->
	  + Terminate-The-Program
