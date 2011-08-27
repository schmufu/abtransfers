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
#include <QDebug>

#include "../abt_conv.h"

UeberweisungsWidget::UeberweisungsWidget(const aqb_banking *banking,
					 TransferWidgetType type,
					 QWidget *parent) :
    QGroupBox(parent),
    my_type(type),
    ui(new Ui::UeberweisungsWidget)
{
	ui->setupUi(this);

	this->m_banking = banking;
	this->da_widget = NULL;

	//Create Validators for Critical Numbers
	QRegExpValidator *validatorKTO = new QRegExpValidator(this->ui->lineEdit_Kontonummer);
	QRegExpValidator *validatorBLZ = new QRegExpValidator(this->ui->lineEdit_Bankleitzahl);
	QRegExpValidator *validatorBetrag = new QRegExpValidator(this->ui->lineEdit_Betrag);

	validatorKTO->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));
	validatorBLZ->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));
	validatorBetrag->setRegExp(QRegExp("[0-9]+,[0-9][0-9]", Qt::CaseSensitive));

	ui->lineEdit_Kontonummer->setValidator(validatorKTO);
	ui->lineEdit_Bankleitzahl->setValidator(validatorBLZ);
	ui->lineEdit_Betrag->setValidator(validatorBetrag);

	//je nachdem was wir sind müssen wir noch weitere Widgets anzeigen
	switch (this->my_type) {
	case UeberweisungsWidget::StandingOrder: //Dauerauftrag
		this->createStandingOrderWidgets();
		break;
	case UeberweisungsWidget::DatedTransfer: //Terminüberweisung
		this->createDatedTransferWidgets();
		break;
	case UeberweisungsWidget::Transfer: //"Normale" Überweisung
		this->createTransferWidgets();
		break;
	case UeberweisungsWidget::InternatinalTransfer: //Internationale Überweisung
		this->createInternationalTransferWidgets();
		break;
	case UeberweisungsWidget::SepaTransfer: //SEPA Überweisung
		this->createSepaTransferWidgets();
		break;
	default:
		qWarning() << "UeberweisungsWidget: TransferWidgetType Unknown!";
		break;
	}
}

UeberweisungsWidget::~UeberweisungsWidget()
{
	//delete this->da_widget; //wird durch parent erledigt
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


/******** private functions for creating extra widgets *******/

/*!
 * Folgende Daten werden für einen Dauerauftrag benötigt:
 *   - period\n
 *     contains the execution period (e.g. whether a standing order is to be
 *     executed weekly or monthly etc).
 *   - cycle\n
 *     The standing order is executed every \a cycle x \a period. So if \a period
 *     is \a weekly and \a cycle is 2 then the standing order is executed every 2
 *     weeks.
 *   - executionDay\n
 *     The execution day. The meaning of this variable depends on the content of
 *     \a period:
 *       -# monthly: day of the month (starting with 1 )
 *       -# weekly: day of the week (starting with 1 =Monday)
 *   - FirstExecutionDate\n
 *     The date when the standing order is to be executed for the first time.
 *   - LastExecutionDate\n
 *     The date when the standing order is to be executed for the last time.
 *   - NextExecutionDate\n
 *     The date when the standing order is to be executed next (this field is
 *     only interesting when retrieving the list of currently active standing
 *     orders)
 *
 */
void UeberweisungsWidget::createStandingOrderWidgets()
{
	QVBoxLayout *vl = new QVBoxLayout();
	this->da_widget = new extraStandingOrdersWidget(this);
	vl->addWidget(this->da_widget);

	this->ui->verticalLayout_11->addLayout(vl);
}

/*!
 * Folgende Daten werden für Terminüberweisungen benötigt:
 *   - executionDay\n
 *     der Tag an dem die Überweisung ausgeführt werden soll
 */
void UeberweisungsWidget::createDatedTransferWidgets()
{

}

/*!
 * Werden weitere Widgets für eine "Normale" Überweisung benötigt?
 *
 * eventuell folgende:
 *   - Textschlüssel
 *   - TransactionCode\n
 *     A 3 digit numerical transaction code, defined for all kinds of different
 *     actions. (Geschaeftsvorfallcode)
 *   - TransactionText\n
 *     Transaction text (e.g. STANDING ORDER) (Buchungstext)
 */
void UeberweisungsWidget::createTransferWidgets()
{

}

void UeberweisungsWidget::createInternationalTransferWidgets()
{

}

void UeberweisungsWidget::createSepaTransferWidgets()
{

}


/******* END - private functions for creating extra widgets *******/

void UeberweisungsWidget::on_lineEdit_Bankleitzahl_editingFinished()
{
	QString Institut;
	Institut = this->m_banking->getInstituteFromBLZ(
			ui->lineEdit_Bankleitzahl->text().toUtf8());
	this->ui->lineEdit_Kredidinstitut->setText(Institut);
}

/*! \brief gibt true zurück wenn die Daten im Widget durch den User geändert wurden */
bool UeberweisungsWidget::hasChanges() const
{
	if (this->da_widget != NULL) {
		if (this->da_widget->hasChanges()) {
			return true; //Änderungen vorhanden
		}
	}

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
	}

	//Wenn wir bis hierher kommen haben keine Änderungen stattgefunden
	return false;
}

