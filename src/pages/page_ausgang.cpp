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
#include <QMenu>
#include "../abt_conv.h"
#include "../abt_settings.h"

Page_Ausgang::Page_Ausgang(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::Page_Ausgang)
{
	ui->setupUi(this);
	//this->jobctrl = jobctrl;
	this->createAllActions();

	this->refreshTreeWidget(NULL); //erstellt "Keine Aufträge vorhanden"

	this->ui->pushButton_exec->setEnabled(false);

	this->on_treeWidget_itemSelectionChanged(); //en-/disable actions and buttons

	connect(this->ui->pushButton_del, SIGNAL(clicked()),
		this->actDelete, SLOT(trigger()));
	connect(this->ui->pushButton_up, SIGNAL(clicked()),
		this->actUp, SLOT(trigger()));
	connect(this->ui->pushButton_down, SIGNAL(clicked()),
		this->actDown, SLOT(trigger()));

}

Page_Ausgang::~Page_Ausgang()
{
	delete ui;

	delete this->actDelete;
	delete this->actEdit;
	delete this->actDown;
	delete this->actUp;
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

//private
void Page_Ausgang::createAllActions()
{
	this->actDelete = new QAction(this);
	this->actDelete->setText(tr("Löschen"));
	this->actDelete->setToolTip(tr("Ausgewählten Job löschen"));
	this->actDelete->setIcon(QIcon::fromTheme("edit-delete"));
	connect(this->actDelete, SIGNAL(triggered()),
		this, SLOT(onActionDeleteTriggered()));

	this->actEdit = new QAction(this);
	this->actEdit->setText(tr("Bearbeiten"));
	this->actEdit->setToolTip(tr("Ausgewählten Job bearbeiten"));
	this->actEdit->setIcon(QIcon::fromTheme("document-edit"));
	connect(this->actEdit, SIGNAL(triggered()),
		this, SLOT(onActionEditTriggered()));

	this->actUp = new QAction(this);
	this->actUp->setText(tr("Auf"));
	this->actUp->setToolTip(tr("Ausgewählten Job nach oben verschieben"));
	this->actUp->setIcon(QIcon(":/icons/up"));
	connect(this->actUp, SIGNAL(triggered()),
		this, SLOT(onActionUpTriggered()));

	this->actDown = new QAction(this);
	this->actDown->setText(tr("Ab"));
	this->actDown->setToolTip(tr("Ausgewählten Job nach unten verschieben"));
	this->actDown->setIcon(QIcon(":/icons/down"));
	connect(this->actDown, SIGNAL(triggered()),
		this, SLOT(onActionDownTriggered()));

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

//public slot
void Page_Ausgang::refreshTreeWidget(const abt_job_ctrl *jobctrl)
{
	QTreeWidgetItem *item, *topItem;
	const QStringList *JobInfo;

	if ((jobctrl == NULL) ||
	    (jobctrl->jobqueueList()->size() == 0)) {
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

	const QList<abt_jobInfo*> *jql = jobctrl->jobqueueList();
	for (int i=0; i<jql->size(); ++i) {
		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, tr("%1").arg(i+1));
		topItem->setData(1, Qt::DisplayRole, jql->at(i)->getType());
		topItem->setData(2, Qt::DisplayRole, jql->at(i)->getStatus());
		//Die Adresse des abt_job_info Objects in der UserRole merken
		QVariant var;
		var.setValue(jql->at(i));
		topItem->setData(0, Qt::UserRole, var);

		JobInfo = jql->at(i)->getInfo();
		for (int j=0; j<JobInfo->size(); j++) {
			item = new QTreeWidgetItem();
			item->setData(0, Qt::DisplayRole, "");
			item->setData(1, Qt::DisplayRole, JobInfo->at(j));
			item->setFlags(Qt::ItemIsSelectable);
			topItem->addChild(item);
		}

		ui->treeWidget->addTopLevelItem(topItem);

		//den zustand wie vor dem refresh wieder herstellen
		if (expanded.contains(jql->at(i))) {
			topItem->setExpanded(true);
		}

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

	this->actUp->setEnabled(enabled);
	this->actDown->setEnabled(enabled);
	this->actEdit->setEnabled(enabled);
	this->actDelete->setEnabled(enabled);
}

/** Den aktuell ausgewählten Eintrag um 1 nach oben verschieben */
void Page_Ausgang::onActionUpTriggered()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	 //damit in refreshTreeWidget() das Item wieder ausgewählt wird
	this->selectedItem = itemNr-1;
	emit this->moveJobInList(itemNr, -1);
}

/** Den aktuell ausgewählten Eintrag um 1 nach unten verschieben */
void Page_Ausgang::onActionDownTriggered()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	//damit in refreshTreeWidget() das Item wieder ausgewählt wird
	this->selectedItem = itemNr+1;
	emit this->moveJobInList(itemNr, 1);
}

/** Den aktuell ausgewählten Eintrag löschen */
void Page_Ausgang::onActionDeleteTriggered()
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

	emit this->removeJob(itemNr);
}


