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

#ifndef EXTRATRANSFERWIDGET_H
#define EXTRATRANSFERWIDGET_H

#include <QFrame>

namespace Ui {
	class extraTransferWidget;
}

class extraTransferWidget : public QFrame {
	Q_OBJECT
public:
	extraTransferWidget(QWidget *parent = 0);
	~extraTransferWidget();

	int getTextKey() const;

protected:
	void changeEvent(QEvent *e);

private:
	Ui::extraTransferWidget *ui;

	void fillTextKeys();

public slots:
	void setTextKey(int key);

};

#endif // EXTRATRANSFERWIDGET_H