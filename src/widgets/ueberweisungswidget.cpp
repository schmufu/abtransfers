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

#include "ueberweisungswidget.h"
#include "ui_ueberweisungswidget.h"

#include <QRegExpValidator>

UeberweisungsWidget::UeberweisungsWidget(const aqb_banking *banking,
					 TransferWidgetType type,
					 QWidget *parent) :
    QGroupBox(parent),
    my_type(type),
    ui(new Ui::UeberweisungsWidget)
{
	ui->setupUi(this);

	this->m_banking = banking;

	//Create Validators for Critical Numbers
	QRegExpValidator *validatorKTO = new QRegExpValidator(this);
	QRegExpValidator *validatorBLZ = new QRegExpValidator(this);
	QRegExpValidator *validatorBetrag = new QRegExpValidator(this);

	validatorKTO->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));
	validatorBLZ->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));
	validatorBetrag->setRegExp(QRegExp("[0-9]+,[0-9][0-9]", Qt::CaseSensitive));

	ui->lineEdit_Kontonummer->setValidator(validatorKTO);
	ui->lineEdit_Bankleitzahl->setValidator(validatorBLZ);
	ui->lineEdit_Betrag->setValidator(validatorBetrag);
}

UeberweisungsWidget::~UeberweisungsWidget()
{
	delete ui;
}

void UeberweisungsWidget::changeEvent(QEvent *e)
{
	QGroupBox::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}


void UeberweisungsWidget::on_lineEdit_Bankleitzahl_editingFinished()
{
	QString Institut;
	Institut = this->m_banking->getInstituteFromBLZ(
			ui->lineEdit_Bankleitzahl->text().toUtf8());
	this->ui->lineEdit_Kredidinstitut->setText(Institut);
}


bool UeberweisungsWidget::hasChanges() const
{
	if (ui->lineEdit_Beguenstigter->isModified() ||
	    ui->lineEdit_Kontonummer->isModified() ||
	    ui->lineEdit_Bankleitzahl->isModified() ||
	    ui->lineEdit_Betrag->isModified() ||
	    ui->lineEdit_Kredidinstitut->isModified() ||
	    ui->lineEdit_Verwendungszweck1->isModified() ||
	    ui->lineEdit_Verwendungszweck2->isModified() ||
	    ui->lineEdit_Verwendungszweck3->isModified() ||
	    ui->lineEdit_Verwendungszweck4->isModified()) {
		return true;
	} else {
		return false;
	}
}

const QStringList UeberweisungsWidget::getPurpose() const
{
	QStringList purpose;
	purpose.clear();
	purpose.append(ui->lineEdit_Verwendungszweck1->text().toUtf8());
	purpose.append(ui->lineEdit_Verwendungszweck2->text().toUtf8());
	purpose.append(ui->lineEdit_Verwendungszweck3->text().toUtf8());
	purpose.append(ui->lineEdit_Verwendungszweck4->text().toUtf8());
	return purpose;
}

const QString UeberweisungsWidget::getPurpose(int line) const
{
	/*! \todo Noch testen ob dies so funktioniert! */
	QString editName = QString("lineEdit_Verwendungszweck%1").arg(line);
	QLineEdit *edit = this->findChild<QLineEdit*>(editName);
	if (edit == NULL)
		return QString("EDIT %1 NOT FOUND").arg(line);
	return edit->text().toUtf8();
}
