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


#include "abt_historymodel.h"

#include "abt_history.h"
#include "abt_historymodelitem.h"
#include "abt_jobinfo.h"

#include "abt_conv.h"

abt_historyModel::abt_historyModel(const abt_history &data, QObject *parent)
	: QAbstractItemModel(parent),
	  m_data(data)
{

	this->setupRootItem();

	this->setupModelData(&this->m_data, this->rootItem);

	connect(&data, SIGNAL(historyListChanged(const abt_history*)),
		this, SLOT(updateModelData(const abt_history*)));
}

abt_historyModel::~abt_historyModel()
{
	delete this->rootItem;
}

//private
void abt_historyModel::setupRootItem()
{
	QList<QVariant> headerData;
	headerData << tr("Datum")
		   << tr("Typ")
		   << tr("Status")
		   << tr("Betrag");
		   //<< tr("Verwendungszweck");

	this->rootItem = new abt_historyModelItem(headerData);
}


int abt_historyModel::columnCount(const QModelIndex &parent) const
{
//	if (parent.internalPointer() && parent.isValid())
	if (parent.isValid())
		return static_cast<abt_historyModelItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant abt_historyModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	abt_historyModelItem *item = static_cast<abt_historyModelItem*>(index.internalPointer());

	return item->data(index.column());
}

Qt::ItemFlags abt_historyModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	//nur die "ChildItems" sind auswählbar! Nicht die childs der childs!
	if (index.parent() == QModelIndex())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	return 0;
}

QVariant abt_historyModel::headerData(int section, Qt::Orientation orientation,
				      int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex abt_historyModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	abt_historyModelItem *parentItem;

//	if (parent.internalPointer() && parent.isValid())
	if (parent.isValid())
		parentItem = static_cast<abt_historyModelItem*>(parent.internalPointer());
	else
		parentItem = rootItem;

	abt_historyModelItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex abt_historyModel::parent(const QModelIndex &index) const
{
//	if (!index.internalPointer() || !index.isValid())
	if (!index.isValid())
		return QModelIndex();

	abt_historyModelItem *childItem = static_cast<abt_historyModelItem*>(index.internalPointer());
	abt_historyModelItem *parentItem = childItem->parent();

	if (!parentItem)
		return QModelIndex();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int abt_historyModel::rowCount(const QModelIndex &parent) const
{
	abt_historyModelItem *parentItem;
	if (parent.column() > 0)
		return 0;

//	if (parent.internalPointer() && parent.isValid())
	if (parent.isValid())
		parentItem = static_cast<abt_historyModelItem*>(parent.internalPointer());
	else
		parentItem = rootItem;

	if (!parentItem)
		parentItem = rootItem;

	return parentItem->childCount();
}

void abt_historyModel::setupModelData(const abt_history *data, abt_historyModelItem *parent)
{
	foreach(const abt_jobInfo *ji, *data->getHistoryList()) {
		QList<QVariant> colData;
		colData << ji->getTransaction()->getRemoteName().at(0);
		colData << ji->getTransaction()->getDate().toString(Qt::SystemLocaleShortDate);
		colData << ji->getType();
		colData << ji->getStatus();
		colData << abt_conv::ABValueToString(ji->getTransaction()->getValue(), true);

		const QStringList purpose = ji->getTransaction()->getPurpose();
		QString purposeStr;
		if (purpose.size() >= 1)
			purposeStr = ji->getTransaction()->getPurpose().at(0);
		if (purpose.size() >= 2)
			purposeStr.append(ji->getTransaction()->getPurpose().at(1));

		colData << purposeStr;


		abt_historyModelItem *childItem = new abt_historyModelItem(colData, parent);
		parent->appendChild(childItem);

		QList<QVariant> colData2;
		for(int i=0; i<ji->getInfo()->size(); ++i) {
			colData2 << ji->getInfo()->at(i);
		}

		foreach(QVariant var, colData2) {
			QList<QVariant> list;
			list << QString() << var;
			abt_historyModelItem *childItem2 = new abt_historyModelItem(list, childItem);
			childItem->appendChild(childItem2);
		}
	}

#if false
    while (number < lines.count()) {
	int position = 0;
	while (position < lines[number].length()) {
	    if (lines[number].mid(position, 1) != " ")
		break;
	    position++;
	}

	QString lineData = lines[number].mid(position).trimmed();

	if (!lineData.isEmpty()) {
	    // Read the column data from the rest of the line.
	    QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
	    QList<QVariant> columnData;
	    for (int column = 0; column < columnStrings.count(); ++column)
		columnData << columnStrings[column];

	    if (position > indentations.last()) {
		// The last child of the current parent is now the new parent
		// unless the current parent has no children.

		if (parents.last()->childCount() > 0) {
		    parents << parents.last()->child(parents.last()->childCount()-1);
		    indentations << position;
		}
	    } else {
		while (position < indentations.last() && parents.count() > 0) {
		    parents.pop_back();
		    indentations.pop_back();
		}
	    }

	    // Append a new item to the current parent's list of children.
	    parents.last()->appendChild(new TreeItem(columnData, parents.last()));
	}

	number++;
    }
#endif

}

void abt_historyModel::updateModelData(const abt_history *data)
{
	delete this->rootItem;

	this->setupRootItem();

	this->setupModelData(data, this->rootItem);
}
