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
 *	Dialog um die Einstellungen von AB-Transfers zu ändern.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "dialogsettings.h"
#include "ui_dialogsettings.h"

#include <QtCore/QDebug>
#include <QtGui/QFileDialog>

#include "../abt_settings.h"

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


	this->loadFromSettings();

	//sicherstellen das der Status der CheckBoxen "Aktualisieren beim Start"
	//konsistent zueinander ist.
	this->onCheckBoxRefereshAtStartStateChanged(0);


	connect(this->ui->checkBox_getBalance, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));
	connect(this->ui->checkBox_getDatedTransfers, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));
	connect(this->ui->checkBox_getStandingOrders, SIGNAL(stateChanged(int)),
		this, SLOT(onCheckBoxRefereshAtStartStateChanged(int)));


	//widgets on the imexporter tab-widget page
	this->ui->tableWidget_profiles->addAction(this->ui->actionEditProfile);
}

DialogSettings::~DialogSettings()
{
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

	this->ui->checkBox_getBalance->setChecked(this->settings->appendJobToOutbox("getBalance"));
	this->ui->checkBox_getStandingOrders->setChecked(this->settings->appendJobToOutbox("getStandingOrders"));
	this->ui->checkBox_getDatedTransfers->setChecked(this->settings->appendJobToOutbox("getDatedTransfers"));
	this->ui->checkBox_executeAtStart->setChecked(this->settings->appendJobToOutbox("executeAtStart"));

	this->ui->checkBox_autoAddNewRecipients->setChecked(this->settings->autoAddNewRecipients());

	this->refreshImExPluginWidget();
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

	this->settings->setAppendJobToOutbox("getBalance", this->ui->checkBox_getBalance->isChecked());
	this->settings->setAppendJobToOutbox("getStandingOrders", this->ui->checkBox_getStandingOrders->isChecked());
	this->settings->setAppendJobToOutbox("getDatedTransfers", this->ui->checkBox_getDatedTransfers->isChecked());
	this->settings->setAppendJobToOutbox("executeAtStart", this->ui->checkBox_executeAtStart->isChecked());

	this->settings->setAutoAddNewRecipients(this->ui->checkBox_autoAddNewRecipients->isChecked());

	//save all as favorit marked im-/export profiles
	QHashIterator<QString, bool> i(*this->imex_favorites);
	while (i.hasNext()) {
		i.next();
		this->settings->setProfileFavorit(i.key(), i.value());
	}
	this->imex_favorites->clear(); //all changes saves

}

void DialogSettings::refreshImExPluginWidget()
{
	Q_ASSERT(this->imexp);

	//remember the selected item
	int selected = this->ui->listWidget_plugins->currentRow();
	if (selected == -1) {
		selected = 0;
	}

	this->ui->listWidget_plugins->clear(); //remove all;

	//add all supported im-/exporter plugins
	foreach(const aqb_iePlugin *plugin, *this->imexp->getPlugins()) {
		this->ui->listWidget_plugins->addItem(plugin->getName());
	}

	//restore selected item
	if (this->ui->listWidget_plugins->count() > selected) {
		this->ui->listWidget_plugins->setCurrentRow(selected);
	}

}

