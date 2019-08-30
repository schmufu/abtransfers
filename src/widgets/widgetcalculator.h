/******************************************************************************
 * Copyright (C) 2013 Patrick Wacker
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
 *	simple electronic calculator.
 *	copied from kmymoney (widgets/kmymoneycalculator.h and .cpp) and
 *	adjusted for the usage without kde includes.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

/* copyright information from the source file of kmymoney */
/***************************************************************************
			  kmymoneycalculator.h  -  description
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


#ifndef WIDGETCALCULATOR_H
#define WIDGETCALCULATOR_H

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QPushButton>

/**
  *@author Thomas Baumgart
  */

/**
  * This class implements a simple electronic calculator with the
  * ability of addition, subtraction, multiplication and division
  * and percentage calculation. Memory locations are not available.
  *
  * The first operand and operation can be loaded from an external
  * source to switch from an edit-widget to the calculator widget
  * without having the user to re-type the data. See setInitialValues()
  * for details.
  */
class WidgetCalculator : public QFrame
{
Q_OBJECT
public:
	WidgetCalculator(QWidget* parent = nullptr);
	~WidgetCalculator();

	const QString result(void) const;

	/** \brief sets the character that is used as the separator.
	 *
	 * This method is used to set the character to be used as the separator
	 * between the integer and fractional part of an operand. Upon creation
	 * of the object, comma is set to the current locale setting.
	 *
	 * @param ch QChar representing the character to be used
	 */
	void setComma(const QChar ch) { comma = ch; }

	void setInitialValues(const QString& value, QKeyEvent* ev);
signals:
	/** \brief this signal is emitted, when a new result is available */
	void resultAvailable();

protected:
	void keyPressEvent(QKeyEvent* ev);
	QString normalizeString(const double& val);

protected slots:
	void digitClicked(int button);
	void calculationClicked(int button);
	void commaClicked(void);
	void plusminusClicked(void);
	void clearClicked(void);
	void clearAllClicked(void);
	void percentClicked(void);
	void changeDisplay(const QString& str);

private:
	QString operand; //!< stores the current (second) operand
	QString lastResult; //!< stores the last result
	QChar comma; //!< stores the representation of the character. (internaly always a period is used)
	double op0; //!< the numeric representation of a stacked first operand
	double op1; //!< the numeric representation of the first operand
	int op; //!< stores the operation to be performed between the first and the second operand.
	int stackedOp; //!< stores a pending addition operation
	QLabel *display; //!< stores a pointer to the display area

	/**
	 * this member array stores the pointers to the various
	 * buttons of the calculator. It is setup during the
	 * constructor of this object
	 */
	QPushButton *buttons[20];

	/** this enumeration type stores the values used for the various keys internally */
	enum {
		/* 0-9 are used by digits */
		COMMA = 10,
		/* make sure, that PLUS through EQUAL remain in
		 * the order they are. Otherwise, check the calculation
		 * signal mapper */
		PLUS,
		MINUS,
		SLASH,
		STAR,
		EQUAL,
		PLUSMINUS,
		PERCENT,
		CLEAR,
		CLEARALL,
		/* insert new buttons before this line */
		MAX_BUTTONS
	};

	/** this flag signals, if the operand should be replaced upon a digit
	 * key pressure. Defaults to false and will be set,
	 * if setInitialValues() is called without an operation.
	 */
	bool clearOperandOnDigit;
};

#endif // WIDGETCALCULATOR_H
