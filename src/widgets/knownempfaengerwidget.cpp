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

#include "knownempfaengerwidget.h"
#include "ui_knownempfaengerwidget.h"

#include "../abt_settings.h"

KnownEmpfaengerWidget::KnownEmpfaengerWidget(const QList<abt_EmpfaengerInfo*> *list, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::KnownEmpfaengerWidget)
{
	ui->setupUi(this);
	this->EmpfaengerList = list; //could be NULL!
	this->DisplayEmpfaenger();
}

KnownEmpfaengerWidget::~KnownEmpfaengerWidget()
{
	delete ui;
}

void KnownEmpfaengerWidget::changeEvent(QEvent *e)
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

void KnownEmpfaengerWidget::DisplayEmpfaenger()
{
	QStringList headerList;
	QTreeWidgetItem *Item = NULL;
	int ItemCount = 0;

	if (this->EmpfaengerList == NULL) {
		/* Anzeigen das keine bekannten Empfänger existieren */
		//kein header und nur eine spalte anzeigen
		ui->treeWidget->setHeaderHidden(true);
		ui->treeWidget->setColumnCount(1);

		Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole, tr("keine bekannten Empfänger vorhanden"));
		Item->setFlags(Qt::NoItemFlags); //Nicht wählbares Item
		ui->treeWidget->addTopLevelItem(Item);
		//vertical strech auf den Wert der enthaltenen Items setzen
		this->sizePolicy().setVerticalStretch(2);
		return; //nichts weiter zu tun
	}

	headerList << tr("Name") << tr("Kto-Nr") << tr("BLZ");
	this->ui->treeWidget->setHeaderHidden(false);
	this->ui->treeWidget->setColumnCount(3);
	this->ui->treeWidget->setHeaderLabels(headerList);


	//Alle bekannten Empfänger durchgehen
	for (int i=0; i<this->EmpfaengerList->size(); ++i) {
		Item = new QTreeWidgetItem;
		ItemCount++;
		Item->setData(0, Qt::DisplayRole, this->EmpfaengerList->at(i)->getName());
		Item->setData(0, Qt::UserRole, (qint64)this->EmpfaengerList->at(i));
		Item->setData(1, Qt::DisplayRole, this->EmpfaengerList->at(i)->getKontonummer());
		Item->setData(2, Qt::DisplayRole, this->EmpfaengerList->at(i)->getBLZ());
		this->ui->treeWidget->addTopLevelItem(Item);
	}

	this->ui->treeWidget->expandAll(); //Alles aufklappen
	//Alle Spalten auf "perfekte" Breite anpassen
	abt_settings::resizeColToContentsFor(this->ui->treeWidget);

	//vertical strech auf den Wert der enthaltenen Items setzen
	this->sizePolicy().setVerticalStretch(ItemCount+2);
}


void KnownEmpfaengerWidget::on_treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (!current)
		current = previous;

	emit this->EmpfaengerSelected((abt_EmpfaengerInfo*)current->data(0, Qt::UserRole).toInt());
}
