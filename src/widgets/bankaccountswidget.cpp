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
#include <QMouseEvent>

#include <QDebug>

#include "../abt_settings.h"

BankAccountsWidget::BankAccountsWidget(aqb_Accounts *accounts, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BankAccountsWidget)
{
	ui->setupUi(this);
	this->m_accounts = accounts;
	this->dragStartPos = QPoint(0,0);
	this->dragObj = NULL;


	QStringList headerlist;
	headerlist.clear();
	headerlist.append(tr("BLZ/Kto-Nr"));
	headerlist.append(tr("Name"));
	headerlist.append(tr("Typ"));
	headerlist.append(tr("Währ."));
	headerlist.append(tr("Land"));
	headerlist.append(tr("Besitzer"));
	headerlist.append(tr("Backend"));
	headerlist.append(tr("BankLine"));
	headerlist.append(tr("BookedBalance"));
	headerlist.append(tr("Date"));

	ui->treeWidget->setColumnCount(10);
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
	i.toFront();
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
		i.toFront();
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
				//Alle Werte für das neu erstellte Item setzen.
				this->setValuesForItem(Item, i.value());

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

	this->ui->treeWidget->viewport()->installEventFilter(this);

	//Alle accounts mit unserem Slot verbinden, damit bei einer Änderung
	//die angezeigten Werte aktualisert werden können
	QHashIterator<int, aqb_AccountInfo*> it(this->m_accounts->getAccountHash());
	it.toFront();
	while (it.hasNext()) {
	     it.next();
	     connect(it.value(), SIGNAL(accountStatusChanged(const aqb_AccountInfo*)),
		     this, SLOT(onAccountStatusChange(const aqb_AccountInfo*)));
	}


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

bool BankAccountsWidget::eventFilter(QObject *obj, QEvent *event)
{
	//qDebug() << this << "eventFilter()";
	if (obj != this->ui->treeWidget->viewport()) {
		//standard event processing
		return QWidget::eventFilter(obj, event);
	}

	if (event->type() == QEvent::MouseButtonPress) {
		this->twMousePressEvent(dynamic_cast<QMouseEvent*>(event));
		return QWidget::eventFilter(obj, event);
	}

	if (event->type() == QEvent::MouseMove) {
		this->twMouseMoveEvent(dynamic_cast<QMouseEvent*>(event));
		return QWidget::eventFilter(obj, event);
	}

	//qDebug() << this << "eventFilter() nothing! type:" << event->type();
	return QWidget::eventFilter(obj, event);
}

void BankAccountsWidget::twMousePressEvent(QMouseEvent *event)
{
	//qDebug() << this << "twMousePressEvent()";
	if (event->button() == Qt::LeftButton) {
		QTreeWidgetItem *item = this->ui->treeWidget->itemAt(event->pos());
		if (item) {
			//Qt::UserRole+1 enthält einen Pointer zum aqb_AccountInfo
			this->dragObj = item->data(0, Qt::UserRole+1).value<aqb_AccountInfo*>();
			this->dragStartPos = event->pos();
		} else {
			this->dragObj = NULL;
			this->dragStartPos = QPoint(0,0);
		}
	}
}

void BankAccountsWidget::twMouseMoveEvent(QMouseEvent *event)
{
	//qDebug() << this << "twMouseMoveEvent()";
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}
	if (this->dragObj == NULL) {
		return; //no object to Drag set!
	}
	if ((event->pos() - this->dragStartPos).manhattanLength()
		< QApplication::startDragDistance()) {
		return;
	}


	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	aqb_AccountInfo* info = this->dragObj;

	qulonglong a = (qulonglong)info;
	qulonglong app = (qulonglong)qApp;
	QString result;
	QTextStream(&result) << a;
	qDebug() << result;
	//Nur dieselbe Instanz darf diesen Pointer verwenden!
	QString mimetype = QString("application/x-abBanking_%1_AccountInfo").arg(app);
	mimeData->setData(mimetype, QByteArray(result.toAscii()));
	drag->setMimeData(mimeData);
	drag->setPixmap(QPixmap(":/icons/bank-icon"));

	drag->exec(Qt::CopyAction);

	//evt. später auch den return wert auswerten um zu wissen was mit dem
	//Drag-Objekt gemacht wurde
	//Qt::DropAction dropAction = drag->exec(Qt::CopyAction);

	//qDebug() << this << "dropAction" << dropAction;
}

