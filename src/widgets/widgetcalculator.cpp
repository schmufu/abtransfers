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

/* copyright information from the source file of kmymoney */
/***************************************************************************
			  kmymoneycalculator.cpp  -  description
			     -------------------
    begin                : Sat Oct 19 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
			   Javier Campos Morales <javi_c@users.sourceforge.net>
			   Felix Rodriguez <frodriguez@users.sourceforge.net>
			   John C <thetacoturtle@users.sourceforge.net>
			   Thomas Baumgart <ipwizard@users.sourceforge.net>
			   Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "widgetcalculator.h"

#include <QLabel>
#include <QSignalMapper>
#include <QRegExp>
#include <QGridLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QLocale>


WidgetCalculator::WidgetCalculator(QWidget* parent) :
	QFrame(parent)
{
	this->comma = QLocale::system().decimalPoint();
	this->clearOperandOnDigit = false;

	QGridLayout* grid = new QGridLayout(this);

	this->display = new QLabel(this);
	QPalette palette;
	palette.setColor(display->backgroundRole(), QColor("#BDFFB4"));
	this->display->setPalette(palette);

	this->display->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	this->display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	grid->addWidget(display, 0, 0, 1, 5);

	this->buttons[0] = new QPushButton("0", this);
	this->buttons[1] = new QPushButton("1", this);
	this->buttons[2] = new QPushButton("2", this);
	this->buttons[3] = new QPushButton("3", this);
	this->buttons[4] = new QPushButton("4", this);
	this->buttons[5] = new QPushButton("5", this);
	this->buttons[6] = new QPushButton("6", this);
	this->buttons[7] = new QPushButton("7", this);
	this->buttons[8] = new QPushButton("8", this);
	this->buttons[9] = new QPushButton("9", this);
	this->buttons[PLUS] = new QPushButton("+", this);
	this->buttons[MINUS] = new QPushButton("-", this);
	this->buttons[STAR] = new QPushButton("X", this);
	this->buttons[COMMA] = new QPushButton(comma, this);
	this->buttons[EQUAL] = new QPushButton("=", this);
	this->buttons[SLASH] = new QPushButton("/", this);
	this->buttons[CLEAR] = new QPushButton("C", this);
	this->buttons[CLEARALL] = new QPushButton("AC", this);
	this->buttons[PLUSMINUS] = new QPushButton("+-", this);
	this->buttons[PERCENT] = new QPushButton("%", this);

	grid->addWidget(buttons[7], 1, 0);
	grid->addWidget(buttons[8], 1, 1);
	grid->addWidget(buttons[9], 1, 2);
	grid->addWidget(buttons[4], 2, 0);
	grid->addWidget(buttons[5], 2, 1);
	grid->addWidget(buttons[6], 2, 2);
	grid->addWidget(buttons[1], 3, 0);
	grid->addWidget(buttons[2], 3, 1);
	grid->addWidget(buttons[3], 3, 2);
	grid->addWidget(buttons[0], 4, 1);

	grid->addWidget(buttons[COMMA], 4, 0);
	grid->addWidget(buttons[PLUS], 3, 3);
	grid->addWidget(buttons[MINUS], 4, 3);
	grid->addWidget(buttons[STAR], 3, 4);
	grid->addWidget(buttons[SLASH], 4, 4);
	grid->addWidget(buttons[EQUAL], 4, 2);
	grid->addWidget(buttons[PLUSMINUS], 2, 3);
	grid->addWidget(buttons[PERCENT], 2, 4);
	grid->addWidget(buttons[CLEAR], 1, 3);
	grid->addWidget(buttons[CLEARALL], 1, 4);

	this->buttons[EQUAL]->setFocus();

	this->op1 = 0.0;
	this->stackedOp = op = 0;
	this->operand.clear();
	this->changeDisplay("0");

	// connect the digit signals through a signal mapper
	QSignalMapper* mapper = new QSignalMapper(this);
	for (int i = 0; i < 10; ++i) {
		mapper->setMapping(buttons[i], i);
		connect(this->buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
	}
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(digitClicked(int)));

	// connect the calculation operations through another mapper
	mapper = new QSignalMapper(this);
	for (int i = PLUS; i <= EQUAL; ++i) {
		mapper->setMapping(buttons[i], i);
		connect(this->buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
	}
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(calculationClicked(int)));

	// connect all remaining signals
	connect(this->buttons[COMMA], SIGNAL(clicked()), SLOT(commaClicked()));
	connect(this->buttons[PLUSMINUS], SIGNAL(clicked()), SLOT(plusminusClicked()));
	connect(this->buttons[PERCENT], SIGNAL(clicked()), SLOT(percentClicked()));
	connect(this->buttons[CLEAR], SIGNAL(clicked()), SLOT(clearClicked()));
	connect(this->buttons[CLEARALL], SIGNAL(clicked()), SLOT(clearAllClicked()));

	for (int i = 0; i < MAX_BUTTONS; ++i) {
		this->buttons[i]->setMinimumSize(40, 30);
		this->buttons[i]->setMaximumSize(40, 30);
	}
	// keep the size determined by the size of the contained buttons no matter what
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

WidgetCalculator::~WidgetCalculator()
{
}

//protected slot
/** \brief This method appends the digit represented by the parameter to the
 *         current operand.
 *
 * @param button integer value of the digit to be added in the range [0..9]
 */
