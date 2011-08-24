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

#include "bankaccountswidget.h"
#include "ui_bankaccountswidget.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringList>

#include <QDebug>

#include "../abt_settings.h"

BankAccountsWidget::BankAccountsWidget(aqb_Accounts *accounts, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BankAccountsWidget)
{
	ui->setupUi(this);
	this->m_accounts = accounts;

	QStringList headerlist;
	headerlist.clear();
	headerlist.append(tr("BLZ/Kto-Nr"));
	headerlist.append(tr("Name"));
	headerlist.append(tr("Typ"));
	headerlist.append(tr("Währ."));
	headerlist.append(tr("Land"));
	headerlist.append(tr("Besitzer"));
	headerlist.append(tr("Backend"));

	ui->treeWidget->setColumnCount(7);
	ui->treeWidget->setHeaderLabels(headerlist);

	ui->treeWidget->setUniformRowHeights(true);

	//wir wollen hier eine zusammenfassung der Banken, damit unter einer
	//Bank nur die Konten bei dieser Bank angezeigt werden.

	//erstmal fügen wir alle BLZs einer Liste hinzu und löschen alle
	//duplikate, danach gehen wir die erstellte Liste durch und ausserdem
	//alle Konten. Wenn ein Konto zu eine Bank gehört fügen wir es unterhalb
	//der Bank hinzu.
	QStringList BLZs;
	QHashIterator<int, aqb_AccountInfo*> i(this->m_accounts->getAccountHash());
	while (i.hasNext()) {
	     i.next();
	     BLZs.append(i.value()->BankCode());
	}

	BLZs.removeDuplicates(); //duplicate löschen
	bool doTop = true;
	int ItemCount = 0;
	QTreeWidgetItem *topItem = NULL;
	QTreeWidgetItem *Item = NULL;
	QTreeWidgetItem *FirstItem = NULL;
	//Alle BLZs durchgehen
	for (int s=0; s<BLZs.size(); ++s) {
		doTop = true;
		QHashIterator<int, aqb_AccountInfo*> i(this->m_accounts->getAccountHash());
		//Alle Konten durchgehen
		while (i.hasNext()) {
			i.next();
			//gehört dieses Konto zur BLZ?
			if (BLZs.at(s) == i.value()->BankCode()) {
				if (doTop) {
					topItem = new QTreeWidgetItem;
					ItemCount++;
					topItem->setData(0, Qt::DisplayRole, i.value()->BankCode());
					topItem->setData(1, Qt::DisplayRole, i.value()->BankName());
					topItem->setFlags(Qt::ItemIsEnabled);
					this->ui->treeWidget->addTopLevelItem(topItem);
					doTop = false;
				}
				Q_ASSERT(topItem != NULL);
				Item = new QTreeWidgetItem;
				ItemCount++;
				Item->setData(0, Qt::DisplayRole, i.value()->Number());
				Item->setData(0, Qt::UserRole, i.value()->get_ID());
				Item->setData(0, Qt::UserRole+1, (quint64)i.value()->get_AB_ACCOUNT());
				Item->setData(1, Qt::DisplayRole, i.value()->Name());
				Item->setData(2, Qt::DisplayRole, i.value()->AccountType());
				Item->setData(3, Qt::DisplayRole, i.value()->Currency());
				Item->setData(4, Qt::DisplayRole, i.value()->Country());
				Item->setData(5, Qt::DisplayRole, i.value()->OwnerName());
				Item->setData(6, Qt::DisplayRole, i.value()->BackendName());

				if (!FirstItem)
					FirstItem = Item;

				topItem->addChild(Item);
			}
		}
	}

	this->ui->treeWidget->expandAll(); //Alles aufklappen
	//Alle Spalten auf "perfekte" Breite anpassen
	abt_settings::resizeColToContentsFor(this->ui->treeWidget);

	//Erstes Wählbares Item auswählen, wenn vorhanden
	if (FirstItem) {
		ui->treeWidget->setItemSelected(FirstItem, true);
	}

	//int ItemHeight = FirstItem->sizeHint(0).height();
	int ItemHeight = this->ui->treeWidget->fontMetrics().height()+4;
	//qDebug() << "ItemHeight: " << ItemHeight;

	//ItemCount+2 = 1xHeader und 1x damit alle angezeigt werden auch wenn
	//eine horizontale Scrollbar vorhanden ist.
	this->setMinimumHeight(ItemHeight*(ItemCount+2) + 8);

}

BankAccountsWidget::~BankAccountsWidget()
{
	delete ui;
}

void BankAccountsWidget::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

aqb_AccountInfo *BankAccountsWidget::getSelectedAccount()
{
	if (ui->treeWidget->selectedItems().size() == 0) {
		return NULL;
	}
	int AccountID = ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	return this->m_accounts->getAccountHash().value(AccountID, NULL);
}

void BankAccountsWidget::on_treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (!current)
		current = previous;

	int AccountID = current->data(0, Qt::UserRole).toInt();

	aqb_AccountInfo *acc = this->m_accounts->getAccountHash().value(AccountID, NULL);

	emit this->Account_Changed(acc);
}
