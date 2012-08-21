/******************************************************************************
 * Copyright (C) 2010-11 Patrick Wacker
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 ******************************************************************************
 * Dont forget: svn propset svn:keywords "Date Author Rev HeadURL" filename
 ******************************************************************************
 * $HeadURL$
 * $Author$
 * $Date$
 * $Rev$
 *
 * description:
 *	Definition of global Variables
 *	the source were globals should be defined must #define DEFINEGLOBALSHERE
 *	before #inlcude "globalvars.h" (#undef after include!?)
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef GLOBALVARS_H
#define GLOBALVARS_H

#include "aqb_banking.h"
#include "abt_settings.h"
#include "widgets/debugdialogwidget.h"

#ifdef DEFINEGLOBALSHERE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN aqb_banking *banking;
EXTERN DebugDialogWidget *debugDialog;
EXTERN abt_settings *settings;

#endif // GLOBALVARS_H
