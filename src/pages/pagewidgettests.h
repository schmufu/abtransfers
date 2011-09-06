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
 *	Nur ein TestWidget um die wiget... klassen zu testen
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef PAGEWIDGETTESTS_H
#define PAGEWIDGETTESTS_H

#include <QWidget>
#include "../widgets/widgetlineeditwithlabel.h"

/*! \brief NUR FÜR TESTZECKE!
 *
 * Diese Klasse wird nur genutzt um widgets zu testen, im eigentlichen
 * Programm wird Sie nicht verwendet!
 */
class pageWidgetTests : public QWidget
{
	Q_OBJECT
public:
	explicit pageWidgetTests(QWidget *parent = 0);
	~pageWidgetTests();

	widgetLineEditWithLabel *lineEdit1;
	widgetLineEditWithLabel *lineEdit2;
	widgetLineEditWithLabel *lineEdit3;
	widgetLineEditWithLabel *lineEdit4;

signals:

public slots:

};

#endif // PAGEWIDGETTESTS_H
