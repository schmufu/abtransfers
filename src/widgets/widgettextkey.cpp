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

#include "widgettextkey.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include "../globalvars.h"

widgetTextKey::widgetTextKey(const QList<int> *keys, QWidget *parent) :
    QWidget(parent)
{
	this->comboBox = new QComboBox(this);
	this->label = new QLabel(tr("Textschlüssel"), this);

	this->fillTextKeys(keys);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(this->label);
	layout->addWidget(this->comboBox);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(2);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

widgetTextKey::~widgetTextKey()
{
	delete this->comboBox;
	delete this->label;
}

/** Setzt alle Textschlüssel der ComboBox */
void widgetTextKey::fillTextKeys(const QList<int> *keys)
{
	QComboBox *cb = this->comboBox;
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
void widgetTextKey::setTextKey(int key)
{
	QComboBox *cb = this->comboBox;

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
int widgetTextKey::getTextKey() const
{
	int idx = this->comboBox->currentIndex();
	if (idx == -1) {
		return idx; //Kein Eintrag gewählt, -1 wird als Fehler zurückgegeben
	}
	int ret = this->comboBox->itemData(idx, Qt::UserRole).toInt();
	return ret;
}

