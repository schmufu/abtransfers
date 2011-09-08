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

#ifndef WIDGETTEXTKEY_H
#define WIDGETTEXTKEY_H

#include <QWidget>

class QComboBox;
class QLabel;

class widgetTextKey : public QWidget
{
	Q_OBJECT
public:
	explicit widgetTextKey(const QList<int> *keys = NULL, QWidget *parent = 0);
	~widgetTextKey();

	int getTextKey() const;

protected:
//	void changeEvent(QEvent *e);

private:
	QComboBox *comboBox;
	QLabel *label;

signals:

public slots:
	void setTextKey(int key);
	void fillTextKeys(const QList<int> *keys);

	void setLimitAllowChange(bool b);

};

#endif // WIDGETTEXTKEY_H