/** Den aktuell ausgewählten Eintrag bearbeiten */
void Page_Ausgang::onActionEditTriggered()
{
	Q_ASSERT(this->ui->treeWidget->selectedItems().size() > 0);
	//repräsentiert auch gleichzeitig die Position in der jobqueliste
	int itemNr = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::DisplayRole).toInt()-1;

	QMessageBox msg;
	msg.setIcon(QMessageBox::Question);
	msg.setWindowTitle(tr("Aufrag Bearbeiten"));
	msg.setText(tr("Dies entfernt den ausgewählten Auftrag aus dem Ausgang "
		       "und öffnet ihn zum bearbeiten.<br />"
		       "Wenn der Auftrag beim bearbeiten nicht mit \"Senden\" "
		       "beendet wird, wird dieser nicht zur Bank gesendet!<br /><br />"
		       "Soll der Auftrag bearbeitet werden?"));
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::Yes);

	int ret = msg.exec();

	if (ret != QMessageBox::Yes) {
		return; //Abbruch
	}

	emit this->editJob(itemNr); //Job soll editiert werden
}

//static  private
bool Page_Ausgang::isJobTypeEditable(const AB_JOB_TYPE type)
{
	switch (type) {
	case AB_Job_TypeDeleteStandingOrder:
	case AB_Job_TypeDeleteDatedTransfer:
	case AB_Job_TypeGetBalance:
	case AB_Job_TypeGetDatedTransfers:
	case AB_Job_TypeGetStandingOrders:
	case AB_Job_TypeGetTransactions:
	case AB_Job_TypeUnknown:
		return false;
		break;
	default:
		return true;
	}
}

void Page_Ausgang::on_treeWidget_customContextMenuRequested(QPoint pos)
{
	//Context-Menu zum Ändern/Löschen/Rauf/runter anzeigen
	bool editable;
	bool disabled = this->ui->treeWidget->selectedItems().size() == 0;

	if (disabled) { //keine Items ausgewählt oder keine vorhanden
		editable = false;
	} else { //Items ausgewählt, somit auch vorhanden!
		//Die UserRole enthält die Adresse des Jobs
		const QVariant var = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole);
		if (var.canConvert<abt_jobInfo*>()) {
			const abt_jobInfo *job = var.value<abt_jobInfo*>();
			const AB_JOB_TYPE jobType = job->getAbJobType();
			editable = this->isJobTypeEditable(jobType);
		} else {
			editable = false;
		}
	}

	this->actEdit->setEnabled(editable);
	this->actDelete->setDisabled(disabled);
	this->actUp->setDisabled(disabled);
	this->actDown->setDisabled(disabled);

	QMenu *contextMenu = new QMenu();
	contextMenu->addAction(this->actUp);
	contextMenu->addAction(this->actDown);
	contextMenu->addAction(this->actEdit);
	contextMenu->addAction(this->actDelete);
	contextMenu->exec(this->ui->treeWidget->viewport()->mapToGlobal(pos));
}

void Page_Ausgang::on_pushButton_exec_clicked()
{
	emit this->executeClicked();
}
