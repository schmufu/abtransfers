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

#include "extratransferwidget.h"
#include "ui_extratransferwidget.h"

#include <QDebug>

extraTransferWidget::extraTransferWidget(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::extraTransferWidget)
{
	ui->setupUi(this);
	this->fillTextKeys();
	qDebug() << this << "created";
}

extraTransferWidget::~extraTransferWidget()
{
	delete ui;
	qDebug() << this << "deleted";
}

void extraTransferWidget::changeEvent(QEvent *e)
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

/** Setzt alle Textschlüssel der ComboBox */
void extraTransferWidget::fillTextKeys()
{
	QComboBox *cb = this->ui->comboBox;
	cb->clear();
	cb->addItem(tr("%1 - Überweisung").arg(51), 51);
	cb->addItem(tr("%1 - Lohn/Gehalt").arg(53), 53);
	cb->addItem(tr("%1 - Vermögenswirksame Leistung (VL)").arg(54), 54);
	cb->setCurrentIndex(0); //Überweisung als Default
}

/** Den übergebenen Key setzen */
void extraTransferWidget::setTextKey(int key)
{
	QComboBox *cb = this->ui->comboBox;

	//Alle Elemente durchgehen und wenn der "richtige" gefunden wurde
	//diesen als aktuelles Item setzen
	for (int i=0; i<cb->count(); ++i) {
		if (cb->itemData(i, Qt::UserRole).toInt() == key) {
			cb->setCurrentIndex(i);
			break;
		}
	}

	//Wenn wir hierher kommen wurde ein nicht in der ComboBox vorhandener
	//key übergeben, wir setzen einfach 0 als default
	cb->setCurrentIndex(0);
}

/** liefert den aktuell gewählten TextKey zurücl */
int extraTransferWidget::getTextKey() const
{
	int idx = this->ui->comboBox->currentIndex();
	int ret = this->ui->comboBox->itemData(idx, Qt::UserRole).toInt();
	return ret;
}



