/*! \brief löscht alle Eingabefelder bzw. setzt Sie auf defaultwerte */
//public slot
void UeberweisungsWidget::clearAllEdits()
{
	if (this->da_widget != NULL) {
		this->da_widget->clearAllEdits();
	}
	ui->lineEdit_Beguenstigter->clear();
	ui->lineEdit_Kontonummer->clear();
	ui->lineEdit_Bankleitzahl->clear();
	ui->lineEdit_Betrag->clear();
	ui->lineEdit_Kredidinstitut->clear();
	ui->lineEdit_Verwendungszweck1->clear();
	ui->lineEdit_Verwendungszweck2->clear();
	ui->lineEdit_Verwendungszweck3->clear();
	ui->lineEdit_Verwendungszweck4->clear();
}

/******************************************************************************/
/****** Funktionen zum setzen und lesen der allgemeinen Daten            ******/
/******************************************************************************/


const QString UeberweisungsWidget::getRemoteName() const
{
	return this->ui->lineEdit_Beguenstigter->text();
}

void UeberweisungsWidget::setRemoteName(const QString &str)
{
	this->ui->lineEdit_Beguenstigter->setText(str);
}

const QString UeberweisungsWidget::getRemoteAccountNumber() const
{
	return this->ui->lineEdit_Kontonummer->text();
}

void UeberweisungsWidget::setRemoteAccountNumber(const QString &str)
{
	this->ui->lineEdit_Kontonummer->setText(str);
}

const QString UeberweisungsWidget::getRemoteBankCode() const
{
	return this->ui->lineEdit_Bankleitzahl->text();
}

void UeberweisungsWidget::setRemoteBankCode(const QString &str)
{
	this->ui->lineEdit_Bankleitzahl->setText(str);
}

const QString UeberweisungsWidget::getRemoteBankName() const
{
	return this->ui->lineEdit_Kredidinstitut->text();
}

void UeberweisungsWidget::setRemoteBankName(const QString &str)
{
	if (!str.isEmpty()) {
		//Übergebenen Namen anzeigen
		this->ui->lineEdit_Kredidinstitut->setText(str);
	} else if (!this->ui->lineEdit_Bankleitzahl->text().isEmpty()) {
		//Bankleitzahl wurde gesetzt, zu dieser den Banknamen holen
		this->on_lineEdit_Bankleitzahl_editingFinished();
	} else {
		this->ui->lineEdit_Kredidinstitut->setText(str);
	}

}

const QString UeberweisungsWidget::getValue() const
{
	return this->ui->lineEdit_Betrag->text();
}

//! Gibt den Wert und die Währung als AB_VALUE zurück
AB_VALUE* UeberweisungsWidget::getValueABV() const
{
	QString text = this->ui->lineEdit_Betrag->text();
	text.replace(",",".",Qt::CaseSensitive);
	return abt_conv::ABValueFromString(text, this->ui->lineEdit_Waehrung->text());
}

void UeberweisungsWidget::setValue(const QString &str)
{
	this->ui->lineEdit_Betrag->setText(str);
}

//! sets the value AND currency from the AB_VALUE* \a value
void UeberweisungsWidget::setValue(const AB_VALUE *value)
{
	this->ui->lineEdit_Betrag->setText(abt_conv::ABValueToString(value, true));
	this->ui->lineEdit_Waehrung->setText(AB_Value_GetCurrency(value));
}

const QString UeberweisungsWidget::getCurrency() const
{
	return this->ui->lineEdit_Waehrung->text();
}

//! if you habe a AB_VALUE use setValue(const AB_VALUE*) to set the value and currency
void UeberweisungsWidget::setCurrency(const QString &str)
{
	this->ui->lineEdit_Waehrung->setText(str);
}

void UeberweisungsWidget::setPurpose(const QStringList &strList)
{
	QString text;
	for (int i=0; i<4; ++i) {
		if (strList.count() > i) {
			text = strList.at(i);
		} else {
			text.clear();
		}
		this->setPurpose(i+1, text);
	}
}

const QStringList UeberweisungsWidget::getPurpose() const
{
	//Alle Verwendungszeckzeilen durchgehen und wenn nicht leer diese
	//der StringListe hinzufügen.
	QStringList purpose;
	QString editName;
	QLineEdit *edit = NULL;
	purpose.clear();

	for (int i=1; i<5; ++i) {
		editName = QString("lineEdit_Verwendungszweck%1").arg(i);
		edit = this->findChild<QLineEdit*>(editName);
		Q_ASSERT(edit != NULL);
		if ( ! edit->text().isEmpty()) {
			purpose.append(edit->text().toUtf8());
		}
	}

	qDebug() << "returned StringList:" << purpose;
	return purpose;
}

