/******************************************************************************
 * Copyright (C) 2011, 2013 Patrick Wacker
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

#include "widgetvalue.h"

#include <QtGui/QLayout>
#include <QApplication>
#include <QDesktopWidget>

#include "widgetcalculator.h"
#include "../abt_validators.h"

widgetValue::widgetValue(QWidget *parent) :
    QWidget(parent)
{
	this->currency = new QLineEdit(this);
	this->value = new QLineEdit(this);
	this->calcFrame = NULL;

	BetragValidator *validatorBetrag = new BetragValidator(this);
	validatorBetrag->setRegExp(QRegExp("[0-9]+,[0-9][0-9]", Qt::CaseSensitive));

	this->currency->setText("EUR");
	this->currency->setReadOnly(true);
	this->currency->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->currency->setMaximumWidth(45);

	this->value->setMinimumWidth(125);
//	this->value->setValidator(validatorBetrag);
	this->value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->value->setAlignment(Qt::AlignRight);
	this->value->installEventFilter(this);

	QHBoxLayout *layout = new QHBoxLayout();
	/** \todo: add fallback icon for calculator button */
	QPushButton *calcBtn = new QPushButton(QIcon::fromTheme("accessories-calculator"), "", this);
	calcBtn->setFocusProxy(this->value);
	connect(calcBtn, SIGNAL(clicked()), this, SLOT(showCalculator()));

	layout->addWidget(this->currency, 1, Qt::AlignRight);
	layout->addWidget(this->value, 0, Qt::AlignRight);
	layout->addWidget(calcBtn);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

widgetValue::~widgetValue()
{
	delete this->value;
	delete this->currency;
	delete this->calcFrame;
}

//protected
bool widgetValue::eventFilter(QObject * /*o*/, QEvent *e)
{
	bool rc = false;

	// we want to catch some keys that are usually handled by
	// the base class (e.g. '+', '-', etc.)
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *k = static_cast<QKeyEvent *>(e);

		rc = true;
		switch (k->key()) {
		case Qt::Key_Plus:
		case Qt::Key_Minus:
			if (this->value->hasSelectedText()) {
				this->value->cut();
			}
			if (this->value->text().length() == 0) {
				rc = false;
				break;
			}
			// in case of '-' we do not enter the calculator when
			// the current position is the beginning and there is
			// no '-' sign at the first position.
			if (k->key() == Qt::Key_Minus) {
				if (this->value->cursorPosition() == 0 &&
				    this->value->text()[0] != '-') {
					rc = false;
					break;
				}
			}
			// otherwise, tricky fall through here!

		case Qt::Key_Slash:
		case Qt::Key_Asterisk:
		case Qt::Key_Percent:
			if (this->value->hasSelectedText()) {
				// remove the selected text
				this->value->cut();
			}
			showCalculator();
			//calculatorOpen(k);
			break;
		default:
			rc = false;
			break;
		}

//	} else if (e->type() == QEvent::FocusOut) {
//		if (!m_edit->text().isEmpty() || !allowEmpty)
//			ensureFractionalPart();
//
//		if (MyMoneyMoney(m_edit->text()) != MyMoneyMoney(m_text) &&
//		    !this->calculator->isVisible()) {
//			emit valueChanged(m_edit->text());
//		}
//		m_text = m_edit->text();
	}
	return rc;
}

//protected
QWidget *widgetValue::focusWidget()
{
	QWidget* w = this->value;
	while (w->focusProxy())
	  w = w->focusProxy();
	return w;
}

//public
QString widgetValue::getValue() const
{
	if (! this->value->isEnabled()) return QString();

	return this->value->text();
}

//public
QString widgetValue::getCurrency() const
{
	if (! this->currency->isEnabled()) return QString();

	return this->currency->text();
}

//public
const AB_VALUE* widgetValue::getValueABV() const
{
	return abt_conv::ABValueFromString(this->value->text(),
					   this->currency->text());
}

//public
bool widgetValue::hasChanges() const
{
	return (this->value->isModified() || this->currency->isModified());
}

//private
void widgetValue::createCalcFrame()
{
	if (this->calcFrame)
		delete this->calcFrame;

	this->calcFrame = new QFrame(this);
	this->calcFrame->setWindowFlags(Qt::Popup);
	this->calcFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
	this->calcFrame->setLineWidth(2);

	this->calculator = new WidgetCalculator(this->calcFrame);
	this->calcFrame->hide();

	connect(this->calculator, SIGNAL(signalResultAvailable()),
		this, SLOT(calculatorResult()));
}

//private slot
void widgetValue::calculatorResult()
{
	this->value->setText(this->calculator->result());
	this->calcFrame->hide();
}

//public slot
void widgetValue::showCalculator()
{
	if (!this->calcFrame)
		this->createCalcFrame();

	this->calculator->setInitialValues(this->value->text(), NULL);

	int h = this->calcFrame->height();
	int w = this->calcFrame->width();

	// usually, the calculator widget is shown underneath the MoneyEdit widget
	// if it does not fit on the screen, we show it above this widget
	QPoint p = this->mapToGlobal(QPoint(0, 0));
	if (p.y() + this->height() + h > QApplication::desktop()->height())
		p.setY(p.y() - h);
	else
		p.setY(p.y() + this->height());

	// usually, it is shown left aligned. If it does not fit, we align it
	// to the right edge of the widget
	if (p.x() + w > QApplication::desktop()->width())
		p.setX(p.x() + this->width() - w);

	QRect r = this->calculator->geometry();
	r.moveTopLeft(p);
	this->calcFrame->setGeometry(r);

	this->calcFrame->show();
	this->calculator->setFocus();
}


//public slot
void widgetValue::clearAll()
{
	this->value->clear();
}

//public slot
void widgetValue::setValue(const QString &value)
{
	this->value->setText(value);
}

//public slot
void widgetValue::setValue(const AB_VALUE *abv)
{
	this->value->setText(abt_conv::ABValueToString(abv, true));
}

//public slot
void widgetValue::setCurrency(const QString &currency)
{
	this->currency->setText(currency);
}

//public slot
void widgetValue::setValueCurrency(const QString &value, const QString &currency)
{
	this->setValue(value);
	this->setCurrency(currency);
}

//public slot
void widgetValue::setLimitAllowChange(int b)
{
	this->setDisabled(b == -1);
}
