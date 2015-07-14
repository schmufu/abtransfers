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


#include "abt_dialog.h"
#include "ui_abt_dialog.h"

#include <QStyle>
#include <QPushButton>
#include <QDebug>

#include "../globalvars.h" //für das settings-Object

abt_dialog::abt_dialog(QWidget *parent,
		       QString title,
		       QString msgtext,
		       QDialogButtonBox::StandardButtons buttons,
		       QDialogButtonBox::StandardButton defaultButton /* = QDialogButtonBox::NoButton */,
		       QMessageBox::Icon icon /* = QMessageBox::NoIcon */,
		       QString dialogName /* = "" */,
		       bool showCheckBox /* = true */,
		       QString checkBoxText /* = "" */) :
	QDialog(parent),
	ui(new Ui::abt_dialog)
{
	ui->setupUi(this);

	this->m_dialogName = dialogName; //wir merken uns unseren Namen
	this->m_defaultButton = defaultButton;
	this->m_result = QDialogButtonBox::NoButton;

	if (m_dialogName.isEmpty() ||
	    (this->m_defaultButton == QDialogButtonBox::NoButton)) {
		//wenn kein Name angegeben ist wird der Dialog immer angezeigt,
		//allerdings ohne die CheckBox (macht dann keinen Sinn)
		this->m_showThisDialog = true;
		showCheckBox = false;
	} else {
		//Wenn ein Name angegeben wurde die Einstellung ob der Dialog
		//angezeigt werden soll oder nicht aus der settings.ini holen.
		this->m_showThisDialog = settings->showDialog(dialogName);
	}

	//Icon entsprechend der vorgabe einstellen
	this->ui->label_Icon->setText(QString()); //keine Beschriftung
	QIcon tmpIcon;
	QStyle *style = parent ? parent->style() : QApplication::style();
	int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, parent);
	switch(icon) {
	case QMessageBox::NoIcon:
		this->ui->label_Icon->setHidden(true);
		//wenn kein Label angezeigt wird auch das Layout so anpassen das
		//der MsgText über die gesammte Breite angezeigt wird.
		this->ui->gridLayout->setHorizontalSpacing(0);
		break;
	case QMessageBox::Information:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, parent);
		break;
	case QMessageBox::Warning:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, parent);
		break;
	case QMessageBox::Critical:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, parent);
		break;
	case QMessageBox::Question:
		tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, parent);
		break;
	default:
		break;
	}

	if (!tmpIcon.isNull()) {
		this->ui->label_Icon->setPixmap(tmpIcon.pixmap(iconSize, iconSize));
	}


	QString tmpTitle = tr("%1 - %2").arg(title,QApplication::applicationName());
	this->setWindowTitle(tmpTitle); //Titel einstellen

	this->ui->label_text->setText(msgtext); //Text anzeigen
	//checkBox ausblenden wenn gewollt
	this->ui->checkBox->setHidden(!showCheckBox);

	//Alle Buttons darstellen die angegeben wurden
	this->ui->buttonBox->setStandardButtons(buttons);

	//Den Default-Button setzen
	QPushButton *btn = this->ui->buttonBox->button(m_defaultButton);
	if (btn) btn->setDefault(true);

	//Den gewollten CheckBoxText setzen, bzw. auf default einstellen.
	QString cbtext;
	if (checkBoxText.isEmpty()) {
		cbtext = tr("Diese Meldung nicht wieder anzeigen");
	} else {
		cbtext = checkBoxText;
	}
	this->ui->checkBox->setText(cbtext);

}

abt_dialog::~abt_dialog()
{
	delete ui;
}

void abt_dialog::changeEvent(QEvent *e)
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

void abt_dialog::on_buttonBox_clicked(QAbstractButton* button)
{
	//Als result liefern wir das Ergebiss welcher Button geklickt wurde zurück
	this->m_result = this->ui->buttonBox->standardButton(button);
	this->setResult(this->m_result);

	//Nur wenn der Dialog mit dem angegebenen Default-Button akzeptiert wurde
	//den Zustand der CheckBox in der ini-Datei speichern.
	if (this->m_defaultButton == this->m_result) {
		//merken ob der Dialog ein weiteres mal angezeigt werden soll
		settings->setShowDialog(this->m_dialogName, !this->ui->checkBox->isChecked());
	}
	this->close();
}

//reimplemented public slot
int abt_dialog::exec()
{
	//Wenn der Dialog angezeigt werden soll führen wir exec() vom QDialog aus.
	if (this->m_showThisDialog) {
		QDialog::exec();
		return this->m_result;
	}

	//Unser Dialog soll nicht angezeigt werden, wir setzen einfach
	//den default return Wert und beenden uns wieder
	this->setResult(this->m_defaultButton);
	this->m_result = this->m_defaultButton;
	return this->m_result;
}
