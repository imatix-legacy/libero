!*===========================================================================*
!                                                                            *
!   lrloadd.l   Dialog description for lrload.c                              *
!                                                                            *
!   Written:    92/10/25  Pieter Hintjens <ph@imatix.com>                    *
!   Revised:    96/12/29                                                     *
!                                                                            *
!   FSM Code Generator.  Copyright (c) 1991-97 iMatix.                       *
!                                                                            *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 2 of the License, or        *
!   (at your option) any later version.                                      *
!                                                                            *
!   This program is distributed in the hope that it will be useful,          *
!   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
!   GNU General Public License for more details.                             *
!                                                                            *
!   You should have received a copy of the GNU General Public License        *
!   along with this program; if not, write to the Free Software              *
!   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
!*===========================================================================*
-source=lrload

After-Init:
    (--) Ok                                 -> Expect-State-Name
          + Open-Dialog-File
          + Expect-State-Name
          + Get-Next-Token

Expect-State-Name:
    (--) State-Name                         -> After-State
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Item-Name                          -> After-State
          + Warning-On-State-Name
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token

After-State:
    (--) State-Name                         -> After-State
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Superstate-Ref                     -> After-State
          + Attach-Uses-Superstate
          + Expect-State-Definition
          + Get-Next-Token
    (--) Event-Number                       -> Expect-Event-Name
          + Store-Event-Number
          + Expect-Event-Name
          + Get-Next-Token
    (--) Item-Name                          -> After-State
          + Warning-On-State-Name
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) State-Name-Term                    -> After-State
          + Get-Next-Token

Expect-Event-Name:
    (--) Item-Name                          -> Expect-Next-Pointer
          + Attach-New-Event
          + Expect-Next-Pointer
          + Get-Next-Token

Expect-Next-Pointer:
    (--) Next-Pointer                       -> After-Next-Pointer
          + Expect-Event-Definition
          + Get-Next-Token
    (--) Module-Flag                        -> After-Module-Flag
          + Attach-Same-Next-State
          + Expect-Module-Name
          + Get-Next-Token

After-Next-Pointer:
    (--) Item-Name                          -> After-Event
          + Attach-Next-State
          + Get-Next-Token
    (--) State-Name                         -> After-State
          + Attach-Same-Next-State
          + Attach-Empty-Module
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Event-Number                       -> Expect-Event-Name
          + Attach-Same-Next-State
          + Attach-Empty-Module
          + Store-Event-Number
          + Expect-Event-Name
          + Get-Next-Token
    (--) Module-Flag                        -> After-Module-Flag
          + Attach-Same-Next-State
          + Expect-Module-Name
          + Get-Next-Token
    (--) No-More                            ->
          + Attach-Same-Next-State
          + Attach-Empty-Module
          + Expand-Macro-Constructs
          + Build-Source-File-Name
          + Close-Dialog-File
          + Terminate-The-Program

After-Event:
    (--) State-Name                         -> After-State
          + Attach-Empty-Module
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Event-Number                       -> Expect-Event-Name
          + Attach-Empty-Module
          + Store-Event-Number
          + Expect-Event-Name
          + Get-Next-Token
    (--) Module-Flag                        -> After-Module-Flag
          + Expect-Module-Name
          + Get-Next-Token
    (--) Item-Name                          -> After-State
          + Attach-Empty-Module
          + Warning-On-State-Name
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token

After-Module-Flag:
    (--) Item-Name                          -> After-Module
          + Attach-New-Module
          + Get-Next-Token
    (--) State-Name                         -> After-State
          + Attach-Empty-Module
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Event-Number                       -> Expect-Event-Name
          + Attach-Empty-Module
          + Store-Event-Number
          + Expect-Event-Name
          + Get-Next-Token
    (--) No-More                            ->
          + Attach-Empty-Module
          + Expand-Macro-Constructs
          + Build-Source-File-Name
          + Close-Dialog-File
          + Terminate-The-Program

After-Module:
    (--) State-Name                         -> After-State
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token
    (--) Event-Number                       -> Expect-Event-Name
          + Store-Event-Number
          + Expect-Event-Name
          + Get-Next-Token
    (--) Module-Flag                        -> After-Module-Flag
          + Expect-Module-Name
          + Get-Next-Token
    (--) Item-Name                          -> After-State
          + Warning-On-State-Name
          + Attach-New-State
          + Expect-State-Definition
          + Get-Next-Token

Defaults:
    (--) State-Name                         ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) State-Name-Term                    ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Superstate-Ref                     ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Event-Number                       ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Module-Flag                        ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Item-Name                          ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Next-Pointer                       ->
          + Display-Unexpected-Token
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) Option                             ->
          + Process-Option-Value
          + Get-Next-Token
    (--) Comment                            ->
          + Process-Comment-Value
          + Get-Next-Token
    (--) Error                              ->
          + Close-Dialog-File
          + Return-Error-Feedback
          + Terminate-The-Program
    (--) No-More                            ->
          + Expand-Macro-Constructs
          + Build-Source-File-Name
          + Close-Dialog-File
          + Terminate-The-Program
