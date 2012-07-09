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


#include "page_history.h"
#include "ui_page_history.h"

#include "../abt_history.h"

#include "../widgets/historyitemwidget.h"

#include "../abt_historymodel.h"
#include "../abt_historydelegate.h"

page_history::page_history(const abt_history *history, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::page_history)
{
	ui->setupUi(this);


	this->ui->scrollAreaWidgetContents->setBackgroundRole(QPalette::Base);

	QVBoxLayout *l = new QVBoxLayout(this->ui->scrollAreaWidgetContents);

	foreach(abt_jobInfo *ji, *history->getHistoryList()) {
		historyItemWidget *hiw = new historyItemWidget(NULL, ji);
		l->addWidget(hiw);
	}

	QSpacerItem *vspacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

	l->addSpacerItem(vspacer);

	//this->ui->scrollAreaWidgetContents->setLayout(l);




	//this->historyModel = new abt_historyModel(*history, this->ui->tableView);
	this->historyModel = new abt_historyModel(*history, this->ui->listView);

	abt_historyDelegate *historyDelegate = new abt_historyDelegate(this);

//	this->ui->tableView->setModel(this->historyModel);
//	this->ui->tableView->setItemDelegate(historyDelegate);

	this->ui->listView->setModel(this->historyModel);
	this->ui->listView->setItemDelegate(historyDelegate);


	//this->ui->tableView_2->setModel(this->historyModel);

//	this->ui->tableView->setHeaderHidden(false);

//	QTreeWidgetItem *item = new QTreeWidgetItem();
//	historyItemWidget *hiw = new historyItemWidget(this->ui->treeWidget, history->getHistoryList()->at(0));
//	QVariant var;
//	var.setValue(hiw);
//	item->setData(0, Qt::DisplayRole, var);

//	this->ui->treeWidget->setItemWidget(item, 0, NULL);


//	this->ui->tableView->resizeColumnsToContents();
//	this->ui->tableView->resizeRowsToContents();
}

page_history::~page_history()
{
//	delete this->historyModel;
	delete ui;
}
