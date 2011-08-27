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

#include <QDebug>
#include <QMessageBox>
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

	//Alle Buttons disablen, werden aktiviert wenn sie sinnvoll sind.
	this->ui->pushButton_del->setEnabled(false);
	this->ui->pushButton_down->setEnabled(false);
	this->ui->pushButton_up->setEnabled(false);
	this->ui->pushButton_exec->setEnabled(false);

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
	const QStringList *JobInfo;

	if (this->jobctrl->jobqueueList()->size() == 0) {
		this->ui->treeWidget->clear(); //Alle Items löschen
		this->ui->treeWidget->setColumnCount(1);
		this->ui->treeWidget->setHeaderHidden(true);
		this->ui->treeWidget->header()->setStretchLastSection(true);

		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("Keine Aufträge zum Ausführen vorhanden"));
		topItem->setFlags(Qt::ItemIsSelectable);
		ui->treeWidget->addTopLevelItem(topItem);
		//Ausführen BTN ausschalten, keine Aufträge vorhanden.
		this->ui->pushButton_exec->setEnabled(false);
		return; //fertig
	}

	//! \todo der Status des Items sollte erhalten bleiben (expanded/selected)
	this->ui->treeWidget->clear(); //erstmal alle löschen
	this->setDefaultTreeWidgetHeader(); //also sets the col widths

	for (int i=0; i<this->jobctrl->jobqueueList()->size(); ++i) {
		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("%1").arg(i+1));
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

		if (this->selectedItem == i) {
			//die vorherige Selection wieder herstellen
			topItem->setSelected(true);
			this->selectedItem = -1; //nur 1 Item auswählbar
		}

	}
	//Ausführen Button Enablen
	this->ui->pushButton_exec->setEnabled(true);

}


/** Up/Down/Del Buttons nur aktivieren wenn ein Item ausgewählt ist */
void Page_Ausgang::on_treeWidget_itemSelectionChanged()
{
	bool enabled = (this->ui->treeWidget->selectedItems().size() > 0);

	this->ui->pushButton_del->setEnabled(enabled);
	this->ui->pushButton_up->setEnabled(enabled);
	this->ui->pushButton_down->setEnabled(enabled);
}

/** Den aktuell ausgewählten Eintrag um 1 nach oben verschieben */
void Page_Ausgang::on_pushButton_up_clicked()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	 //damit in refreshTreeWidget() das Item wieder ausgewählt wird
	this->selectedItem = itemNr-1;
	this->jobctrl->moveJob(itemNr, -1);
}

/** Den aktuell ausgewählten Eintrag um 1 nach unten verschieben */
void Page_Ausgang::on_pushButton_down_clicked()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	//damit in refreshTreeWidget() das Item wieder ausgewählt wird
	this->selectedItem = itemNr+1;
	this->jobctrl->moveJob(itemNr, 1);
}

/** Den aktuell ausgewählten Eintrag löschen */
void Page_Ausgang::on_pushButton_del_clicked()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	QMessageBox msg;
	msg.setIcon(QMessageBox::Question);
	msg.setWindowTitle(tr("Aufrag entfernen"));
	msg.setText(tr("Soll der ausgewählte Auftrag wirklich gelöscht werden?"));
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::Yes);

	int ret = msg.exec();

	if (ret != QMessageBox::Yes) {
		return; //Abbruch
	}

	this->jobctrl->deleteJob(itemNr);
}

