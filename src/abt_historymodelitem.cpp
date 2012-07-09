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
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "abt_historymodelitem.h"

abt_historyModelItem::abt_historyModelItem(const QList<QVariant> &data, abt_historyModelItem *parent)
{
	this->parentItem = parent;
	this->itemData = data;
}


abt_historyModelItem::~abt_historyModelItem()
{
	qDeleteAll(this->childItems);
}

void abt_historyModelItem::appendChild(abt_historyModelItem *item)
{
	this->childItems.append(item);
}

abt_historyModelItem *abt_historyModelItem::child(int row)
{
	return this->childItems.value(row);
}

int abt_historyModelItem::childCount() const
{
	return this->childItems.count();
}

int abt_historyModelItem::columnCount() const
{
	return this->itemData.count();
}

QVariant abt_historyModelItem::data(int column) const
{
	return this->itemData.value(column);
}

abt_historyModelItem *abt_historyModelItem::parent()
{
	return this->parentItem;
}

int abt_historyModelItem::row() const
{
	if (this->parentItem)
		return this->parentItem->childItems.indexOf(const_cast<abt_historyModelItem*>(this));

	return 0;
}
