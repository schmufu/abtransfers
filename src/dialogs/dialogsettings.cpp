/******************************************************************************
 * Copyright (C) 2012-2013 Patrick Wacker
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
 *	Dialog um die Einstellungen von AB-Transfers zu ändern.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "dialogsettings.h"
#include "ui_dialogsettings.h"

#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include "../abt_settings.h"
#include "../dialogs/abt_dialog.h"

#include "../globalvars.h"


DialogSettings::DialogSettings(abt_settings *settings, AB_BANKING *ab, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);
	Q_ASSERT(settings);

	this->settings = settings;
	this->imexp = new aqb_imexporters(ab); //loads all im-/exporters from aqbanking
	this->imex_favorites = new QHash<QString, bool>();

	//normaly we want the first tab active after start (everybody could use
	// the following public function to set the active tab after creation)
	this->setActiveTab(0);

	this->loadFromSettings();

	//ensure that the state of the checkboxes "refresh at start" are consistant
	this->onCheckBoxRefereshAtStartStateChanged(0);

	QIcon ico;
	ico = QIcon::fromTheme("document-new", QIcon(":/icons/document-new"));
	this->ui->actionNewProfile->setIcon(ico);
	ico = QIcon::fromTheme("document-edit", QIcon(":/icons/document-edit"));
	this->ui->actionEditProfile->setIcon(ico);
	ico = QIcon::fromTheme("edit-delete", QIcon(":/icons/delete"));
	this->ui->actionDeleteProfile->setIcon(ico);

	connect(this->ui->checkBox_getBalance, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));
	connect(this->ui->checkBox_getDatedTransfers, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));
	connect(this->ui->checkBox_getStandingOrders, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));

	connect(this->imexp, SIGNAL(imexportersLoaded()),
		this, SLOT(updatedImExporters()));


	//widgets on the imexporter tab-widget page
	this->ui->tableWidget_profiles->addAction(this->ui->actionNewProfile);
	this->ui->tableWidget_profiles->addAction(this->ui->actionEditProfile);
	this->ui->tableWidget_profiles->addAction(this->ui->actionDeleteProfile);

	//set optimal column withs for the tableWidget
	//this->ui->tableWidget_profiles->resizeColumnsToContents();
	this->ui->tableWidget_profiles->setColumnWidth(0, 112); //name
	this->ui->tableWidget_profiles->setColumnWidth(1, 296); //description
	this->ui->tableWidget_profiles->setColumnWidth(2, 28); //import
	this->ui->tableWidget_profiles->setColumnWidth(3, 28); //export
	this->ui->tableWidget_profiles->setColumnWidth(4, 28); //global
	this->ui->tableWidget_profiles->setColumnWidth(5, 34); //favorit
	this->ui->tableWidget_profiles->setColumnWidth(6, 56); //Version

	//at initialisation select the first item in the listwidget
	if (this->ui->listWidget_plugins->count() > 0)
		this->ui->listWidget_plugins->setCurrentRow(0);

}

DialogSettings::~DialogSettings()
{
	this->imex_favorites->clear();
	delete this->imex_favorites;
	delete this->imexp;
	delete ui;
}

void DialogSettings::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

//private
void DialogSettings::loadFromSettings()
{
	this->ui->lineEdit_dataDir->setText(this->settings->getDataDir());
	this->ui->lineEdit_accountData->setText(this->settings->getAccountDataFilename());
	this->ui->lineEdit_history->setText(this->settings->getHistoryFilename());
	this->ui->lineEdit_recipients->setText(this->settings->getRecipientsFilename());

	this->ui->checkBox_warnCosts->setChecked(this->settings->showDialog("WarnCosts"));
	this->ui->checkBox_jobAddedToOutput->setChecked(this->settings->showDialog("JobAddOutput"));
	this->ui->checkBox_warnDeleteProfile->setChecked(this->settings->showDialog("ProfileConfirmDelete"));
	this->ui->checkBox_warnDeleteHistory->setChecked(this->settings->showDialog("HistoryConfirmDelete"));
	this->ui->checkBox_runtimeLanguageChange->setChecked(this->settings->showDialog("RuntimeLanguageChange"));

	this->ui->checkBox_getBalance->setChecked(this->settings->appendJobToOutbox("getBalance"));
	this->ui->checkBox_getStandingOrders->setChecked(this->settings->appendJobToOutbox("getStandingOrders"));
	this->ui->checkBox_getDatedTransfers->setChecked(this->settings->appendJobToOutbox("getDatedTransfers"));
	this->ui->checkBox_executeAtStart->setChecked(this->settings->appendJobToOutbox("executeAtStart"));

	this->ui->checkBox_adv_manualOutboxRearrange->setChecked(
			this->settings->isAdvancedOptionSet("ManualOutboxRearrange"));

	this->loadFavoriteImExpFromSettings();

	this->refreshImExPluginListWidget();
	this->refreshAutoExportComboBoxPlugin();

	this->ui->checkBox_autoAddNewRecipients->setChecked(this->settings->autoAddNewRecipients());
	this->ui->checkBox_autoExport->setChecked(this->settings->autoExportEnabled());
	this->ui->lineEdit_autoExportFilename->setText(this->settings->autoExportFilename());

	//update enable/disabled state of corresponding widgets
	this->on_checkBox_autoExport_toggled(this->ui->checkBox_autoExport->isChecked());

	//select previous selected plugin and profile entries
	QString iexpName = this->settings->autoExportPluginName();
	for (int i=0; i<this->ui->comboBox_plugin->count(); ++i) {
		if (this->ui->comboBox_plugin->itemText(i) == iexpName) {
			this->ui->comboBox_plugin->setCurrentIndex(i);
			break;
		}
	}

	iexpName = this->settings->autoExportProfileName();
	for (int i=0; i<this->ui->comboBox_profile->count(); ++i) {
		if (this->ui->comboBox_profile->itemText(i) == iexpName) {
			this->ui->comboBox_profile->setCurrentIndex(i);
			break;
		}
	}

}

//private
void DialogSettings::saveToSettings()
{
	this->settings->setDataDir(this->ui->lineEdit_dataDir->text());
	this->settings->setAccountDataFilename(this->ui->lineEdit_accountData->text());
	this->settings->setHistoryFilename(this->ui->lineEdit_history->text());
	this->settings->setRecipientsFilename(this->ui->lineEdit_recipients->text());

	this->settings->setShowDialog("WarnCosts", this->ui->checkBox_warnCosts->isChecked());
	this->settings->setShowDialog("JobAddOutput", this->ui->checkBox_jobAddedToOutput->isChecked());
	this->settings->setShowDialog("ProfileConfirmDelete", this->ui->checkBox_warnDeleteProfile->isChecked());
	this->settings->setShowDialog("HistoryConfirmDelete", this->ui->checkBox_warnDeleteHistory->isChecked());
	this->settings->setShowDialog("RuntimeLanguageChange", this->ui->checkBox_runtimeLanguageChange->isChecked());

	this->settings->setAppendJobToOutbox("getBalance", this->ui->checkBox_getBalance->isChecked());
	this->settings->setAppendJobToOutbox("getStandingOrders", this->ui->checkBox_getStandingOrders->isChecked());
	this->settings->setAppendJobToOutbox("getDatedTransfers", this->ui->checkBox_getDatedTransfers->isChecked());
	this->settings->setAppendJobToOutbox("executeAtStart", this->ui->checkBox_executeAtStart->isChecked());

	this->settings->setAdvancedOption("ManualOutboxRearrange",
			this->ui->checkBox_adv_manualOutboxRearrange->isChecked());

	this->settings->setAutoAddNewRecipients(this->ui->checkBox_autoAddNewRecipients->isChecked());

	this->settings->setAutoExportEnabled(this->ui->checkBox_autoExport->isChecked());
	this->settings->setAutoExportPluginName(this->ui->comboBox_plugin->currentText());
	this->settings->setAutoExportProfileName(this->ui->comboBox_profile->currentText());
	this->settings->setAutoExportFilename(this->ui->lineEdit_autoExportFilename->text());

	this->saveFavoriteImExpToSettings();

}

void DialogSettings::loadFavoriteImExpFromSettings()
{
	//load all as favorite marked im-/exort profiles
	this->imex_favorites->clear();

	foreach(const QString key, this->settings->getAllProfileFavorites()) {
		this->imex_favorites->insert(key, this->settings->isProfileFavorit(key));
	}
}


void DialogSettings::saveFavoriteImExpToSettings()
{
	//save all as favorit marked im-/export profiles
	QHashIterator<QString, bool> i(*this->imex_favorites);
	while (i.hasNext()) {
		i.next();
		this->settings->setProfileFavorit(i.key(), i.value());
	}
}

/**
 * @brief reloads the supported Im-/Exporters from AqBanking
 *
 * This calls the reload at @ref aqb_imexporters which emits a signal when
 * all data is loaded which calls our connected slot @ref updatedImExporters().
 */
