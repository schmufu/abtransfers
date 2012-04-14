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
 *	Speichert Informationen zu einem bekannten Dauerauftrag
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef ABT_STANDINGORDERINFO_H
#define ABT_STANDINGORDERINFO_H

#include "abt_transaction_base.h"


/*! \brief Daten von gespeicherten Daueraufträgen
 *
 * Diese Klasse kapselt die Daten eines Dauerauftrages.
 * Diese werden in einer Liste zu jedem Account geladen und stehen somit über
 * den jeweiligen account zur Verfügung.
 */

class abt_standingOrderInfo
{
public:
	abt_standingOrderInfo(abt_transaction *transaction);
	abt_standingOrderInfo(const AB_TRANSACTION *transaction);
	~abt_standingOrderInfo();
private:
	abt_transaction *t;
public:
	abt_transaction* getTransaction() const { return this->t; }
};

#endif // ABT_STANDINGORDERINFO_H