void WidgetCalculator::digitClicked(int button)
{
	if (this->clearOperandOnDigit) {
		this->operand.clear();
		this->clearOperandOnDigit = false;
	}

	this->operand += QChar(button + 0x30);
	if (this->operand.length() > 16)
		this->operand = this->operand.left(16);
	this->changeDisplay(operand);
}

//protected slot
/** \brief This method appends a period (comma).
 *
 * This is used to initialize the fractional part of an operand. The period
 * is only appended once.
 */
void WidgetCalculator::commaClicked(void)
{
	if (this->operand.length() == 0)
		this->operand = '0';
	if (this->operand.contains('.', Qt::CaseInsensitive) == 0)
		this->operand.append('.');

	if (this->operand.length() > 16)
		this->operand = operand.left(16);
	this->changeDisplay(operand);
}

//protected slot
/** \brief This method reverses the sign of the current operand */
void WidgetCalculator::plusminusClicked(void)
{
	if (this->operand.length() == 0 &&
	    this->lastResult.length() > 0) {
		this->operand = lastResult;
	}

	if (this->operand.length() > 0) {
		if (this->operand.indexOf('-') != -1) {
			this->operand.remove('-');
		} else {
			this->operand.prepend('-');
		}
		this->changeDisplay(operand);
	}
}

//protected slot
/** \brief This methods starts the operation contained in the parameter.
 *
 * @param button The Qt::Keycode for the button pressed or clicked
 */
void WidgetCalculator::calculationClicked(int button)
{
	if (this->operand.length() == 0 && this->op != 0 && button == EQUAL) {
		this->op = 0;
		this->lastResult = this->normalizeString(op1);
		this->changeDisplay(this->lastResult);

	} else if (this->operand.length() > 0 && this->op != 0) {
		// perform operation
		double op2 = this->operand.toDouble();
		bool error = false;

		// if the pending operation is addition and we now do multiplication
		// we just stack op1 and remember the operation in
		if ((this->op == PLUS || this->op == MINUS) &&
		    (button == STAR || button == SLASH)) {
			this->op0 = this->op1;
			this->stackedOp = this->op;
			this->op = 0;
		}

		switch (this->op) {
		case PLUS:
			op2 = this->op1 + op2;
			break;
		case MINUS:
			op2 = this->op1 - op2;
			break;
		case STAR:
			op2 = this->op1 * op2;
			break;
		case SLASH:
			if (op2 == 0.0)
				error = true;
			else
				op2 = this->op1 / op2;
			break;
		}

		// if we have a pending addition operation, and the next operation is
		// not multiplication, we calculate the stacked operation
		if (this->stackedOp && button != STAR && button != SLASH) {
			switch (this->stackedOp) {
			case PLUS:
				op2 = this->op0 + op2;
				break;
			case MINUS:
				op2 = this->op0 - op2;
				break;
			}
			this->stackedOp = 0;
		}

		if (error) {
			this->op = 0;
			this->changeDisplay("Error");
			this->operand.clear();
		} else {
			this->op1 = op2;
			this->lastResult = this->normalizeString(op1);
			this->changeDisplay(this->lastResult);
		}
	} else if (this->operand.length() > 0 && this->op == 0) {
		this->op1 = this->operand.toDouble();
		this->lastResult = this->normalizeString(op1);
		this->changeDisplay(this->lastResult);
	}

	if (button != EQUAL) {
		this->op = button;
	} else {
		this->op = 0;
		emit resultAvailable();
	}
	this->operand.clear();
}

//protected
/** \brief This method is used to transform a double into a QString and
 *         removing any trailing 0's and decimal separators.
 *
 * @param val reference to double value to be converted
 * @return QString object containing the converted value
 */
QString WidgetCalculator::normalizeString(const double& val)
{
	QString str;
	str.setNum(val, 'f');
	int i = str.length();
	while (i > 1 && str[i-1] == '0') {
		--i;
	}
	// cut off trailing 0's
	str.remove(i, str.length());
	if (str.length() > 0) {
		// possibly remove trailing period
		if (str[str.length()-1] == '.') {
			str.remove(str.length() - 1, 1);
		}
	}
	return str;
}

//protected slot
/** \brief This method clears the current operand */
void WidgetCalculator::clearClicked(void)
{
	if (this->operand.length() > 0) {
		this->operand = this->operand.left(this->operand.length() - 1);
	}
	if (this->operand.length() == 0)
		this->changeDisplay("0");
	else
		this->changeDisplay(this->operand);
}

//protected slot
/** \brief This method clears all registers */
void WidgetCalculator::clearAllClicked(void)
{
	this->operand.clear();
	this->op = 0;
	this->changeDisplay("0");
}