//private
void DialogSettings::reloadImExporters()
{
	//reload all data
	this->imexp->reloadImExporterData();
}

/**
 * @brief refreshes all items in the listWidget for the Plugins
 *
 * The selection is remembered and retored after all items are removed and
 * newly added to the list.
 */
//private
void DialogSettings::refreshImExPluginListWidget()
{
	Q_ASSERT(this->imexp);

	//remember the selected item
	QString pluginName = "";
	int row = this->ui->listWidget_plugins->currentRow();
	if (row != -1) {
		pluginName = this->ui->listWidget_plugins->item(row)->text();
	}

	this->ui->listWidget_plugins->clear(); //remove all;

	//add all supported im-/exporter plugins
	foreach(const aqb_iePlugin *plugin, *this->imexp->getPlugins()) {
		this->ui->listWidget_plugins->addItem(plugin->getName());
	}

	//restore selected item
	for (int i=0; i<this->ui->listWidget_plugins->count(); ++i) {
		if (this->ui->listWidget_plugins->item(i)->text() == pluginName) {
			this->ui->listWidget_plugins->setCurrentRow(i);
			break; //nothing else can match
		}
	}

}

//private
void DialogSettings::refreshImExProfileTableWidget()
{
	const aqb_iePlugin *plugin = NULL;
	int row = this->ui->listWidget_plugins->currentRow();
	QString pluginName = "";
	if (row >= 0)
		pluginName = this->ui->listWidget_plugins->item(row)->text();

	this->ui->tableWidget_profiles->clearContents();
	this->ui->tableWidget_profiles->setRowCount(0);

	plugin = this->imexp->getPluginByName(pluginName);
	if (!plugin) {
		return; //no plugin exists, so no profiles available
	}


	//get all profiles for the selected plugin an add them to the tableWidget
	foreach(const aqb_ieProfile *profile, *plugin->getProfiles()) {
		QTableWidgetItem *item;
		Qt::CheckState checkState;

		//we need a new row
		int rowc = this->ui->tableWidget_profiles->rowCount();
		this->ui->tableWidget_profiles->setRowCount(rowc + 1);
		//"rowc" is now the last row in the table

		item = new QTableWidgetItem();
		item->setText(profile->getValue("name").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(rowc, 0, item);

		bool selected = this->selection.value(pluginName, "") == item->text();
		//qDebug() << "ROW:" << rowc << "profile" << item->text() << selected;

		item = new QTableWidgetItem();
		item->setText(profile->getValue("shortDescr").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(rowc, 1, item);

		//import possible?
		if (profile->getValue("import").isValid() &&
		    profile->getValue("import").toBool()) {
			checkState = Qt::Checked;
		} else {
			checkState = Qt::Unchecked;
		}
		item = new QTableWidgetItem();
		item->setCheckState(checkState); //import state
		item->setFlags(Qt::ItemIsSelectable);
		this->ui->tableWidget_profiles->setItem(rowc, 2, item);

		//export possible?
		if (profile->getValue("export").isValid() &&
		    profile->getValue("export").toBool()) {
			checkState = Qt::Checked;
		} else {
			checkState = Qt::Unchecked;
		}
		item = new QTableWidgetItem();
		item->setCheckState(checkState); //export state
		item->setFlags(Qt::ItemIsSelectable);
		this->ui->tableWidget_profiles->setItem(rowc, 3, item);

		//global profile?
		if (profile->getValue("isGlobal").isValid() &&
		    profile->getValue("isGlobal").toBool()) {
			checkState = Qt::Checked;
		} else {
			checkState = Qt::Unchecked;
		}
		item = new QTableWidgetItem();
		item->setCheckState(checkState); //global state
		item->setFlags(Qt::ItemIsSelectable);
		this->ui->tableWidget_profiles->setItem(rowc, 4, item);

		//if the profile is a favorite this is stored in the private QHash
		QString key = pluginName;
		key.append("/");
		key.append(profile->getValue("name").toString());
		if (this->imex_favorites->value(key, false)) {
			checkState = Qt::Checked;
		} else {
			checkState = Qt::Unchecked;
		}
		item = new QTableWidgetItem();
		item->setCheckState(checkState);
		this->ui->tableWidget_profiles->setItem(rowc, 5, item);

		item = new QTableWidgetItem();
		item->setText(profile->getValue("version").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(rowc, 6, item);

		//restore selection
		if (selected) {
			this->ui->tableWidget_profiles->selectRow(rowc);
		}

	}

	//if nothing is selected, we select the first row, if present
	if (this->ui->tableWidget_profiles->selectedItems().size() == 0 &&
	    this->ui->tableWidget_profiles->rowCount() > 0) {
		this->ui->tableWidget_profiles->selectRow(0);
	}

	//now we have a selected profile, if any profiles available
	//scroll to the selected item
	if (this->ui->tableWidget_profiles->selectedItems().size() != 0) {
		QTableWidgetItem *item;
		item = this->ui->tableWidget_profiles->selectedItems().at(0);
		this->ui->tableWidget_profiles->scrollToItem(item);
	}


}

//private
/** \brief updates the entries in the combo box for automatic export
 *
 * This function must only be called when the imexp (Im-/Export) object
 * has loaded all plugins and profiles (this is the case when the slot
 * updatedImExporters() was called or the object was just created).
 */
void DialogSettings::refreshAutoExportComboBoxPlugin()
{
	Q_ASSERT(this->imexp);
	const QString currText = this->ui->comboBox_plugin->currentText();
	int exportsAvailable;
	int idx = 0;

	this->ui->comboBox_plugin->clear();
	foreach(const aqb_iePlugin *plugin, *this->imexp->getPlugins()) {
		exportsAvailable = false;
		foreach(aqb_ieProfile *profile, *plugin->getProfiles()) {
			//the plugin must contain a profile useable for export
			if (profile->getNames()->contains("export") &&
			    profile->getValue("export").toInt() == 1) {
				exportsAvailable = true;
				break;
			}
		}

		if (exportsAvailable) {
			const QString pluginName = plugin->getName();
			const QString pluginHint = plugin->getDescShort();
			this->ui->comboBox_plugin->addItem(pluginName);
			this->ui->comboBox_plugin->setItemData(this->ui->comboBox_plugin->count()-1,
							       pluginHint,
							       Qt::ToolTipRole);
			if (currText == pluginName) {
				idx = this->ui->comboBox_plugin->count() - 1;
			}
		}
	}

	//restore previous selected entry
	this->ui->comboBox_plugin->setCurrentIndex(idx);
}

//private
bool DialogSettings::getSelectedPluginAndProfile(const aqb_iePlugin **plugin,
						 const aqb_ieProfile **profile) const
{
	Q_ASSERT(plugin);
	Q_ASSERT(profile);

	const aqb_iePlugin *selPlugin = NULL;
	const aqb_ieProfile *selProfile = NULL;

	QString pluginName;
	int listWidgetRow = this->ui->listWidget_plugins->currentRow();
	if (listWidgetRow >= 0) {
		pluginName = this->ui->listWidget_plugins->item(listWidgetRow)->text();
		selPlugin = this->imexp->getPluginByName(pluginName);
	}

	if (!selPlugin) {
		//we did not found a plugin, we set the pointer to NULL
		//and return false
		*plugin = NULL;
		*profile = NULL; //no plugin -> no profile
		return false;
	}

	//we got a selected plugin and set the pointer in the supplied var
	*plugin = selPlugin;


	//we have the plugin now we need the corresponding selected profile
	QString profileName;
	int curRow = this->ui->tableWidget_profiles->currentRow();
	if (curRow >= 0) {
		//at col 0 is the name of the profile
		profileName = this->ui->tableWidget_profiles->item(curRow, 0)->text();
		foreach(const aqb_ieProfile *pro, *selPlugin->getProfiles()) {
			if (pro->getValue("name").toString() == profileName) {
				selProfile = pro;
				break; //profile found, another cant match
			}
		} //foreach
	} //curRow >= 0

	if (!selProfile) {
		//we did not found a profile
		*profile = NULL;
		return false;
	}

	//we got a selected profile and set the pointer in the supplied var
	*profile = selProfile;

	return (selPlugin != NULL && selProfile != NULL);

}

void DialogSettings::setActiveTab(int tabId)
{
	if (this->ui->tabWidget->count() <= tabId) {
		return; //tab does not exists, abort
	}

	this->ui->tabWidget->setCurrentIndex(tabId);
}

//private slot
/**
* @brief this slot is called when the @ref aqb_imexporters emits @ref imexportersLoaded();
*
* The refresh also changes the currentRow and emits the signal
* currentRowChanged() which refreshes the tableWidget_profiles
*
* In addition the combo box entries for the automatic export are updated.
*/
void DialogSettings::updatedImExporters()
{
	//we refresh the ListWidget for the plugins
	this->refreshImExPluginListWidget();
	this->refreshAutoExportComboBoxPlugin();
}

//private slot
void DialogSettings::on_buttonBox_clicked(QAbstractButton* button)
{
	switch(this->ui->buttonBox->standardButton(button)) {
	case QDialogButtonBox::RestoreDefaults:
	case QDialogButtonBox::Reset:
		this->loadFromSettings();
		break;
	case QDialogButtonBox::Save:
	case QDialogButtonBox::SaveAll:
		this->saveToSettings();
		break;
	case QDialogButtonBox::Cancel:
		this->reject();
		break;
	case QDialogButtonBox::Ok:
		this->saveToSettings();
		this->accept();
		break;

	default:
		//not handled, so nothing to do
		break;
	}
}

//private slot
void DialogSettings::on_toolButton_selectDataDir_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this,
							      tr("Standard-Ordner"),
							      this->ui->lineEdit_dataDir->text(),
							      QFileDialog::ShowDirsOnly);

	QDir dir(directory);
	if (dir.exists() && !directory.isEmpty()) {
		this->ui->lineEdit_dataDir->setText(directory);
	}
}

//private slot
void DialogSettings::on_toolButton_selectAccountData_clicked()
{
	QString file = QFileDialog::getSaveFileName(this,
						    tr("Aktuelle Daten Speichern in ..."),
						    this->ui->lineEdit_dataDir->text(),
						    tr("Context-Dateien (*.ctx);;Alle Dateien (*.*)"),
						    NULL,
						    QFileDialog::DontConfirmOverwrite);

	if (!file.isEmpty() || QFile::exists(file)) {
		this->ui->lineEdit_accountData->setText(file);
	}
}

//private slot
void DialogSettings::on_toolButton_selectHistory_clicked()
{
	QString file = QFileDialog::getSaveFileName(this,
						    tr("Historie Speichern in ..."),
						    this->ui->lineEdit_dataDir->text(),
						    tr("Context-Dateien (*.ctx);;Alle Dateien (*.*)"),
						    NULL,
						    QFileDialog::DontConfirmOverwrite);

	if (!file.isEmpty() || QFile::exists(file)) {
		this->ui->lineEdit_history->setText(file);
	}
}

//private slot
void DialogSettings::on_toolButton_selectRecipients_clicked()
{
	QString file = QFileDialog::getSaveFileName(this,
						    tr("Bekannte Empfänger Speichern in ..."),
						    this->ui->lineEdit_dataDir->text(),
						    tr("Text-Dateien (*.txt);;Alle Dateien (*.*)"),
						    NULL,
						    QFileDialog::DontConfirmOverwrite);

	if (!file.isEmpty() || QFile::exists(file)) {
		this->ui->lineEdit_recipients->setText(file);
	}
}

//private slot
void DialogSettings::on_toolButton_selectAutoExportFilename_clicked()
{
	QString file = QFileDialog::getSaveFileName(this,
						    tr("Automatischen Export Speichern in ..."),
						    this->ui->lineEdit_dataDir->text(),
						    tr("Alle Dateien (*.*)"),
						    NULL);

	if (!file.isEmpty() || QFile::exists(file)) {
		this->ui->lineEdit_autoExportFilename->setText(file);
	}
}


/**
 * @brief sets the states (enabled/disabled) of the refreshAtStart checkbox
 *
 * The checkbox refreshAtStart must only be selectable when one of the other
 * checkboxes are checked. If no one is checked, the refreshAtStart checkbox
 * is unchecked and disabled.
 */
//private slot
void DialogSettings::onCheckBoxRefereshAtStartStateChanged(int /* state */)
{
	//is one ore more checkboxes checked
	if (this->ui->checkBox_getBalance->isChecked() ||
	    this->ui->checkBox_getDatedTransfers->isChecked() ||
	    this->ui->checkBox_getStandingOrders->isChecked()) {
		//executeAtStart enabled
		this->ui->checkBox_executeAtStart->setEnabled(true);
	} else {
		//otherwise deaktivated and unchecked
		this->ui->checkBox_executeAtStart->setEnabled(false);
		this->ui->checkBox_executeAtStart->setChecked(false);
	}
}


void DialogSettings::on_listWidget_plugins_currentRowChanged(int currentRow)
{
	qDebug() << Q_FUNC_INFO << "current row:" << currentRow;
	//at first we disable the actions, they are activated depending
	//on the selection in the tableWidget_profiles
	this->ui->actionDeleteProfile->setEnabled(false);
	this->ui->actionEditProfile->setEnabled(false);
	this->ui->actionNewProfile->setEnabled(false);

	//then we refresh all profiles for the current selected plugin
	this->refreshImExProfileTableWidget();
}

void DialogSettings::on_tableWidget_profiles_itemChanged(QTableWidgetItem *item)
{
	if (item->column() != 5) {
		return; //Favorit not changed
	}

	//qDebug() << Q_FUNC_INFO << "item" << item << "changed! Selected:" << item->checkState();

	//favorit changed, store the actual Value
	bool checked = item->checkState() == Qt::Checked;

	//get the plugin name for the changed profile item
	QString key; // in the form of "pluginname/profilename"
	int listWidgetRow = this->ui->listWidget_plugins->currentRow();
	key = this->ui->listWidget_plugins->item(listWidgetRow)->text(); //pluginname
	key.append("/");
	key.append(this->ui->tableWidget_profiles->item(item->row(), 0)->text()); //profilename

	this->imex_favorites->insert(key, checked);
}

void DialogSettings::on_tableWidget_profiles_itemSelectionChanged()
{
	QTableWidget *profilesWidget = this->ui->tableWidget_profiles;
	if (profilesWidget->selectedItems().size() == 0) {
		return; //nothing selected
	}

	QString pluginName;
	int row = this->ui->listWidget_plugins->currentRow();
	pluginName = this->ui->listWidget_plugins->item(row)->text();

	//AqBanking remains the owner of 'ie', so we must not free it!
	AB_IMEXPORTER *ie = AB_Banking_GetImExporter(banking->getAqBanking(),
						     pluginName.toUtf8());

	bool enabled;
	enabled = (AB_ImExporter_GetFlags(ie) & AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED);
	//enable the actions if an editor is supported by AqBanking
	this->ui->actionNewProfile->setEnabled(enabled);
	this->ui->actionEditProfile->setEnabled(enabled);

	row = profilesWidget->selectedItems().at(0)->row();
	if (profilesWidget->item(row, 4) != NULL) {
		//enabled if not a global profile
		enabled = profilesWidget->item(row, 4)->checkState() == Qt::Unchecked;
	} else {
		enabled = false;
	}
	this->ui->actionDeleteProfile->setEnabled(enabled);

	//remember the selection for refresh
	if (profilesWidget->item(row, 0) != NULL) {
		QString profileName = profilesWidget->item(row, 0)->text();
		this->selection.insert(pluginName, profileName);
	}
}


void DialogSettings::on_actionEditProfile_triggered()
{
	const aqb_iePlugin *plugin = NULL;
	const aqb_ieProfile *profile = NULL;
	bool ok = false;

	ok = this->getSelectedPluginAndProfile(&plugin, &profile);

	if (!ok) {
		qWarning() << Q_FUNC_INFO << "no plugin or no profile found. Aborting.";
		return;
	}

	QString profileName = profile->getValue("name").toString();

	if (profile->getValue("isGlobal").isValid() &&
	    profile->getValue("isGlobal").toBool()) {
		QString msg = tr("Sie möchten das Profil \"%1\" ändern, dieses "
				 "Profil ist ein 'globales' Profil. Wenn von "
				 "Ihnen der Name (\"%1\") nicht geändert wird, "
				 "wird das dann als lokal gespeicherte Profil "
				 "das globale <i>überdecken</i>!<br />"
				 "Sie können dann also nur noch auf das geänderte "
				 "Profil zugreifen und nicht mehr auf die "
				 "ursprüngliche Version!<br />"
				 "<i>Nach dem löschen des lokalen Profils würde "
				 "das globale wieder verwendet werden können.</i>")
			      .arg(profileName);

		QMessageBox::information(this, tr("Globales Profil ändern"), msg);

		//the folder for the new local profile must exists, otherwise the
		//profile could not be saved by aqbanking
		QDir lclImexpDir;
		lclImexpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks |
				      QDir::NoDotAndDotDot);

		QString path = banking->getUserDataDir();

		path.append("/imexporters");
		path.append("/").append(plugin->getName());
		path.append("/profiles/");
		lclImexpDir.setPath(path);

		if (!lclImexpDir.exists()) {
			//the directory does not exist, we create it
			bool dirOK;
			dirOK = lclImexpDir.mkpath(QDir::toNativeSeparators(path));
			if (!dirOK) {
				//we only produce a warning, maybe the
				//editProfileWithAqbDialog() could create it
				qWarning() << Q_FUNC_INFO << "could not create"
					   << "the directory for local profiles:"
					   << QDir::toNativeSeparators(path);
			}
			lclImexpDir.setPath(path);
			if (!lclImexpDir.exists()) {
				qWarning() << Q_FUNC_INFO << "directory does not exists"
					   << "after creation, something is wrong!";
			}
		}

	}

	GWEN_DB_NODE *dbProfile = AB_Banking_GetImExporterProfile(banking->getAqBanking(),
								  plugin->getName(),
								  profileName.toUtf8());

	QString filename = profile->getValue("fileName").toString();

	int ret = this->imexp->editProfileWithAqbDialog(dbProfile,
							plugin->getName(),
							filename.toUtf8());

	if (ret < 0) {
		//something went wrong
		QMessageBox::critical(this, tr("Profil Ändern"),
				      tr("Beim Ändern des Profils %1 ist ein "
					 "unerwarteter Fehler aufgetreten.<br />"
					 "In den Debug-Ausgaben können evt. "
					 "weitere nützliche Informationen "
					 "enthalten sein.").arg(profileName));
	}

	//im-/export profiles might be changed, reaload them
	this->reloadImExporters();

}