void UeberweisungsWidget::setPurpose(int line, const QString &str)
{
	QString editName = QString("lineEdit_Verwendungszweck%1").arg(line);
	QLineEdit *edit = this->findChild<QLineEdit*>(editName);
	if (edit) {
		edit->setText(str);
	} else {
		qWarning() << "setPurpose(int, QString): "
				<< editName << " not found";
	}
}

const QString UeberweisungsWidget::getPurpose(int line) const
{
	// \todo Noch testen ob dies so funktioniert!
	// Done: Funktioniert! (Patrick Wacker 24.08.2011)
	QString editName = QString("lineEdit_Verwendungszweck%1").arg(line);
	QLineEdit *edit = this->findChild<QLineEdit*>(editName);
	if (edit == NULL)
		return QString("EDIT \"%1\" NOT FOUND").arg(editName);
	return edit->text().toUtf8();
}



/******************************************************************************/
/****** Wrapper Funktionen die die Daten in this->da_widget setzen       ******/
/****** bzw. die dort eingegebenen Daten abfragen                        ******/
/******************************************************************************/

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setPeriod(AB_TRANSACTION_PERIOD period)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setPeriod(period);
}

//! wrapper to extraStandingOrdersWidget
AB_TRANSACTION_PERIOD UeberweisungsWidget::period() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->period();
}

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setExecutionDay(int day)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setExecutionDay(day);
}

//! wrapper to extraStandingOrdersWidget
int UeberweisungsWidget::executionDay() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->executionDay();
}

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setCycle(int cycle)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setCycle(cycle);
}

//! wrapper to extraStandingOrdersWidget
int UeberweisungsWidget::cycle() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->cycle();
}

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setFirstExecutionDate(const QDate &date)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setFirstExecutionDate(date);
}

//! wrapper to extraStandingOrdersWidget
const QDate UeberweisungsWidget::firstExecutionDate() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->firstExecutionDate();
}

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setLastExecutionDate(const QDate &date)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setLastExecutionDate(date);
}

//! wrapper to extraStandingOrdersWidget
const QDate UeberweisungsWidget::lastExecutionDate() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->lastExecutionDate();
}

//! wrapper to extraStandingOrdersWidget
void UeberweisungsWidget::setNextExecutionDate(const QDate &date)
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	this->da_widget->setNextExecutionDate(date);
}

//! wrapper to extraStandingOrdersWidget
const QDate UeberweisungsWidget::nextExecutionDate() const
{
	Q_ASSERT_X(this->da_widget != NULL, "UeberweisungsWidget", "extraStandingOrdersWidget called without object");
	return this->da_widget->nextExecutionDate();
}



















/******************************************************************************
  Methods and Event handling for Drag'n'Drop
*******************************************************************************/

void UeberweisungsWidget::dragEnterEvent(QDragEnterEvent *event)
{
	//qDebug() << "dragEnterEvent: Format =" << event->mimeData()->formats();
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") &&
	    event->possibleActions() & Qt::CopyAction) {
		event->setDropAction(Qt::CopyAction);
		event->accept();
	}
}


void UeberweisungsWidget::dropEvent(QDropEvent *event)
{
//	QString dropText = event->mimeData()->data(event->mimeData()->formats().at(0));
//	qDebug() << "dropped: " << dropText;

	QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
	QDataStream stream(&encoded, QIODevice::ReadOnly);

	while (!stream.atEnd())
	{
		int row, col;
		QMap<int,  QVariant> roleDataMap;
		stream >> row >> col >> roleDataMap;

		//enable the debug line to see whats in the data
		//qDebug() << "row:" << row << "col:" << col << "roleDataMap" << roleDataMap;
		switch (col) {
		case 0: //Name in Qt::DisplayRole und ptr zu abt_EmpfaengerInfo in Qt::userRole
			this->setRemoteName(roleDataMap.value(Qt::DisplayRole).toString());
			break;
		case 1: //KontoNummer
			this->setRemoteAccountNumber(roleDataMap.value(Qt::DisplayRole).toString());
			break;
		case 2: //BankCode
			this->setRemoteBankCode(roleDataMap.value(Qt::DisplayRole).toString());
			break;
		default:
			break;
		}
	}

	//nachdem alles gesetzt wurde den bankname ermitteln, bzw wenn bereits
	//gesetzt so belassen (siehe inhalt der Funktion!)
	this->setRemoteBankName(QString(""));

	event->setDropAction(Qt::CopyAction);
	event->accept();
}