//private
void BankAccountsWidget::setValuesForItem(QTreeWidgetItem *item,
					  const aqb_AccountInfo *acc) const
{
	item->setData(0, Qt::DisplayRole, acc->Number());
	item->setData(0, Qt::UserRole, acc->get_ID());
	//pointer zum aqb_AccountInfo Object
	item->setData(0, Qt::UserRole+1, QVariant::fromValue(acc));
	//wird anscheinen nirgends verwendet!
	//item->setData(0, Qt::UserRole+1, (quint64)i.value()->get_AB_ACCOUNT());
	item->setData(1, Qt::DisplayRole, acc->Name());
	item->setData(2, Qt::DisplayRole, acc->AccountType());
	item->setData(3, Qt::DisplayRole, acc->Currency());
	item->setData(4, Qt::DisplayRole, acc->Country());
	item->setData(5, Qt::DisplayRole, acc->OwnerName());
	item->setData(6, Qt::DisplayRole, acc->BackendName());
	item->setData(7, Qt::DisplayRole, acc->getBankLine());
	item->setData(8, Qt::DisplayRole, acc->getBookedBalance());
	item->setData(9, Qt::DisplayRole, QString("%1").arg(acc->getDate().toString(Qt::DefaultLocaleLongDate)));
}

aqb_AccountInfo *BankAccountsWidget::getSelectedAccount()
{
	if (ui->treeWidget->selectedItems().size() == 0) {
		return NULL;
	}
	int AccountID = ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	return this->m_accounts->getAccountHash().value(AccountID, NULL);
}

/** setzt den aktuell ausgewählten Account auf \a account */
void BankAccountsWidget::setSelectedAccount(const aqb_AccountInfo *account)
{
	//Alle Items durchgehen und wenn wir das Item mit derselben ID gefunden
	//haben dieses Auswählen.

	int selectID = -1;
	if (account != NULL) { //wenn Account übergeben, dessen ID selectieren
		selectID = account->get_ID();
	}

	//alle Selectionen neu setzen
	for (int i=0; i<this->ui->treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *topItem = this->ui->treeWidget->topLevelItem(i);
		for (int j=0; j<topItem->childCount(); ++j) {
			QTreeWidgetItem *childItem = topItem->child(j);
			//Prüfen ob dieser Eintrag ausgewählt werden soll, und
			//entsprechend setzen
			bool sel = childItem->data(0, Qt::UserRole).toInt() == selectID;
			childItem->setSelected(sel);
		}
		//TopItems sind immer deselectet!
		topItem->setSelected(false);
	}
}

//private slot
void BankAccountsWidget::on_treeWidget_itemSelectionChanged()
{
	if (this->ui->treeWidget->selectedItems().size() > 0) {
		QTreeWidgetItem *selItem = this->ui->treeWidget->selectedItems().at(0);
		int accountID = selItem->data(0, Qt::UserRole).toInt();
		aqb_AccountInfo *acc = this->m_accounts->getAccountHash().value(accountID, NULL);
		emit this->Account_Changed(acc);
	} else {
		emit this->Account_Changed(NULL);
	}
}

//private slot
void BankAccountsWidget::onAccountStatusChange(const aqb_AccountInfo *account)
{
	//Beim übergebenen account hat sich etwas geändert
	//Wir suchen das QTreeWidgetItem welches die Werte anzeigt und erstellen
	//die Werte neu.

	//Item enthält in der row 0 als Qt::UserRole+1 die Addresse des Accounts!
	QTreeWidgetItem *item = NULL;
	QTreeWidgetItem *wantedItem = NULL;

	//das Item finden welches für den Account zuständig ist
	for(int i=0; i<this->ui->treeWidget->topLevelItemCount(); ++i) {
		item = this->ui->treeWidget->topLevelItem(i);
		for (int j=0; j<item->childCount(); ++j) {
			if (item->child(j)->data(0, Qt::UserRole+1).value<const aqb_AccountInfo*>() == account) {
				wantedItem = item->child(j);
			}
		}
	}

	if (wantedItem) {
		this->setValuesForItem(wantedItem, account);
	} else {
		qWarning() << Q_FUNC_INFO << "should update account but no item found!";
	}
}
