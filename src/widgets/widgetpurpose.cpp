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

#include "widgetpurpose.h"

#include <QtCore/QDebug>
#include <QtGui/QLayout>
#include <QtGui/QTextBlock>


/*! \bug
 wenn der Text:
	shirt orange, grafik schwarz	(28 Zeichen)
 eingegeben wird erfolgt KEIN automatischer Umbruch!!!
 wenn dagegen
	SHIRT ORANGE, GRAFIK SCHWARZ
 eingegeben wird erfolgt bei der eingabe von Z ein automatischer Umbruch!
 --> evt alles in Großbuchstaben wandeln!
*/

widgetPurpose::widgetPurpose(QWidget *parent) :
	QWidget(parent)
{
	this->textEdit = new QTextEdit(this);
	this->statusString = new QString(tr("(max %1 Zeilen, a %2 Zeichen) [%3 Zeichen in %4 Zeilen übrig]"));
	this->statusLabel = new QLabel(this);
	QFont labelFont(this->statusLabel->font());
	labelFont.setPointSize(7);
	this->statusLabel->setFont(labelFont);
	this->textEdit->setReadOnly(false);
	this->textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->maxLength = 27;
	this->maxLines = 4;
	this->updateStatusLabel();
	this->textEdit->setWordWrapMode(QTextOption::WordWrap);
	this->textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
	this->textEdit->setLineWrapColumnOrWidth(this->maxLength);

	this->textEdit->installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(this->textEdit);
	layout->addWidget(this->statusLabel);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(this->textEdit, SIGNAL(textChanged()),
		this, SLOT(plainTextEdit_TextChanged()));
}

widgetPurpose::~widgetPurpose()
{
	delete this->textEdit;
	delete this->statusString;
	delete this->statusLabel;
	qDebug() << this << "deleted";
}


//protected
/*! kontrolliert die möglichen Zeichen bei der TextEdit-Eingabe */
bool widgetPurpose::eventFilter(QObject *obj, QEvent *event)
{
	//wir behandeln nur keyPress Events vom TextEdit
	if (obj != this->textEdit) return false;
	if (event->type() != QEvent::KeyPress) return false;

	QKeyEvent *ev = dynamic_cast<QKeyEvent*>(event);
	//Nur Zeichen gemäß ZKA-Zeichensatz, aber auch Kleinbuchstaben, zulassen
	QRegExp regex("^[-+ .,/*&%0-9A-Za-z]$", Qt::CaseSensitive);

//	qDebug() << "Eingabe:" << ev->key() << "Zeichen:" << ev->text();

	if (regex.indexIn(ev->text()) != -1) { //Zeichen ist erlaubt!
		return false;
	}

	switch (ev->key()) {
	case Qt::Key_Backspace:
	case Qt::Key_Return:
	case Qt::Key_Delete:
	case Qt::Key_Enter:
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:
	case Qt::Key_End:
	case Qt::Key_Home:
	case Qt::Key_Insert:
	case Qt::Key_Control:	//for copy,paste,cut
	case Qt::Key_C:	//copy
	case Qt::Key_V:	//pase
	case Qt::Key_X:	//cut
		return false; //Zeichen ist Steuerzeichen und auch erlaubt
		break;
	}

	//Wenn wir hierher kommen ist das Zeichen nicht erlaubt.
	ev->setAccepted(false); //Eingabe unterbinden.
	return true; //Weitere Behandlung nicht erforderlich!
}

//private
void widgetPurpose::updateStatusLabel()
{
	this->statusLabel->setText(
			this->statusString->arg(
				this->maxLines).arg(
				this->maxLength).arg(
				(this->maxLines*this->maxLength)-
				this->textEdit->toPlainText().length()));

}

//public
QStringList widgetPurpose::getPurpose() const
{
	//Wir benötigen die einzelnen Zeilen des QTextEdits, so wie sie auch
	//dargestellt werden!
	// Ein Block beginnt nach einem Manuellen Zeilenumbruch (Enter) und die
	// einzelnen Zeilen in jedem Block beginnen wenn eine Zeile mehr als
	// this->maxLength Zeichen enthält.

	QStringList purposeLines;

	int blockCnt = this->textEdit->document()->blockCount();
	//qDebug() << "blockCnt:" << blockCnt;
	for (int block=0; block<blockCnt; ++block) {
		//qDebug() << "block:" << block;
		QTextBlock *textBlock = &this->textEdit->document()->findBlockByNumber(block);
		int lineCnt = textBlock->layout()->lineCount();
		//qDebug() << "lineCnt:" << lineCnt;
		for (int line=0; line<lineCnt; ++line) {
			int start = textBlock->layout()->lineAt(line).textStart();
			int len = textBlock->layout()->lineAt(line).textLength();
			//qDebug() << "line:" << line << "\t" << "start:" << start << " len:" << len;
			purposeLines << textBlock->text().mid(start, len).trimmed();
		}
	}

	qDebug() << "Purpose: " << purposeLines;
	return purposeLines;
}

//public
bool widgetPurpose::hasChanges() const
{
	return this->textEdit->document()->isModified();
}

//private slot
void widgetPurpose::plainTextEdit_TextChanged()
{
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setPurpose(const QString &text)
{
	this->textEdit->setPlainText(text);
}

//public slot
void widgetPurpose::setPurpose(const QStringList &text)
{
	this->setPurpose(text.join("\n"));
}

//public slot
void widgetPurpose::setLimitMaxLen(int maxLen)
{
	this->maxLength = maxLen;
	this->textEdit->setLineWrapColumnOrWidth(this->maxLength);
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setLimitMaxLines(int lines)
{
	this->maxLines = lines;
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setLimitAllowChange(int b)
{
	this->setDisabled(b == -1);
}

//public slot
void widgetPurpose::clearAll()
{
	this->textEdit->clear();
}
