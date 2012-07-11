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


page_history::page_history(const abt_history *history, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::page_history)
{
	ui->setupUi(this);

	this->history = history;

	this->setDefaultTreeWidgetHeader(); //also sets the col widths

	this->refreshTreeWidget(history);

}

page_history::~page_history()
{
	delete ui;
}

void page_history::changeEvent(QEvent *e)
{
	QFrame::changeEvent(e);
	switch(e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void page_history::resizeEvent(QResizeEvent *event)
{
	this->setTreeWidgetColWidths();
	QFrame::resizeEvent(event);
}

//private
void page_history::setTreeWidgetColWidths()
{
	if (ui->treeWidget->header()->stretchLastSection())
		return; //nichts ändern, es werden keine Items angezeigt

	int currWidth = ui->treeWidget->width();
	ui->treeWidget->setColumnWidth(0,40);
	ui->treeWidget->setColumnWidth(2,100);
	ui->treeWidget->setColumnWidth(1,currWidth-146);
}

//private
void page_history::setDefaultTreeWidgetHeader()
{
	QStringList header;
	header  << tr("Nr.")
		<< tr("Typ")
		<< tr("Datum");

	ui->treeWidget->setColumnCount(3);
	ui->treeWidget->setHeaderHidden(false);
	ui->treeWidget->header()->setStretchLastSection(false);
	ui->treeWidget->setHeaderLabels(header);
	this->setTreeWidgetColWidths();
}

//public slot
void page_history::refreshTreeWidget(const abt_history *hist)
{
	QTreeWidgetItem *item, *topItem;
	const QStringList *historyInfo;

	if ((hist == NULL) ||
	    (hist->getHistoryList()->size() == 0)) {
		this->ui->treeWidget->clear(); //Alle Items löschen
		this->ui->treeWidget->setColumnCount(1);
		this->ui->treeWidget->setHeaderHidden(true);
		this->ui->treeWidget->header()->setStretchLastSection(true);

		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("Keine Einträge in der Historie vorhanden"));
		topItem->setFlags(Qt::ItemIsSelectable);
		ui->treeWidget->addTopLevelItem(topItem);
		return; //fertig
	}

	// der Status des Items soll erhalten bleiben (expanded/selected)
	// alle expandierten abt_job_info Adressen in einer Liste speichern
	QList<const abt_jobInfo*> expanded;
	for (int i=0; i<this->ui->treeWidget->topLevelItemCount(); ++i) {
		if (this->ui->treeWidget->topLevelItem(i)->isExpanded()) {
			QVariant tliVar = this->ui->treeWidget->topLevelItem(i)->data(0, Qt::UserRole);
			expanded.append(tliVar.value<abt_jobInfo*>());
		}
	}

	this->ui->treeWidget->clear(); //alle Items löschen
	this->setDefaultTreeWidgetHeader(); //also sets the col widths

	const QList<abt_jobInfo*> *jql = hist->getHistoryList();
	for (int i=0; i<jql->size(); ++i) {
		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("",
							"first coloum in the History."
							"%1 could be used as the position").arg(i+1));
		topItem->setData(1, Qt::DisplayRole, jql->at(i)->getType());
		//the idForApplication is the unix timestamp of the creation
		quint32 ts = jql->at(i)->getTransaction()->getIdForApplication();
		topItem->setData(2, Qt::DisplayRole, QDateTime::fromTime_t(ts));

		//Die Adresse des abt_job_info Objects in der UserRole merken
		QVariant var;
		var.setValue(jql->at(i));
		topItem->setData(0, Qt::UserRole, var);

		historyInfo = jql->at(i)->getInfo();
		for (int j=0; j<historyInfo->size(); j++) {
			item = new QTreeWidgetItem();
			item->setData(0, Qt::DisplayRole, "");
			item->setData(1, Qt::DisplayRole, historyInfo->at(j));
			item->setFlags(Qt::ItemIsSelectable);
			topItem->addChild(item);
		}

		ui->treeWidget->addTopLevelItem(topItem);

		//den zustand wie vor dem refresh wieder herstellen
		if (expanded.contains(jql->at(i))) {
			topItem->setExpanded(true);
		}

//		if (this->selectedItem == i) {
//			//die vorherige Selection wieder herstellen
//			topItem->setSelected(true);
//			this->selectedItem = -1; //nur 1 Item auswählbar
//		}

	}
	//Ausführen Button Enablen
//	this->ui->pushButton_exec->setEnabled(true);
}

