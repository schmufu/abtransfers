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
#include "widgetaccountdata.h"

#include <QMouseEvent>
#include <QDebug>
#include <QString>
#include <QMenu>
#include <QDialog>
#include <QLayout>
#include <QPushButton>

KnownEmpfaengerWidget::KnownEmpfaengerWidget(const QList<abt_EmpfaengerInfo*> *list, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::KnownEmpfaengerWidget)
{
	ui->setupUi(this);
	this->CreateAllActions();
	this->EmpfaengerList = list; //could be NULL!
	this->DisplayEmpfaenger();

	this->dragStartPos = QPoint(0,0);
	this->dragObj = NULL;
//	this->ui->treeWidget->setDragEnabled(true);
	this->ui->treeWidget->viewport()->installEventFilter(this);
	this->ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	this->ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
	connect(this->ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(onContextMenuRequest(QPoint)));
	qDebug() << this << "created";
}

KnownEmpfaengerWidget::~KnownEmpfaengerWidget()
{
	delete ui;
	qDebug() << this << "deleted";
}

void KnownEmpfaengerWidget::changeEvent(QEvent *e)
{
	QGroupBox::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		this->retranslateCppCode();
		break;
	default:
		break;
	}
}

bool KnownEmpfaengerWidget::eventFilter(QObject *obj, QEvent *event)
{
	//qDebug() << this << "eventFilter()";
	if (obj != this->ui->treeWidget->viewport()) {
		//standard event processing
		return QGroupBox::eventFilter(obj, event);
	}

	if (event->type() == QEvent::MouseButtonPress) {
		this->twMousePressEvent(dynamic_cast<QMouseEvent*>(event));
		return QGroupBox::eventFilter(obj, event);
	}

	if (event->type() == QEvent::MouseMove) {
		this->twMouseMoveEvent(dynamic_cast<QMouseEvent*>(event));
		return QGroupBox::eventFilter(obj, event);
	}

	//qDebug() << this << "eventFilter() nothing! type:" << event->type();
	return QGroupBox::eventFilter(obj, event);
}

void KnownEmpfaengerWidget::twMousePressEvent(QMouseEvent *event)
{
	//qDebug() << this << "twMousePressEvent()";
	if (event->button() == Qt::LeftButton) {
		QTreeWidgetItem *item = this->ui->treeWidget->itemAt(event->pos());
		if (item) {
			this->dragObj = item->data(0, Qt::UserRole).value<abt_EmpfaengerInfo*>();
			this->dragStartPos = event->pos();
		} else {
			this->dragObj = NULL;
			this->dragStartPos = QPoint(0,0);
		}
	}
}

//protected
/** \copydoc MainWindow::retranslateCppCode() */
void KnownEmpfaengerWidget::retranslateCppCode()
{
	//we simple delete and recreate the actions.
	delete this->actNew;
	delete this->actEdit;
	delete this->actDelete;
	this->CreateAllActions(); //this sets the new tr() strings ;)
	this->DisplayEmpfaenger(); //redraw the whole tree widget
}

void KnownEmpfaengerWidget::twMouseMoveEvent(QMouseEvent *event)
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
	abt_EmpfaengerInfo* info = this->dragObj;

	qulonglong a = (qulonglong)info;
	qulonglong app = (qulonglong)qApp; //unsere Instanz!
	QString result;
	QTextStream(&result) << a;
	qDebug() << result;
	//Nur dieselbe Instanz darf diesen Pointer verwenden!
	QString mimetype = QString("application/x-abBanking_%1_KnownRecipient").arg(app);
	mimeData->setData(mimetype, QByteArray(result.toAscii()));
	//mimeData->setData("text/plain", info);
	drag->setMimeData(mimeData);
	drag->setPixmap(QPixmap(":/icons/knownEmpfaenger"));

	drag->exec(Qt::CopyAction);
	//evt. später auch den return wert auswerten um zu wissen was mit dem
	//Drag-Objekt gemacht wurde
	//Qt::DropAction dropAction = drag->exec(Qt::CopyAction);

	//qDebug() << this << "dropAction" << dropAction;
}

//private
void KnownEmpfaengerWidget::CreateAllActions()
{
	this->actNew = new QAction(this);
	this->actNew->setText(tr("Neu"));
	this->actNew->setToolTip(tr("Einen neuen Empfänger anlegen"));
	this->actNew->setIcon(QIcon::fromTheme("document-new", QIcon(":/icons/document-new")));
	connect(this->actNew, SIGNAL(triggered()), this, SLOT(onActionNewTriggered()));

	this->actDelete = new QAction(this);
	this->actDelete->setText(tr("Löschen"));
	this->actDelete->setToolTip(tr("Ausgewählten Empfänger löschen"));
	this->actDelete->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/delete")));
	connect(this->actDelete, SIGNAL(triggered()), this, SLOT(onActionDeleteTriggered()));

	this->actEdit = new QAction(this);
	this->actEdit->setText(tr("Ändern"));
	this->actEdit->setToolTip(tr("Ausgewählten Empfänger bearbeiten"));
	this->actEdit->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/document-edit")));
	connect(this->actEdit, SIGNAL(triggered()), this, SLOT(onActionEditTriggered()));
}

