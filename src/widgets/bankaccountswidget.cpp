/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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
#include <QDrag>
#include <QMimeData>

#include <QDebug>

#include "../abt_settings.h"

BankAccountsWidget::BankAccountsWidget(const aqb_Accounts *accounts,
				       QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BankAccountsWidget)
{
	ui->setupUi(this);
	this->m_accounts = accounts; //could be NULL!
	this->dragStartPos = QPoint(0,0);
	this->dragObj = NULL;


	QTreeWidgetItem *headerItem = new QTreeWidgetItem;

	this->setHeaderItemCaptions(headerItem);

	ui->treeWidget->setColumnCount(10);
	ui->treeWidget->setHeaderItem(headerItem);
	ui->treeWidget->setUniformRowHeights(true);

	this->setAccounts(accounts);
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
		this->retranslateCppCode();
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

//protected
/** \copydoc MainWindow::retranslateCppCode() */
void BankAccountsWidget::retranslateCppCode()
{
	QTreeWidgetItem *header = this->ui->treeWidget->headerItem();
	this->setHeaderItemCaptions(header);
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
	QString mimetype = QString::fromUtf8("application/x-abBanking_%1_AccountInfo").arg(app);
	mimeData->setData(mimetype, QByteArray(result.toLatin1()));
	drag->setMimeData(mimeData);
	drag->setPixmap(QPixmap(QString::fromUtf8(":/icons/bank-icon")));

	drag->exec(Qt::CopyAction);

	//evt. später auch den return wert auswerten um zu wissen was mit dem
	//Drag-Objekt gemacht wurde
	//Qt::DropAction dropAction = drag->exec(Qt::CopyAction);

	//qDebug() << this << "dropAction" << dropAction;
}

//public slot
void BankAccountsWidget::setAccounts(const aqb_Accounts *accounts)
{
	this->m_accounts = accounts; //could be NULL!

	this->ui->treeWidget->clear();//alle vorhandenen Einträge löschen
	//kein Drag&Drop mehr verwalten (keine Objecte im treeWidget!)
	this->ui->treeWidget->viewport()->removeEventFilter(this);

	if (this->m_accounts == NULL) {
		//Es existieren keine Accounts, deswegen brauchen wir auch
		//nichts machen, erst wenn wir mit einem gültigen aqb_Accounts
		//Objekt aufgerufen werden erstellen wir auch die Daten
		return; //Abbrechen
	}

	//wir wollen hier eine zusammenfassung der Banken, damit unter einer
	//Bank nur die Konten bei dieser Bank angezeigt werden.

	//erstmal fügen wir alle BLZs einer Liste hinzu und löschen alle duplikate
	QStringList BLZs;
	foreach(const aqb_AccountInfo* acc, this->m_accounts->getAccountHash().values()) {
		BLZs.append(acc->BankCode());
	}
	BLZs.removeDuplicates(); //duplicate löschen

	//Danach gehen wir die erstellte Liste durch und ausserdem alle Konten.
	//Wenn ein Konto zu einer Bankleitzahl gehört fügen wir es unterhalb
	//der Bank hinzu.

	bool doTop = true;
	int ItemCount = 0;
	QTreeWidgetItem *topItem = NULL;
	QTreeWidgetItem *Item = NULL;
	QTreeWidgetItem *FirstItem = NULL;
	//Alle BLZs durchgehen
	foreach(const QString blz, BLZs) {
		doTop = true;
		//Alle Konten durchgehen
		foreach (const aqb_AccountInfo* acc, this->m_accounts->getAccountHash().values()) {
			//gehört dieses Konto zur BLZ?
			if (blz == acc->BankCode()) {
				if (doTop) { //wenn ein TopItem erstellt werden muss
					topItem = new QTreeWidgetItem;
					ItemCount++;
					topItem->setData(0, Qt::DisplayRole, acc->BankCode());
					topItem->setData(1, Qt::DisplayRole, acc->BankName());
					topItem->setFlags(Qt::ItemIsEnabled);
					this->ui->treeWidget->addTopLevelItem(topItem);
					doTop = false;
				}
				Item = new QTreeWidgetItem;
				ItemCount++;
				//Alle Werte für das neu erstellte Item setzen.
				this->setValuesForItem(Item, acc);

				if (!FirstItem) { //damit es später ausgewählt werden kann
					FirstItem = Item;
				}
				//Die Werte des Accounts der BLZ zuweisen
				topItem->addChild(Item);
			}
		}
	}

	this->ui->treeWidget->expandAll(); //Alles aufklappen
	//Alle Spalten auf "perfekte" Breite anpassen
	abt_settings::resizeColToContentsFor(this->ui->treeWidget);

	//if at least one item exist, we select this and calculate the
	//minimum height of the treeWidget.
	if (FirstItem) {
		ui->treeWidget->setItemSelected(FirstItem, true);

		int itemHeight = ui->treeWidget->visualItemRect(FirstItem).height();
		int headerHeight = ui->treeWidget->header()->height();
		int scrollbarHeight = 19; //were can we get this value

		//the minimum heigth should be right do display all entrys.
		//But we wont get larger than 150 pixel for the minimum.
		int minHeight = (ItemCount * itemHeight) + headerHeight + scrollbarHeight;
		if (minHeight > 150) minHeight = 150;
		this->setMinimumHeight(minHeight);
	}

	//eventFilter einsetzen damit wir Drag&Drop verwalten können
	this->ui->treeWidget->viewport()->installEventFilter(this);

	//Alle accounts mit unserem Slot verbinden, damit bei einer Änderung
	//die angezeigten Werte aktualisert werden können
	foreach(const aqb_AccountInfo *acc, this->m_accounts->getAccountHash().values()) {
		connect(acc, SIGNAL(accountStatusChanged(const aqb_AccountInfo*)),
			this, SLOT(accountChangedUpdateDisplay(const aqb_AccountInfo*)));
	}

}

//private
void BankAccountsWidget::setValuesForItem(QTreeWidgetItem *item,
					  const aqb_AccountInfo *acc) const
{
	item->setData(0, Qt::DisplayRole, acc->Number());
	item->setData(0, Qt::UserRole, acc->get_ID());
	//pointer zum aqb_AccountInfo Object
	item->setData(0, Qt::UserRole+1, QVariant::fromValue(acc));
	item->setData(1, Qt::DisplayRole, acc->Name());
	item->setData(2, Qt::DisplayRole, acc->getBookedBalance());
	item->setData(2, Qt::TextAlignmentRole, Qt::AlignRight); //Saldo rechtsbündig
	item->setData(3, Qt::DisplayRole, acc->Currency());
	item->setData(4, Qt::DisplayRole, acc->getBankLine());
	item->setData(4, Qt::TextAlignmentRole, Qt::AlignRight); //Dispo rechtsbündig
	QDate date = acc->getDate();
	QString format = QString::fromUtf8("ddd dd. MMM yyyy");
	QString value = QString::fromUtf8("%1").arg(date.toString(format));
	item->setData(5, Qt::DisplayRole, value);
	item->setData(5, Qt::TextAlignmentRole, Qt::AlignHCenter); //Datum mittig
	item->setData(6, Qt::DisplayRole, acc->AccountType());
	item->setData(7, Qt::DisplayRole, acc->Country());
	item->setData(8, Qt::DisplayRole, acc->OwnerName());
	item->setData(9, Qt::DisplayRole, acc->BackendName());
}

//private
/** \brief sets all captions for the header item
 *
 * The captions are set on the supplied \a headerItem and also the
 * TextAlignment attributes are set.
 */
void BankAccountsWidget::setHeaderItemCaptions(QTreeWidgetItem *headerItem) const
{
	headerItem->setData(0, Qt::DisplayRole, tr("BLZ/Kto-Nr"));
	headerItem->setData(1, Qt::DisplayRole, tr("Name"));
	headerItem->setData(2, Qt::DisplayRole, tr("Saldo"));
	headerItem->setData(2, Qt::TextAlignmentRole, Qt::AlignHCenter); //Saldo centered
	headerItem->setData(3, Qt::DisplayRole, tr("Währ."));
	headerItem->setData(4, Qt::DisplayRole, tr("Dispo"));
	headerItem->setData(4, Qt::TextAlignmentRole, Qt::AlignHCenter); //Dispo centered
	headerItem->setData(5, Qt::DisplayRole, tr("Daten vom"));
	headerItem->setData(5, Qt::TextAlignmentRole, Qt::AlignHCenter); //Datum centered
	headerItem->setData(6, Qt::DisplayRole, tr("Typ"));
	headerItem->setData(7, Qt::DisplayRole, tr("Land"));
	headerItem->setData(8, Qt::DisplayRole, tr("Besitzer"));
	headerItem->setData(9, Qt::DisplayRole, tr("Backend"));
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

//public slot
void BankAccountsWidget::accountChangedUpdateDisplay(const aqb_AccountInfo *account)
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