//private slot
void DialogSettings::on_actionNewProfile_triggered()
{
	//at first we need a new name for the profile
	QString newname;
	bool inputOk = false;

	newname = QInputDialog::getText(this, tr("Profil Name"),
					tr("Bitte geben sie einen Namen für "
					   "das neue Profil ein"),
					QLineEdit::Normal, "", &inputOk);

	if (!inputOk || newname.isEmpty()) {
		//no name was given or cancel clicked
		return;
	}

	//get the current plugin and check if it already contains a profile
	//with the entered name
	const aqb_iePlugin *selPlugin = NULL;
	const aqb_ieProfile *selProfile = NULL;
	bool profileExists = false;

	this->getSelectedPluginAndProfile(&selPlugin, &selProfile);
	//the selProfile is not necessary
	if (!selPlugin) {
		qWarning() << Q_FUNC_INFO << "cant get the pointer to the"
			   << "current plugin, aborting.";
		return;
	}

	foreach(const aqb_ieProfile *pro, *selPlugin->getProfiles()) {
		if (pro->getValue("name").toString() == newname) {
			profileExists = true;
			break; //one profile found, another must not match
		}
	}

	if (profileExists) {
		QMessageBox::critical(this, tr("Profil Name"),
				      tr("Ein Profil mit dem Namen %1 existiert "
					 "bereits innerhalb des Plugins '%2'."
					 "<br /><br />"
					 "Ein neues Profil mit demselben Namen "
					 "kann nicht erstellt werden!")
				      .arg(newname).arg(selPlugin->getName()));
		return;
	}


	//OK, we have a name which does not exists so we can create the new
	//profile.

	GWEN_DB_NODE *dbProfile = GWEN_DB_Group_new(newname.toUtf8());
	//we set the name of the new Profile
	GWEN_DB_SetCharValue(dbProfile, GWEN_DB_FLAGS_DEFAULT, "name",
			     newname.toUtf8());

	QString filename = newname;
	filename.append(".conf");

	//the folder for the new local profile must exists, otherwise the
	//profile could not be saved by aqbanking
	QDir lclImexpDir;
	lclImexpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks |
			      QDir::NoDotAndDotDot);

	QString path = banking->getUserDataDir();

	path.append("/imexporters");
	path.append("/").append(selPlugin->getName());
	path.append("/profiles/");
	lclImexpDir.setPath(path);

	if (!lclImexpDir.exists()) {
		//the directory does not exist, we create it
		bool dirOK;
		dirOK = lclImexpDir.mkpath(QDir::toNativeSeparators(path));
		if (!dirOK) {
			//we only produce a warning, maybe the
			//editProfileWithAqbDialog() could create it
			qWarning() << Q_FUNC_INFO << "could not create"
				   << "the directory for local profiles:"
				   << QDir::toNativeSeparators(path);
		}
		lclImexpDir.setPath(path);
		if (!lclImexpDir.exists()) {
			qWarning() << Q_FUNC_INFO << "directory does not exists"
				   << "after creation, something is wrong!";
		}
	}

	int ret = this->imexp->editProfileWithAqbDialog(dbProfile,
							selPlugin->getName(),
							filename.toUtf8());

	if (ret < 0) {
		//something went wrong
		QMessageBox::critical(this, tr("Profil Anlegen"),
				      tr("Beim Anlegen des Profils %1 ist ein "
					 "unerwarteter Fehler aufgetreten.<br />"
					 "In den Debug-Ausgaben können evt. "
					 "weitere nützliche Informationen "
					 "enthalten sein.").arg(newname));
	}

	//profile were saved, free the DB_NODE
	GWEN_DB_Group_free(dbProfile);

	//im-/export profiles might be changed, reaload them
	this->reloadImExporters();

}

