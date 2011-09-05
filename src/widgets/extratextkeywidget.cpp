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

#include "extratextkeywidget.h"
#include "ui_extratextkeywidget.h"

#include <QDebug>
#include "../globalvars.h"

extraTextKeyWidget::extraTextKeyWidget(const QList<int> *keys, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::extraTextKeyWidget)
{
	ui->setupUi(this);
	this->fillTextKeys(keys);
	qDebug() << this << "created";
}

extraTextKeyWidget::~extraTextKeyWidget()
{
	delete ui;
	qDebug() << this << "deleted";
}

void extraTextKeyWidget::changeEvent(QEvent *e)
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
void extraTextKeyWidget::fillTextKeys(const QList<int> *keys)
{
	QComboBox *cb = this->ui->comboBox;
	cb->clear();
	if (keys == NULL) {
		return; //Abbruch, keine Keys setzen und ComboBox leer lassen
	}

	const QHash<int, QString> *desc = settings->getTextKeyDescriptions();

	//Alle Testschlüssel mit Bezeichnung in der ComboBox darstellen, wenn
	//zu einem Schlüssel kein Text existiert wird unbekannt angezeigt.
	foreach (int i, *keys) {
		cb->addItem(tr("%1 - %2").arg(i).arg(desc->value(i,tr("Unbekannt"))), i);
	}
	cb->setCurrentIndex(0); //Erster Eintrag als Default
}

/** Den übergebenen Key setzen */
void extraTextKeyWidget::setTextKey(int key)
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
	//key übergeben, wir setzen einfach den ersten
	cb->setCurrentIndex(0);
}

/** liefert den aktuell gewählten TextKey zurück */
int extraTextKeyWidget::getTextKey() const
{
	int idx = this->ui->comboBox->currentIndex();
	if (idx == -1) {
		return idx; //Kein Eintrag gewählt, -1 wird als Fehler zurückgegeben
	}
	int ret = this->ui->comboBox->itemData(idx, Qt::UserRole).toInt();
	return ret;
}



















