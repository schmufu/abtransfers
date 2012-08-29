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
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtCore/QDebug>

#include "../dialogs/abt_dialog.h"
#include "../abt_history.h"

#include "../globalvars.h"

#include <aqbanking/abgui.h>
#include <aqbanking/dlg_importer.h>

#include "../aqb_imexporters.h"


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
		return; //do nothing! No items shown.

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
	this->ui->treeWidget->addAction(this->actGenerateNewTransaction);

	QIcon exportIcon = this->ui->toolButton_export->icon();
	this->actExportSelected = new QAction(exportIcon, tr("Exportieren"), this);
	this->actExportSelected->setToolTip(tr("Exportiert die ausgewählten Einträge "
					       "für eine andere Anwendung."));
	connect(this->actExportSelected, SIGNAL(triggered()),
		this, SLOT(onActExportSelected()));
	this->ui->toolButton_export->setDefaultAction(this->actExportSelected);
	this->ui->treeWidget->addAction(this->actExportSelected);

	QIcon deleteIcon = this->ui->toolButton_delete->icon();
	this->actDeleteSelected = new QAction(deleteIcon, tr("Löschen"), this);
	this->actDeleteSelected->setToolTip(tr("Löscht die ausgewählten Einträge "
					       "aus der Historie."));
	connect(this->actDeleteSelected, SIGNAL(triggered()),
		this, SLOT(onActDeleteSelected()));
	this->ui->toolButton_delete->setDefaultAction(this->actDeleteSelected);
	this->ui->treeWidget->addAction(this->actDeleteSelected);
}

