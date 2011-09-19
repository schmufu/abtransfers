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

#ifndef WIDGETPURPOSE_H
#define WIDGETPURPOSE_H

#include <QWidget>

#include <QtGui/QPlainTextEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QLabel>


class widgetPurpose : public QWidget
{
	Q_OBJECT
public:
	explicit widgetPurpose(QWidget *parent = 0);
	~widgetPurpose();

private:
	//QPlainTextEdit *plainEdit;
	QTextEdit *textEdit;
	const QString *statusString;
	QLabel *statusLabel;

	int maxLines;
	int maxLength;


	void updateStatusLabel();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

public:
	QStringList getPurpose() const;

signals:

private slots:
	void plainTextEdit_TextChanged();

public slots:
	void setPurpose(const QString &text);
	void setPurpose(const QStringList &text);

	void setLimitMaxLen(int maxLen);
	void setLimitMaxLines(int lines);
	void setLimitAllowChange(int b);

	void clearAll();
};

#endif // WIDGETPURPOSE_H
