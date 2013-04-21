/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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
 *	Speichert Informationen zu einer bekannten terminierten Überweisung
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "abt_datedtransferinfo.h"

#include <QDebug>

/**
  * Die übergebene abt_transaction \a transaction wird beim löschen dieses
  * Objectes auch wieder gelöscht.
  */
abt_datedTransferInfo::abt_datedTransferInfo(abt_transaction *transaction)
        : m_t(transaction)
{

}

/** \overload
  * Es wird eine Kopie der übergebenen AB_TRANSACTION \a transaction erstellt
  * und diese Kopie verwendet.
  */
abt_datedTransferInfo::abt_datedTransferInfo(const AB_TRANSACTION *transaction)
{
	//wenn wir eine AB_TRANSACTION erhalten erstellen wir davon eine
	//Kopie und von dieser eine abt_transaction die im destructor wieder
	//gelöscht wird.
	AB_TRANSACTION *t = AB_Transaction_dup(transaction);
        this->m_t = new abt_transaction(t, true);
}

abt_datedTransferInfo::~abt_datedTransferInfo()
{
        delete this->m_t;	//abt_Transaction Object löschen
        this->m_t = NULL;
}
