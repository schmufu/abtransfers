/******************************************************************************
 * Copyright (C) 2011 Patrick Wacker
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
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef ABT_TRANSACTIONS_H
#define ABT_TRANSACTIONS_H

#include "abt_transaction_base.h"

class trans_StandingOrder : public abt_transaction
{
private:
	void load(int id);
	int id;

public:
	trans_StandingOrder(int id = 0);

	void save();

};


class trans_DatedTransfer : public abt_transaction
{
public:
	trans_DatedTransfer();
};


class trans_SingleTransfer : public abt_transaction
{
public:
	trans_SingleTransfer();
};

class trans_SingleDebitNote : public abt_transaction
{
public:
	trans_SingleDebitNote();
};

class trans_EuTransfer : public abt_transaction
{
public:
	trans_EuTransfer();
};

class trans_InternalTransfer : public abt_transaction
{
public:
	trans_InternalTransfer();
};

class trans_SepaTransfer : public abt_transaction
{
public:
	trans_SepaTransfer();
};

#endif // ABT_TRANSACTIONS_H
