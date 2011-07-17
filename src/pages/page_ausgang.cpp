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

#include "page_ausgang.h"
#include "ui_page_ausgang.h"

#include <QTreeWidgetItem>
#include "../abt_conv.h"
#include "../abt_settings.h"

Page_Ausgang::Page_Ausgang(abt_job_ctrl *jobctrl, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::Page_Ausgang)
{
	ui->setupUi(this);
	this->jobctrl = jobctrl;

	this->refreshTreeWidget();

	connect(ui->pushButton_exec, SIGNAL(clicked()),
		this->jobctrl, SLOT(execQueuedTransactions()));

	connect(this->jobctrl, SIGNAL(jobQueueListChanged()),
		this, SLOT(refreshTreeWidget()));
}

Page_Ausgang::~Page_Ausgang()
{
	delete ui;

	disconnect(ui->pushButton_exec, SIGNAL(clicked()),
		   this->jobctrl, SLOT(execQueuedTransactions()));
	disconnect(this->jobctrl, SIGNAL(jobQueueListChanged()),
		   this, SLOT(refreshTreeWidget()));
}

void Page_Ausgang::changeEvent(QEvent *e)
{
	QFrame::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void Page_Ausgang::resizeEvent(QResizeEvent *event)
{
	this->setTreeWidgetColWidths();
	QFrame::resizeEvent(event);
}

void Page_Ausgang::setDefaultTreeWidgetHeader()
{
	QStringList header;
	header  << tr("Nr.")
		<< tr("Typ")
		<< tr("Status");

	ui->treeWidget->setColumnCount(3);
	ui->treeWidget->setHeaderHidden(false);
	ui->treeWidget->header()->setStretchLastSection(false);
	ui->treeWidget->setHeaderLabels(header);
	this->setTreeWidgetColWidths();
}

void Page_Ausgang::setTreeWidgetColWidths()
{
	if (ui->treeWidget->header()->stretchLastSection())
		return; //nichts ändern, es werden keine Items angezeigt

	int currWidth = ui->treeWidget->width();
	ui->treeWidget->setColumnWidth(0,40);
	ui->treeWidget->setColumnWidth(2,100);
	ui->treeWidget->setColumnWidth(1,currWidth-146);
}

void Page_Ausgang::refreshTreeWidget()
{
	QTreeWidgetItem *item, *topItem;
	ui->treeWidget->clear();
	const QStringList *JobInfo;

	if (this->jobctrl->jobqueueList()->size() == 0) {
		this->ui->treeWidget->setColumnCount(1);
		this->ui->treeWidget->setHeaderHidden(true);
		this->ui->treeWidget->header()->setStretchLastSection(true);

		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("Keine Aufträge zum Ausführen vorhanden"));
		topItem->setFlags(Qt::ItemIsSelectable);
		ui->treeWidget->addTopLevelItem(topItem);
		return; //fertig
	}

	this->setDefaultTreeWidgetHeader(); //also sets the col widths

	for (int i=0; i<this->jobctrl->jobqueueList()->size(); ++i) {
		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("%1").arg(i));
		topItem->setData(1, Qt::DisplayRole, this->jobctrl->jobqueueList()->at(i)->getType());
		topItem->setData(2, Qt::DisplayRole, this->jobctrl->jobqueueList()->at(i)->getStatus());

		JobInfo = this->jobctrl->jobqueueList()->at(i)->getInfo();
		for (int j=0; j<JobInfo->size(); j++) {
			item = new QTreeWidgetItem();
			item->setData(0, Qt::DisplayRole, "");
			item->setData(1, Qt::DisplayRole, JobInfo->at(j));
			item->setFlags(Qt::ItemIsSelectable);
			topItem->addChild(item);
		}

		ui->treeWidget->addTopLevelItem(topItem);
	}

}

