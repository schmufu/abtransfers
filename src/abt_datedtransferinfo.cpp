/******************************************************************************
 * Copyright (C) 2012-2013 Patrick Wacker
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

/** @brief The supplied abt_transaction @a transaction will be deleted at
 *         destruction of this object.
 */
abt_datedTransferInfo::abt_datedTransferInfo(abt_transaction *transaction)
	: t(transaction)
{

}

/** @brief a copy of the supplied @a transaction will be created and used.
 *
 * \overload
 */
abt_datedTransferInfo::abt_datedTransferInfo(const AB_TRANSACTION *transaction)
{
	//Create a copie of the supplied transaction, use the copie.
	//This copie is deleted at the end of the object.
	AB_TRANSACTION *t = AB_Transaction_dup(transaction);
	this->t = new abt_transaction(t, true);
}

abt_datedTransferInfo::~abt_datedTransferInfo()
{
	delete this->t;
}
