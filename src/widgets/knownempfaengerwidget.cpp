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

#include <QMouseEvent>
#include <QDebug>

KnownEmpfaengerWidget::KnownEmpfaengerWidget(const QList<abt_EmpfaengerInfo*> *list, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::KnownEmpfaengerWidget)
{
	ui->setupUi(this);
	this->EmpfaengerList = list; //could be NULL!
	this->DisplayEmpfaenger();

	this->dragStartPos = QPoint(0,0);
	this->dragObj = NULL;
//	this->ui->treeWidget->setDragEnabled(true);
	this->ui->treeWidget->viewport()->installEventFilter(this);
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
	QString result;
	QTextStream(&result) << a;
	qDebug() << result;
	mimeData->setData("application/x-abBaning_KnownRecipient", QByteArray(result.toAscii()));
	//mimeData->setData("text/plain", info);
	drag->setMimeData(mimeData);
	drag->setPixmap(QPixmap(":/icons/knownEmpfaenger"));

	Qt::DropAction dropAction = drag->exec(Qt::CopyAction);

	//qDebug() << this << "dropAction" << dropAction;
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