//protected slot
/** \brief This method executes the percent operation */
void WidgetCalculator::percentClicked(void)
{
	if (this->op != 0) {
		double op2 = this->operand.toDouble();
		switch (this->op) {
		case PLUS:
		case MINUS:
			op2 = (this->op1 * op2) / 100;
			break;

		case STAR:
		case SLASH:
			op2 /= 100;
			break;
		}
		this->operand = this->normalizeString(op2);
		this->changeDisplay(this->operand);
	}
}

//public
/** \brief this method is used to extract the result of the last calculation
 *
 * The fractional part is separated from the integral part by the character
 * setup using setComma().
 *
 * @return QString representing the result of the last operation
 */
const QString WidgetCalculator::result(void) const
{
	QString txt = lastResult;
	txt.replace(QRegExp("\\."), this->comma);
	if (txt[0] == '-') {
		txt = txt.mid(1); // get rid of the minus sign
		QString mask;
		mask = QLocale::system().negativeSign();
		mask.append("%1");
		txt = QString(mask).arg(txt);
	}
	return txt;

/***** original ****
		switch (KGlobal::locale()->negativeMonetarySignPosition()) {
		case KLocale::ParensAround:
			mask = "(%1)";
			break;
		case KLocale::AfterQuantityMoney:
			mask = "%1-";
			break;
		case KLocale::AfterMoney:
		case KLocale::BeforeMoney:
			mask = "%1 -";
			break;
		case KLocale::BeforeQuantityMoney:
			mask = "-%1";
			break;
		}
		txt = QString(mask).arg(txt);
	}
	return txt;
*********************************/
}

//protected slot
/** \brief This method updates the display of the calculator with the text
 *         passed as argument.
 *
 * @param str reference to QString containing the new display contents
 */
void WidgetCalculator::changeDisplay(const QString& str)
{
	QString txt = str;
	txt.replace(QRegExp("\\."), comma);
	this->display->setText("<b>" + txt + "</b>");
}

//protected
/** \brief handles the keyPressEvent's and executes the expected functions.
 *
 * This member function is used to handle all key press events. When e.g. 2
 * is pressed this is mapped to the same action as by clicking the "2" button.
 */
void WidgetCalculator::keyPressEvent(QKeyEvent* ev)
{
	int button = -1;

	switch (ev->key()) {
	case Qt::Key_0:
	case Qt::Key_1:
	case Qt::Key_2:
	case Qt::Key_3:
	case Qt::Key_4:
	case Qt::Key_5:
	case Qt::Key_6:
	case Qt::Key_7:
	case Qt::Key_8:
	case Qt::Key_9:
		if (this->clearOperandOnDigit) {
			this->operand.clear();
			this->clearOperandOnDigit = false;
		}
		button = ev->key() - Qt::Key_0;
		break;
	case Qt::Key_Plus:
		button = PLUS;
		break;
	case Qt::Key_Minus:
		button = MINUS;
		break;
	case Qt::Key_Comma:
	case Qt::Key_Period:
		if (this->clearOperandOnDigit) {
			this->operand.clear();
			this->clearOperandOnDigit = false;
		}
		button = COMMA;
		break;
	case Qt::Key_Slash:
		button = SLASH;
		break;
	case Qt::Key_Backspace:
		button = CLEAR;
		break;
	case Qt::Key_Asterisk:
		button = STAR;
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Equal:
		button = EQUAL;
		break;
	case Qt::Key_Escape:
		button = CLEARALL;
		break;
	case Qt::Key_Percent:
		button = PERCENT;
		break;
	default:
		ev->ignore();
		break;
	}
	if (button != -1)
		this->buttons[button]->animateClick();

	this->clearOperandOnDigit = false;
}

//public
/** \brief This method is used to preset the first operand and start execution
 *         of an operation.
 *
 * This method is currently used by widgetValue. If @p ev is 0, then no
 * operation will be started.
 *
 * @param value reference to QString representing the operands value
 * @param ev    pointer to QKeyEvent representing the operation's key
 */
void WidgetCalculator::setInitialValues(const QString& value, QKeyEvent* ev)
{
	bool negative = false;
	// setup operand
	this->operand = value;
	//operand.replace(QRegExp(QString('\\') + KGlobal::locale()->thousandsSeparator()), QChar());
	this->operand.replace(QRegExp(QString('\\') + QLocale::system().groupSeparator()), QChar());
	this->operand.replace(QRegExp(QString('\\') + comma), ".");
	if (this->operand.contains('(')) {
		negative = true;
		this->operand.remove('(');
		this->operand.remove(')');
	}
	if (this->operand.contains('-')) {
		negative = true;
		this->operand.remove('-');
	}
	if (this->operand.isEmpty())
		this->operand = '0';
	else if (negative)
		this->operand = QString("-%1").arg(operand);

	this->changeDisplay(this->operand);

	// and operation
	this->op = 0;
	if (ev)
		this->keyPressEvent(ev);
	else
		this->clearOperandOnDigit = true;
}