//private
void DialogSettings::refreshImExProfileTableWidget()
{
	const aqb_iePlugin *curPlugin;
	int curRow = this->ui->listWidget_plugins->currentRow();
	QString curSelectionName = this->ui->listWidget_plugins->item(curRow)->text();
	curPlugin = this->imexp->getPluginByName(curSelectionName);

	if (!curPlugin) {
		qWarning() << Q_FUNC_INFO << "No im-/exporter-plugin for"
			   << curSelectionName << "found. Aborting.";
		return;
	}

	this->ui->tableWidget_profiles->clearContents();
	this->ui->tableWidget_profiles->setRowCount(0);

	//get all profiles for the selected plugin an add them to the tableWidget
	foreach(const aqb_ieProfile *profile, *curPlugin->getProfiles()) {
		QTableWidgetItem *item;
		Qt::CheckState checkState;

		//we need a new row
		int row = this->ui->tableWidget_profiles->rowCount();
		this->ui->tableWidget_profiles->setRowCount(row + 1);
		//"row" is now the last row in the table

		item = new QTableWidgetItem();
		item->setText(profile->getValue("name").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(row, 0, item);

		item = new QTableWidgetItem();
		item->setText(profile->getValue("shortDescr").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(row, 1, item);

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
		this->ui->tableWidget_profiles->setItem(row, 2, item);

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
		this->ui->tableWidget_profiles->setItem(row, 3, item);

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
		this->ui->tableWidget_profiles->setItem(row, 4, item);

		//Favorit is stored in the local settings object
		QString key = QString(curPlugin->getName());
		key.append("/");
		key.append(profile->getValue("name").toString());
		if (this->settings->isProfileFavorit(key) ||
		    this->imex_favorites->value(key, false)) {
			checkState = Qt::Checked;
		} else {
			checkState = Qt::Unchecked;
		}
		item = new QTableWidgetItem();
		item->setCheckState(checkState);
		this->ui->tableWidget_profiles->setItem(row, 5, item);

		item = new QTableWidgetItem();
		item->setText(profile->getValue("version").toString());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		this->ui->tableWidget_profiles->setItem(row, 6, item);

	}

	//set optimal column withs
	//this->ui->tableWidget_profiles->resizeColumnsToContents();
	this->ui->tableWidget_profiles->setColumnWidth(0, 112); //name
	this->ui->tableWidget_profiles->setColumnWidth(1, 296); //description
	this->ui->tableWidget_profiles->setColumnWidth(2, 28); //import
	this->ui->tableWidget_profiles->setColumnWidth(3, 28); //export
	this->ui->tableWidget_profiles->setColumnWidth(4, 28); //global
	this->ui->tableWidget_profiles->setColumnWidth(5, 34); //favorit
	this->ui->tableWidget_profiles->setColumnWidth(6, 56); //Version

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
							      tr("Standart-Ordner"),
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
void DialogSettings::onCheckBoxRefereshAtStartStateChanged(int /* state */)
{
	//Die checkbox executeAtStart darf nur auswählbar sein wenn auch
	//mindestens eine andere ausgewählt ist

	//wenn eine der drei Checkboxen ausgewählt ist
	if (this->ui->checkBox_getBalance->isChecked() ||
	    this->ui->checkBox_getDatedTransfers->isChecked() ||
	    this->ui->checkBox_getStandingOrders->isChecked()) {
		//executeAtStart aktivieren
		this->ui->checkBox_executeAtStart->setEnabled(true);
	} else {
		//ansonsten deaktivieren und ausschalten
		this->ui->checkBox_executeAtStart->setEnabled(false);
		this->ui->checkBox_executeAtStart->setChecked(false);
	}
}


void DialogSettings::on_listWidget_plugins_currentRowChanged(int currentRow)
{
	qDebug() << Q_FUNC_INFO << "current row:" << currentRow;
	this->refreshImExProfileTableWidget();
}

void DialogSettings::on_tableWidget_profiles_itemChanged(QTableWidgetItem *item)
{
	QString pluginName;
	int listWidgetRow = this->ui->listWidget_plugins->currentRow();
	pluginName = this->ui->listWidget_plugins->item(listWidgetRow)->text();

	//AqBanking remains the owner of 'ie', so we must not free it!
	AB_IMEXPORTER *ie = AB_Banking_GetImExporter(banking->getAqBanking(),
						     pluginName.toStdString().c_str());

	bool editorSupported = false;
	editorSupported = (AB_ImExporter_GetFlags(ie) & AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED);
	//enable the profile-edit if an editor is supported by AqBanking
	this->ui->actionEditProfile->setEnabled(editorSupported);

	if (item->column() != 5) {
		return; //Favorit not changed
	}

	//qDebug() << Q_FUNC_INFO << "item" << item << "changed! Selected:" << item->checkState();

	//favorit changed, store the actual Value
	bool checked = item->checkState() == Qt::Checked;

	pluginName.append("/");
	pluginName.append(this->ui->tableWidget_profiles->item(item->row(), 0)->text()); //profile-name

	this->imex_favorites->insert(pluginName, checked);
}


void DialogSettings::on_actionEditProfile_triggered()
{
	qDebug() << Q_FUNC_INFO << "executing";

	if (this->ui->tableWidget_profiles->currentRow() <= -1) {
		//no item selected, we cant edit this
		return;
	}

	QString pluginName = this->ui->listWidget_plugins->currentItem()->text();
	const aqb_iePlugin *plugin = this->imexp->getPluginByName(pluginName);

	int curRow = this->ui->tableWidget_profiles->currentRow();
	QString profileName = this->ui->tableWidget_profiles->item(curRow, 0)->text();
	const aqb_ieProfile *profile = NULL;
	foreach(const aqb_ieProfile *pro, *plugin->getProfiles()) {
		if (pro->getValue("name").toString() == profileName) {
			profile = pro;
			break; //profile found, another cant match
		}
	}

	if (!profile) {
		qWarning() << Q_FUNC_INFO << "no profile with the name"
			   << profileName << "found. Aborting.";
		return;
	}

	//AqBanking remains the owner of 'ie', so we must not free it!
	AB_IMEXPORTER *ie = AB_Banking_GetImExporter(banking->getAqBanking(),
						     pluginName.toStdString().c_str());
	GWEN_DB_NODE *dbProfile = AB_Banking_GetImExporterProfile(banking->getAqBanking(),
								  pluginName.toStdString().c_str(),
								  profileName.toStdString().c_str());
	GWEN_DIALOG *pDlg = NULL;


	//I have no idea why the conversion from QString to const char* must
	//be done in the way following. If anyone can make this simpler, please
	//do this.
	//The commented out codelines are for verification if the strings are
	//converted in the expected way.

	qDebug() << Q_FUNC_INFO << *profile->getNames();

	std::string filename = profile->getValue("fileName").toString().toStdString();
	//qDebug() << Q_FUNC_INFO << "Filename QString(" << filename.c_str() << ")";

	const char *filename_cstr = filename.c_str();
	//strcpy(filename_cstr, filename.c_str()); /* doesnt work */

	//qDebug() << Q_FUNC_INFO << "filename_cstr =" << filename_cstr;

	int ret = AB_ImExporter_GetEditProfileDialog(ie, dbProfile,
						     filename_cstr,
						     &pDlg);
	qDebug() << Q_FUNC_INFO << "return of AB_ImExporter_GetEditProfileDialog()"
		 << "is:" << ret;
	//free(filename_cstr); //free the copy

	int ret2 = 0; //default aborted
	if (pDlg) {
		uint32_t guiid = GWEN_Dialog_GetGuiId(pDlg);
		ret2 = GWEN_Gui_ExecDialog(pDlg, guiid);
	} else {
		qWarning() << Q_FUNC_INFO << "AB_ImExporter_GetEditProfileDialog()"
			   << "could not get a pDlg for the profile" << profileName;
	}

	//free the dialog after execution
	GWEN_Dialog_free(pDlg);

	qDebug() << Q_FUNC_INFO << "return of GWEN_Gui_ExecDialog():" << ret2;

	if (ret2) {
		//User did not cancel the dialog, save profile
		ret2 = AB_Banking_SaveLocalImExporterProfile(banking->getAqBanking(),
							     pluginName.toStdString().c_str(),
							     dbProfile,
							     filename_cstr);
		qDebug() << Q_FUNC_INFO << "SaveLocalImExporterProfile returned" << ret2;
	}


}