AB_IMEXPORTER_CONTEXT *page_history::getContextFromSelected() const
{
	if (this->ui->treeWidget->selectedItems().size() == 0)
		return NULL; //Nothing selected

	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();


	foreach(const QTreeWidgetItem *item, this->ui->treeWidget->selectedItems()) {
		/**
		 * @todo We store everything as a transaction, this should be
		 *	 tunable at the settings.
		 */

		//the Qt::UserRole contains a QVariant with the adress of the
		//abt_jobInfo object from the history.
		const abt_jobInfo *job = item->data(0, Qt::UserRole).value<abt_jobInfo*>();

		AB_TRANSACTION *t = NULL;

		switch(job->getAbJobType()) {
		case AB_Job_TypeCreateDatedTransfer:
		case AB_Job_TypeModifyDatedTransfer:
		case AB_Job_TypeDeleteDatedTransfer:
			//append a dated transfer to the ctx
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			//AB_ImExporterContext_AddDatedTransfer(ctx, t);
			AB_ImExporterContext_AddTransaction(ctx, t);
			break;
		case AB_Job_TypeCreateStandingOrder:
		case AB_Job_TypeModifyStandingOrder:
		case AB_Job_TypeDeleteStandingOrder:
			//append a standing order to the ctx
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			//AB_ImExporterContext_AddStandingOrder(ctx, t);
			AB_ImExporterContext_AddTransaction(ctx, t);
			break;
		case AB_Job_TypeTransfer:
		case AB_Job_TypeEuTransfer:
		case AB_Job_TypeInternalTransfer:
		case AB_Job_TypeLoadCellPhone:
		case AB_Job_TypeSepaDebitNote:
		case AB_Job_TypeDebitNote:
		case AB_Job_TypeSepaTransfer:
			//appent a transfer to the ctx
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			//AB_ImExporterContext_AddTransfer(ctx, t);
			AB_ImExporterContext_AddTransaction(ctx, t);
			break;

		default: //other JobTypes not possible, yet
			break;
		}

	}

	//now all selected history items are in the created context
	return ctx;
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

	QString msgTitle = tr("Historie Eintrag löschen");
	QString msgText = tr("Sollen die gewählten Einträge aus der Historie wirklich "
			     "gelöscht werden?<br />"
			     "<i>(Dies kann nicht rückgängig gemacht werden)</i>");
	abt_dialog dialog(this, msgTitle, msgText,
			  QDialogButtonBox::Yes | QDialogButtonBox::No,
			  QDialogButtonBox::Yes, QMessageBox::Question,
			  "HistoryConfirmDelete");
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
	/**
	 * @todo This code should be splitted into some functions and cleaned
	 *	 up for a release!
	 *	 The usage of "goto" should be removed too.
	 */

	aqb_imexporters* iep = new aqb_imexporters(banking->getAqBanking());

	qDebug() << Q_FUNC_INFO << "ImExporters loaded:" << iep->getSize();

	QMenu *exportConextMenu = new QMenu();

	QMenu *exportPreferred = new QMenu(tr("bevorzugt"), exportConextMenu);
	exportPreferred->setToolTip(tr("Änderbar in den Einstellungen"));

	exportConextMenu->addMenu(exportPreferred);
	exportConextMenu->addSeparator();

	for (int i=0; i<iep->getPlugins()->size(); ++i) {
		//we create a submenu for every plugin
		QMenu *sub = new QMenu(exportConextMenu);
		aqb_iePlugin *plugin = iep->getPlugins()->at(i);

		sub->setTitle(plugin->getName());
		sub->setToolTip(plugin->getDescShort());

		for (int j=0; j<plugin->getProfiles()->size(); ++j) {
			//add every profile that belongs to the plugin
			aqb_ieProfile *profile = plugin->getProfiles()->at(j);

			//if this is not an export-profile, we cant use it.
			//Therefore, export must exist and must be unequal 0
			if (!profile->getNames()->contains("export") ||
			    profile->getValue("export").toInt() == 0) {
				continue; //next profile
			}

			QAction *item = new QAction(sub);
			//we store the pointer to the plugin and profile in the
			//actions user data, so we can use it later.
			item->setUserData(0, reinterpret_cast<QObjectUserData*>(plugin));
			item->setUserData(1, reinterpret_cast<QObjectUserData*>(profile));

			item->setText(profile->getValue("name").toString());
			item->setToolTip(profile->getValue("shortDescr").toString());

			//check if the profile is favorite and add it to the
			//preferred submenu with the plugin-name as prefix
			QString key = plugin->getName();
			key.append("/");
			key.append(profile->getValue("name").toString());
			if (settings->isProfileFavorit(key)) {
				QAction *prefItem = new QAction(exportPreferred);
				QString itemText = item->text();
				itemText.prepend(" - ");
				itemText.prepend(plugin->getName());
				prefItem->setText(itemText);
				prefItem->setUserData(0, reinterpret_cast<QObjectUserData*>(plugin));
				prefItem->setUserData(1, reinterpret_cast<QObjectUserData*>(profile));
				exportPreferred->addAction(prefItem);
			}

			sub->addAction(item);

		}

		if (sub->actions().size() == 0) {
			//the submenu has no entrys, we dont need it.
			delete sub;
		} else {
			exportConextMenu->addMenu(sub);
		}
	}

	QAction *sel = exportConextMenu->exec(QCursor::pos());

	if (sel) {
		//we stored the pointer to plugin and profile in the user-data,
		//lets restore them
		aqb_iePlugin *selPlugin = reinterpret_cast<aqb_iePlugin*>(sel->userData(0));
		aqb_ieProfile *selProfile = reinterpret_cast<aqb_ieProfile*>(sel->userData(1));

		if (selPlugin && selProfile) {
			qDebug() << Q_FUNC_INFO << "selected Plugin :" << selPlugin->getName();
			qDebug() << Q_FUNC_INFO << "selected Profile:" << selProfile->getValue("name");
		} else {
			qWarning() << Q_FUNC_INFO << "something went wrong!"
				   << "could not restore the pointers to Plugin or Profile!";
			goto ONACTEXPORTSELECTED_CLEANUP;
		}

		//Now we need to know where to store the data
		QString dialogTitle = tr("(%1 / %2) Exportieren ...").arg(
					      selPlugin->getName()).arg(
					      selProfile->getValue("name").toString());

		QFileDialog dialog(this);
		dialog.setModal(true);
		dialog.setWindowTitle(dialogTitle);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setNameFilter(tr("Alle Dateien (*.*)"));
		dialog.setNameFilterDetailsVisible(true);
		dialog.setViewMode(QFileDialog::Detail);
		dialog.setAcceptMode(QFileDialog::AcceptSave);

		QString saveFilename = "";
		if (dialog.exec()) {
			saveFilename = dialog.selectedFiles().at(0);
		} else {
			goto ONACTEXPORTSELECTED_CLEANUP;
		}

		if (saveFilename.isEmpty()) {
			QMessageBox::warning(this, tr("Export abgebrochen"),
					     tr("Es wurde kein Dateiname angegeben "
						"unter den die Exportierten Daten "
						"gespeichert werden sollen!"),
					     QMessageBox::Ok);
		} else {
			//create a ctx from the selected items in the history
			//and save this data with the selected profile from the
			//plugin.
			//Saving is done by a function from AqBanking

			//create a context for export from all selected items
			AB_IMEXPORTER_CONTEXT *ctx = this->getContextFromSelected();


			int err = AB_Banking_ExportToFile(banking->getAqBanking(),
							  ctx,
							  selPlugin->getName(),
							  selProfile->getValue("name").toString().toStdString().c_str(),
							  saveFilename.toStdString().c_str());

			//we must free the ctx after using it
			AB_ImExporterContext_free(ctx);

			if (err != 0) {
				QMessageBox::critical(this, tr("Export fehlerhaft"),
						      tr("Beim export trat ein Fehler auf!</ br>"
							 "Bitte kontrollieren Sie die exportierten "
							 "Daten und wiederholen ggf. den Vorgang!"),
						      QMessageBox::Ok);
			} else {
				QMessageBox::information(this, tr("Export erfolgreich"),
							 tr("Die ausgewählten Einträge "
							    "der Historie wurden unter "
							    "%1 gespeichert.").arg(saveFilename),
							 QMessageBox::Ok);
			}
		}

	}

	ONACTEXPORTSELECTED_CLEANUP:

	qDebug() << "deleting exportContextMenu";
	delete exportConextMenu; //Menu and all childs no longer needed

	qDebug() << "deleting aqb_imexporters (iep)";
	delete iep; //also deletes ALL childs objects from aqb_imexporters!

}


