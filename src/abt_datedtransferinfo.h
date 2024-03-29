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


#ifndef ABT_DATEDTRANSFERINFO_H
#define ABT_DATEDTRANSFERINFO_H

#include "abt_transaction_base.h"


/** @brief information about dated transfers
 *
 * This class includes all information about a dated transfer.
 *
 * This information are loaded for each account and therefore are reachable for
 * each individuell account.
 */
class abt_datedTransferInfo
{
public:
	abt_datedTransferInfo(abt_transaction *transaction);
	abt_datedTransferInfo(const AB_TRANSACTION *transaction);
	~abt_datedTransferInfo();
private:
	abt_transaction *t;
public:
	abt_transaction* getTransaction() const { return this->t; }
};

#endif // ABT_DATEDTRANSFERINFO_H
