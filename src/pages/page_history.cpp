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

#include <QMessageBox>

#include "../dialogs/abt_dialog.h"
#include "../abt_history.h"


page_history::page_history(const abt_history *history, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::page_history)
{
	ui->setupUi(this);

	this->history = history;
	this->createActions();

	this->setDefaultTreeWidgetHeader(); //also sets the col widths
	this->refreshTreeWidget(this->history);

	//calling the itemSelectionChanged() slot enables/disables the actions
	this->on_treeWidget_itemSelectionChanged();

	connect(this->history, SIGNAL(historyListChanged(const abt_history*)),
		this, SLOT(refreshTreeWidget(const abt_history*)));

}

page_history::~page_history()
{
	delete ui;
	delete this->actGenerateNewTransaction;
	delete this->actExportSelected;
	delete this->actDeleteSelected;
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

//private
void page_history::createActions()
{
	QIcon newIcon = this->ui->toolButton_new->icon();
	this->actGenerateNewTransaction = new QAction(newIcon, tr("Neu von Vorlage"), this);
	this->actGenerateNewTransaction->setToolTip(tr("Neuen Auftrag, mit den Daten "
						       "des gewählten Eintrags als "
						       "Vorlage, erstellen."));
	connect(this->actGenerateNewTransaction, SIGNAL(triggered()),
		this, SLOT(onActGenerateNewTransaction()));
	this->ui->toolButton_new->setDefaultAction(this->actGenerateNewTransaction);

	QIcon deleteIcon = this->ui->toolButton_delete->icon();
	this->actDeleteSelected = new QAction(deleteIcon, tr("Löschen"), this);
	this->actDeleteSelected->setToolTip(tr("Löscht die ausgewählten Einträge "
					       "aus der Historie."));
	connect(this->actDeleteSelected, SIGNAL(triggered()),
		this, SLOT(onActDeleteSelected()));
	this->ui->toolButton_delete->setDefaultAction(this->actDeleteSelected);

	QIcon exportIcon = this->ui->toolButton_export->icon();
	this->actExportSelected = new QAction(exportIcon, tr("Exportieren"), this);
	this->actExportSelected->setToolTip(tr("Exportiert die ausgewählten Einträge "
					       "für eine andere Anwendung."));
	connect(this->actExportSelected, SIGNAL(triggered()),
		this, SLOT(onActExportSelected()));
	this->ui->toolButton_export->setDefaultAction(this->actExportSelected);
}

//private slot
void page_history::onActGenerateNewTransaction()
{
	//only one history item is selected when calling this action!
	QVariant var = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole);

	emit createNewFromHistory(var.value<abt_jobInfo*>());
}

//private slot
void page_history::onActDeleteSelected()
{
	QList<QTreeWidgetItem*> items;
	QList<abt_jobInfo*> jiList;

	QString msgTitle = tr("Historie löschen");
	QString msgText = tr("Sollen die gewählten Einträge aus der Historie "
			     "gelöscht werden?<br />"
			     "<i>(Dies kann nicht rückgängig gemacht werden)</i>");
	abt_dialog dialog(this, msgTitle, msgText,
			  QDialogButtonBox::Yes | QDialogButtonBox::No,
			  QDialogButtonBox::Yes, QMessageBox::Question,
			  "historyDelete");
	int ret = dialog.exec();

	if (ret != QDialogButtonBox::Yes) {
		return;
	}

	items = this->ui->treeWidget->selectedItems();

	for(int i=0; i<items.size(); ++i) {
		QVariant var = items.at(i)->data(0, Qt::UserRole);
		jiList.append(var.value<abt_jobInfo*>());
	}

	emit deleteFromHistory(jiList);
}

//private slot
void page_history::onActExportSelected()
{
	QMessageBox::information(this, tr("Export"),
				 tr("Exportieren von durchgeführten Aufträgen "
				    "ist derzeit leider noch nicht möglich.<br />"
				    "<br />"
				    "Dies wird vorraussichtlich in der nächsten "
				    "Version enthalten sein."),
				 QMessageBox::Ok);
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

void page_history::on_treeWidget_itemSelectionChanged()
{
	bool enabled = (this->ui->treeWidget->selectedItems().size() > 0);
	bool oneSelected = (this->ui->treeWidget->selectedItems().size() == 1);

	this->actGenerateNewTransaction->setEnabled(enabled && oneSelected);
	this->actExportSelected->setEnabled(enabled);
	this->actDeleteSelected->setEnabled(enabled);
}
