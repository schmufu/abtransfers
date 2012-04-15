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

#include <QFileDialog>

#include "../abt_settings.h"

DialogSettings::DialogSettings(abt_settings *settings, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);
	Q_ASSERT(settings);

	this->settings = settings;

	this->loadFromSettings();

	//sicherstellen das der Status der CheckBoxen "Aktualisieren beim Start"
	//konsistent zueinander ist.
	this->onCheckBoxRefereshAtStartClicked();

	connect(this->ui->checkBox_getBalance, SIGNAL(clicked()),
		this, SLOT(onCheckBoxRefereshAtStartClicked()));
	connect(this->ui->checkBox_getDatedTransfers, SIGNAL(clicked()),
		this, SLOT(onCheckBoxRefereshAtStartClicked()));
	connect(this->ui->checkBox_getStandingOrders, SIGNAL(clicked()),
		this, SLOT(onCheckBoxRefereshAtStartClicked()));

}

DialogSettings::~DialogSettings()
{
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
void DialogSettings::onCheckBoxRefereshAtStartClicked()
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
