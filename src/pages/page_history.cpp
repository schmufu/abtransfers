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
	QMessageBox::information(
		this, tr("Export"),
		tr("Exportieren von durchgeführten Aufträgen ist derzeit leider "
		   "noch nicht möglich.<br /><br />"
		   "Dies wird vorraussichtlich in der nächsten Version "
		   "enthalten sein."),
		QMessageBox::Ok);

	//for testing
	aqb_imexporters* iep = new aqb_imexporters();

	qDebug() << Q_FUNC_INFO << "ImExporters loaded:" << iep->getSize();

	for(int i=0; i<iep->getPlugins()->size(); ++i) {
		qDebug() << Q_FUNC_INFO
			 << "Profile:"
			 << iep->getPlugins()->at(i)->getAuthor() << "-"
			 << iep->getPlugins()->at(i)->getName() << "-"
			 << iep->getPlugins()->at(i)->getType() << "-"
			 << iep->getPlugins()->at(i)->getDescShort();

		for(int j=0; j<iep->getPlugins()->at(i)->getProfiles()->size(); ++j) {
			aqb_ieProfile *pro = iep->getPlugins()->at(i)->getProfiles()->at(j);
			qDebug() << Q_FUNC_INFO
				 << *pro->getNames()
				 << "-- name:" << pro->getValue("name");
//				 << "-- global:" << pro->getValue("isGlobal");

		}
	}


	delete iep;


#ifdef false
	//get the list of available im/exporters (we must free this)
	GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl = NULL;
	pdl = AB_Banking_GetImExporterDescrs(banking->getAqBanking());

	int cnt = GWEN_PluginDescription_List2_GetSize(pdl);
	qDebug() << Q_FUNC_INFO << "read" << cnt << "Plugin descriptions";

	GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pdli;
	GWEN_PLUGIN_DESCRIPTION *pd = NULL;
	pdli = GWEN_PluginDescription_List2_First(pdl);
	if (pdli) {
		pd = GWEN_PluginDescription_List2Iterator_Data(pdli);
	}
	int i = 0;
	while (pd) {
		const char *name = GWEN_PluginDescription_GetName(pd);
		QString type = GWEN_PluginDescription_GetType(pd);
		QString desc_s = GWEN_PluginDescription_GetShortDescr(pd);
		QString desc_l = GWEN_PluginDescription_GetLongDescr(pd);
		QString filename = GWEN_PluginDescription_GetFileName(pd);
		QString path = GWEN_PluginDescription_GetPath(pd);

		qDebug() << ++i << ": " << name << type << desc_s << filename << path;
		//qDebug() << i << ": " << desc_l;

		//for testing lets see whats inside the profile
		GWEN_DB_NODE *dbProfiles = AB_Banking_GetImExporterProfiles(banking->getAqBanking(), name);
		GWEN_DB_NODE *n = GWEN_DB_GetFirstGroup(dbProfiles);
		while (n) {
			const char *n_name = GWEN_DB_GetCharValue(n, "name", 0, "notSet");
			const char *n_file = GWEN_DB_GetCharValue(n, "fileName", 0, "notSet");
			int n_global = GWEN_DB_GetIntValue(n, "isGlobal", 0, 2);
			qDebug() << n_name << n_file << n_global;

			GWEN_DB_NODE *nvars = GWEN_DB_GetFirstVar(n);
			i = 0;
			while (nvars) {
				const char *varname = GWEN_DB_VariableName(nvars);
				GWEN_DB_NODE_TYPE vartype = GWEN_DB_GetVariableType(n, varname);

				QString value;
				switch(vartype) {
				case GWEN_DB_NodeType_ValueChar:
					value = GWEN_DB_GetCharValue(n, varname, 0, "nothing");
					break;
				case GWEN_DB_NodeType_ValueInt:
					value = QString("int %1").arg(GWEN_DB_GetIntValue(n, varname, 0, -9));
					break;
				default:
					value = "Not handled!";
					break;
				}

				qDebug() << ++i << ": " << vartype << varname << "=" << value;

				nvars = GWEN_DB_GetNextVar(nvars);
			}

			n = GWEN_DB_GetNextGroup(n);
		}
		GWEN_DB_Group_free(dbProfiles);

		pd = GWEN_PluginDescription_List2Iterator_Next(pdli); //next in list
	} /* while (pd) */

	//free ListIterator
	GWEN_PluginDescription_List2Iterator_free(pdli);



	GWEN_PluginDescription_List2_freeAll(pdl);

//	//AqBanking remains the owner of 'ie', so we must not free it!
//	AB_IMEXPORTER *ie = AB_Banking_GetImExporter(banking->getAqBanking(), "csv");

//	GWEN_DB_NODE *dbProfile = AB_Banking_GetImExporterProfiles(banking->getAqBanking(), "csv");
//	GWEN_DIALOG *pDlg;

//	int ret = AB_ImExporter_GetEditProfileDialog(ie, dbProfile, "test", &pDlg);

//	uint32_t guiid = GWEN_Dialog_GetGuiId(pDlg);
//	GWEN_Gui_ExecDialog(pDlg, guiid);

//	GWEN_Dialog_free(pDlg);

//	qDebug() << Q_FUNC_INFO << "return value:" << ret;


//	AB_ImExporter_GetEditProfileDialog(ie, )
#endif

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