void DialogSettings::on_actionDeleteProfile_triggered()
{
	//get the selected plugin and profile and check if the file could be
	//deleted.
	const aqb_iePlugin *selPlugin = NULL;
	const aqb_ieProfile *selProfile = NULL;
	bool ok = false;

	ok = this->getSelectedPluginAndProfile(&selPlugin, &selProfile);

	if (!ok) {
		qWarning() << Q_FUNC_INFO << "something went wrong on getting"
			   << "the selected profile (" << selProfile << ")"
			   << "and plugin (" << selPlugin << ") - Aborting.";
		return;
	}

	//we got the selected plugin and profile

	//check if the profile is a global profile which should not be deleted
	if (selProfile->getValue("isGlobal").isValid() &&
	    selProfile->getValue("isGlobal").toBool()) {
		qWarning() << Q_FUNC_INFO << "the selected profile"
			   << selProfile->getValue("name").toString() << "is"
			   << "global and could not be deleted!";
		return;
	}

	//get the filename and directory
	QDir lclImexpDir;
	lclImexpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks |
			      QDir::NoDotAndDotDot);

	QString path = banking->getUserDataDir();

	path.append("/imexporters");
	path.append("/").append(selPlugin->getName());
	path.append("/profiles/");
	lclImexpDir.setPath(path);

	if (!lclImexpDir.exists()) {
		qWarning() << Q_FUNC_INFO << "directory does not exists -"
			   << lclImexpDir.path() << "- Aborting.";
		return;
	}

	//check if the filename exists in the directory and delete it if the
	//user want it.
	QString profileFilename = selProfile->getValue("fileName").toString();
	if (lclImexpDir.entryList().contains(profileFilename)) {
		//abt_dialog, so the user can decide to not get asked again
		abt_dialog delDia(this, tr("Profil löschen"),
				  tr("Soll das Profil %1 wirklich gelöscht "
				     "werden?").arg(selProfile->getValue("name").toString()),
				  QDialogButtonBox::Yes | QDialogButtonBox::No,
				  QDialogButtonBox::Yes, QMessageBox::Question,
				  "ProfileConfirmDelete");
		if (delDia.exec() == QDialogButtonBox::Yes) {
			//remember the key for settings before deletion
			QString key = selPlugin->getName();
			key.append("/");
			key.append(selProfile->getValue("name").toString());

			QString file = lclImexpDir.absolutePath();
			file.append("/").append(profileFilename);
			QFile::remove(file);
			qDebug() << Q_FUNC_INFO << "file" << file << "deleted!";

			//also remove the favorite settings
			this->imex_favorites->remove(key);
			this->settings->deleteProfileFavorit(key);
		}
	}

	//im-/export profiles might be changed, reaload them
	this->reloadImExporters();

}

