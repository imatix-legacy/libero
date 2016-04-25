!============================================================================*
!									     *
!   lrcalcd.l	Dialog for expression calculation function in lrcalc.c	     *
!									     *
!   Written:	95/03/19  Pieter Hintjens <ph@imatix.com>		     *
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
-source=lrcalc

After-Init:
    (--) Ok				    -> Expecting-Initial
	  + Get-Next-Token
    (--) Error				    ->
	  + Terminate-The-Program

Expecting-Initial:
    (--) Term-Op			    ->
	  + Allow-Signed-Number
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Number 			    -> Expecting-Operator
	  + Stack-The-Number
	  + Get-Next-Token
    (--) Left-Par			    -> Expecting-Operand
	  + Stack-The-Operator
	  + Get-Next-Token
    (--) End-Mark			    ->
	  + Terminate-The-Program

Expecting-Operand:
    (--) Term-Op			    ->
	  + Allow-Signed-Number
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Number 			    -> Expecting-Operator
	  + Stack-The-Number
	  + Get-Next-Token
    (--) Left-Par			    -> Expecting-Operand
	  + Stack-The-Operator
	  + Get-Next-Token

Expecting-Operator:
    (--) Term-Op			    -> Expecting-Operand
	  + Unstack-Ge-Operators
	  + Stack-The-Operator
	  + Get-Next-Token
    (--) Factor-Op			    -> Expecting-Operand
	  + Unstack-Ge-Operators
	  + Stack-The-Operator
	  + Get-Next-Token
    (--) End-Mark			    ->
	  + Unstack-All-Operators
	  + Unstack-If-End-Mark
	  + Terminate-The-Program
    (--) Right-Par			    -> Expecting-Operator
	  + Unstack-All-Operators
	  + Unstack-If-Left-Par
	  + Get-Next-Token

Defaults:
    (--) Number 			    ->
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Term-Op			    ->
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Factor-Op			    ->
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) End-Mark			    ->
	  + Signal-Token-Missing
	  + Terminate-The-Program
    (--) Left-Par			    ->
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Right-Par			    ->
	  + Signal-Invalid-Token
	  + Terminate-The-Program
    (--) Exception			    ->
	  + Terminate-The-Program