void KnownEmpfaengerWidget::DisplayEmpfaenger()
{
	QStringList headerList;
	QTreeWidgetItem *Item = NULL;
	int ItemCount = 0;

	//alle evt. vorhandenen Einträge löschen
	this->ui->treeWidget->clear();

	if (this->EmpfaengerList == NULL) {
		/* Anzeigen das keine bekannten Empfänger existieren */
		//kein header und nur eine spalte anzeigen
		ui->treeWidget->setHeaderHidden(true);
		ui->treeWidget->setColumnCount(1);

		Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole, tr("Keine bekannten Empfänger vorhanden"));
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
		//Pointer zum abt_EmpfaengerInfo in der Qt::UserRole speichern
		Item->setData(0, Qt::UserRole, QVariant::fromValue(this->EmpfaengerList->at(i)));
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

	//emit this->EmpfaengerSelected((abt_EmpfaengerInfo*)current->data(0, Qt::UserRole).toInt());
}

//public slot
void KnownEmpfaengerWidget::onEmpfaengerListChanged()
{
	this->DisplayEmpfaenger();
}

//private slot
void KnownEmpfaengerWidget::onContextMenuRequest(const QPoint &pos)
{
	//Actions disablen wenn sie nicht sinnvoll sind
	bool dis = this->ui->treeWidget->selectedItems().size() == 0;
	this->actEdit->setDisabled(dis);
	this->actDelete->setDisabled(dis);

	QMenu *contextMenu = new QMenu();
	contextMenu->addAction(this->actNew);
	contextMenu->addAction(this->actEdit);
	contextMenu->addAction(this->actDelete);

	contextMenu->exec(this->ui->treeWidget->viewport()->mapToGlobal(pos));
}

void KnownEmpfaengerWidget::onActionEditTriggered()
{
	abt_EmpfaengerInfo *origRecipient = NULL;
	abt_EmpfaengerInfo *newRecipient = NULL;

	//Pointer zum abt_EmpfaengerInfo wurde in der Qt::UserRole gespeichert
	origRecipient = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).value<abt_EmpfaengerInfo*>();

	QDialog *d = new QDialog(this);

	QHBoxLayout *btnLayout = new QHBoxLayout();
	QPushButton *btnSave = new QPushButton(tr("Speichern"), d);
	btnSave->setDefault(true);
	QPushButton *btnCancel = new QPushButton(tr("Abbrechen"), d);
	connect(btnSave, SIGNAL(clicked()), d, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked()), d, SLOT(reject()));

	btnLayout->addWidget(btnSave, 0, Qt::AlignCenter);
	btnLayout->addWidget(btnCancel, 0, Qt::AlignCenter);

	QVBoxLayout *layout = new QVBoxLayout();
	widgetAccountData *acc = new widgetAccountData(d, NULL, NULL, true, true);
	acc->setAllowDropAccount(false);
	acc->setAllowDropKnownRecipient(false);
	acc->setName(origRecipient->getName());
	acc->setAccountNumber(origRecipient->getKontonummer());
	acc->setBankCode(origRecipient->getBLZ());
	acc->setIBAN(origRecipient->getIBAN());
	acc->setBIC(origRecipient->getBIC());
	acc->setBankName(origRecipient->getInstitut());

	layout->addWidget(acc);

	layout->addLayout(btnLayout);

	d->setLayout(layout);


	if (d->exec() == QDialog::Accepted) {
		newRecipient = new abt_EmpfaengerInfo();
		newRecipient->setName(acc->getName());
		newRecipient->setKontonummer(acc->getAccountNumber());
		newRecipient->setBLZ(acc->getBankCode());
		newRecipient->setInstitut(acc->getBankName());
		newRecipient->setIBAN(acc->getIBAN());
		newRecipient->setBIC(acc->getBIC());
		int pos = this->EmpfaengerList->indexOf(origRecipient);
		emit replaceKnownEmpfaenger(pos, newRecipient);
	}
	// ansonsten Änderungen einfach verwerfen.

	delete d;
}

void KnownEmpfaengerWidget::onActionDeleteTriggered()
{
	abt_EmpfaengerInfo *Receiver = NULL;

	//Pointer zum abt_EmpfaengerInfo wurde in der Qt::UserRole gespeichert
	Receiver = this->ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).value<abt_EmpfaengerInfo*>();

	emit this->deleteKnownEmpfaenger(Receiver);
}

void KnownEmpfaengerWidget::onActionNewTriggered()
{
	abt_EmpfaengerInfo *newRecipient = NULL;

	QDialog *d = new QDialog(this);

	QHBoxLayout *btnLayout = new QHBoxLayout();
	QPushButton *btnSave = new QPushButton(tr("Speichern"), d);
	btnSave->setDefault(true);
	QPushButton *btnCancel = new QPushButton(tr("Abbrechen"), d);
	connect(btnSave, SIGNAL(clicked()), d, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked()), d, SLOT(reject()));

	btnLayout->addWidget(btnSave, 0, Qt::AlignCenter);
	btnLayout->addWidget(btnCancel, 0, Qt::AlignCenter);

	QVBoxLayout *layout = new QVBoxLayout();
	widgetAccountData *acc = new widgetAccountData(d, NULL, NULL, true, true);
	acc->setAllowDropAccount(false);
	acc->setAllowDropKnownRecipient(false);

	layout->addWidget(acc);

	layout->addLayout(btnLayout);

	d->setLayout(layout);


	if (d->exec() == QDialog::Accepted) {
		newRecipient = new abt_EmpfaengerInfo();
		newRecipient->setName(acc->getName());
		newRecipient->setKontonummer(acc->getAccountNumber());
		newRecipient->setBLZ(acc->getBankCode());
		newRecipient->setInstitut(acc->getBankName());
		newRecipient->setIBAN(acc->getIBAN());
		newRecipient->setBIC(acc->getBIC());
		emit addNewKnownEmpfaenger(newRecipient);
	}
	// ansonsten Änderungen einfach verwerfen.


	delete d;

}