//private slot
/** \brief reloads the profiles for the corresponding plugin.
 *
 * When the plugin selection changed, this function refresh the combobox
 * for the profiles of the newly selected plugin.
 */
void DialogSettings::on_comboBox_plugin_currentIndexChanged(const QString &arg1)
{
	Q_ASSERT(this->imexp);
	int idx = -1;
	const QString currText = this->ui->comboBox_profile->currentText();
	const QList<aqb_ieProfile*> *profiles = NULL;

	if (this->imexp->getPluginByName(arg1) != NULL) {
		profiles = this->imexp->getPluginByName(arg1)->getProfiles();
	} else {
		//no profiles available
		this->ui->comboBox_profile->clear();
		return;
	}

	this->ui->comboBox_profile->clear();
	foreach(aqb_ieProfile *profile, *profiles) {
		//export possible?
		if (profile->getValue("export").isValid() &&
		    profile->getValue("export").toBool()) {
			const QString itemName = profile->getValue("name").toString();
			const QString itemHint = profile->getValue("shortDescr").toString();

			this->ui->comboBox_profile->addItem(itemName);
			this->ui->comboBox_profile->setItemData(this->ui->comboBox_profile->count()-1,
								itemHint, Qt::ToolTipRole);
			if (currText == itemName) {
				idx = this->ui->comboBox_profile->count() - 1;
			}
		}
	}

	//restore selected entry, set it to "default" (if possible) or set it
	//to the first entry
CURRENT_INDEX_CHANGED_SET_INDEX:
	if (idx != -1) {
		//idx is possible, use it
		this->ui->comboBox_profile->setCurrentIndex(idx);
		return;
	} else {
		for (int i=0; i<this->ui->comboBox_profile->count(); ++i) {
			if (this->ui->comboBox_profile->itemText(i) == "default") {
				idx = i;
				//default idx found, use it
				goto CURRENT_INDEX_CHANGED_SET_INDEX;
			}
		}
	}

	//set first entry, no used and no default entry was found
	if (this->ui->comboBox_profile->count() >= 1) {
		this->ui->comboBox_profile->setCurrentIndex(0);
	}
}

//private slot
/** \brief enables or disables corresponding widgets depending on \a checked state.
 */
void DialogSettings::on_checkBox_autoExport_toggled(bool checked)
{
	this->ui->lineEdit_autoExportFilename->setEnabled(checked);
	this->ui->comboBox_plugin->setEnabled(checked);
	this->ui->comboBox_profile->setEnabled(checked);
	this->ui->toolButton_selectAutoExportFilename->setEnabled(checked);
	this->ui->label_8->setEnabled(checked);
	this->ui->label_9->setEnabled(checked);
	this->ui->label_10->setEnabled(checked);
}