/** \brief creates all history items in the treeWidget
 *
 * This slot can be called to delete all items in the treeWidget and recreate
 * them.
 * The items that are expanded are memorized and if the ever exist the will be
 * expanded after the new creation.
 *
 * If no history items available a message is shown that no items exist.
 */
//public slot
void page_history::refreshTreeWidget(const abt_history *hist)
{
	QTreeWidgetItem *topItem;

	if ((hist == NULL) ||
	    (hist->getHistoryList()->size() == 0)) {
		this->ui->treeWidget->clear(); //delete all items
		this->ui->treeWidget->setColumnCount(1);
		this->ui->treeWidget->setHeaderHidden(true);
		this->ui->treeWidget->header()->setStretchLastSection(true);

		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole,
				 tr("Keine Einträge in der Historie vorhanden"));
		topItem->setFlags(Qt::NoItemFlags);
		ui->treeWidget->addTopLevelItem(topItem);
		return; //fertig
	}

	// the current state of the item should be recoverd (expanded/selected).
	// Saving all adresses of abt_jobInfo* in the expanded list.
	QList<const abt_jobInfo*> expanded;
	for (int i=0; i<this->ui->treeWidget->topLevelItemCount(); ++i) {
		if (this->ui->treeWidget->topLevelItem(i)->isExpanded()) {
			QVariant tliVar = this->ui->treeWidget->topLevelItem(i)->data(0, Qt::UserRole);
			expanded.append(tliVar.value<abt_jobInfo*>());
		}
	}

	this->ui->treeWidget->clear(); //delete all items
	this->setDefaultTreeWidgetHeader(); //also sets the col widths

	const QList<abt_jobInfo*> *jql = hist->getHistoryList();
	for (int i=0; i<jql->size(); ++i) {
		QTreeWidgetItem *item;
		const QStringList *historyInfo;

		topItem = new QTreeWidgetItem();
		topItem->setData(0, Qt::DisplayRole, QString("")); // first col empty
		topItem->setData(1, Qt::DisplayRole, jql->at(i)->getType());
		//the idForApplication is the unix timestamp of the creation
		quint32 ts = jql->at(i)->getTransaction()->getIdForApplication();
		topItem->setData(2, Qt::DisplayRole, QDateTime::fromTime_t(ts));

		//store the address of the abt_job_info Objects in the UserRoles
		QVariant var;
		var.setValue(jql->at(i));
		topItem->setData(0, Qt::UserRole, var);

		historyInfo = jql->at(i)->getInfo();
		for (int j=0; j<historyInfo->size(); j++) {
			item = new QTreeWidgetItem();
			item->setData(0, Qt::DisplayRole, "");
			item->setData(1, Qt::DisplayRole, historyInfo->at(j));
			item->setFlags(Qt::NoItemFlags);
			topItem->addChild(item);
		}

		ui->treeWidget->addTopLevelItem(topItem);

		//recover the state of the item
		if (expanded.contains(jql->at(i))) {
			topItem->setExpanded(true);
		}

	}
}

//private slot
void page_history::on_treeWidget_itemSelectionChanged()
{
	bool enabled = (this->ui->treeWidget->selectedItems().size() > 0);
	bool oneSelected = (this->ui->treeWidget->selectedItems().size() == 1);

	this->actGenerateNewTransaction->setEnabled(enabled && oneSelected);
	this->actExportSelected->setEnabled(enabled);
	this->actDeleteSelected->setEnabled(enabled);
}

void page_history::on_treeWidget_itemClicked(QTreeWidgetItem *item, int /* column */)
{
	//if the clicked item isnt selectable, switch the selection of the parent
	if (!(item->flags() & Qt::ItemIsSelectable)) {
		QTreeWidgetItem *top = item->parent();
		top->setSelected(!top->isSelected());
	}
}
