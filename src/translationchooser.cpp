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
 *	a simple class for switching the supported languages at runtime.
 *	(inspired by i18n-example of Qt 4.8)
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "translationchooser.h"

#include <QTranslator>
#include <QString>
#include <QList>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QLibraryInfo>
#include <QMenu>
#include <QAction>



//not possible! Because the QApplication is not instantiated at this point in
//time. The private QString 'appFilename' is used instead and assigned at
//the Constructor.
//QT_STATIC_CONST QString TC_APPNAME = QFileInfo(QApplication::applicationFilePath()).fileName();

/** default language (tr() in the source is used with this language) */
QT_STATIC_CONST QString TC_DEFAULT_LANGUAGE = "Deutsch";
/** default locale for TC_DEFAULT_LANGUAGE */
QT_STATIC_CONST QString TC_DEFAULT_LOCALE = "de_DE";
/** default translation verison number (if none is supplied by the translator) */
QT_STATIC_CONST double TC_DEFAULT_VERSION_NR = 0.0;

/** directory in the resources under which the *.qm files are */
QT_STATIC_CONST QString TC_TRANS_RESOURCE_DIRNAME = "translation/";
/** addional directory for nearly every path */
QT_STATIC_CONST QString TC_TRANSLATION_DIRNAME = "translation/";
/** directory for system wide shared data (mainly used under Linux) */
QT_STATIC_CONST QString TC_SYSTEM_SHARED = "/usr/share/";
/** directory for Mac OS resources within an Application package
 *  (only supported unter Mac OS) */
QT_STATIC_CONST QString TC_RESOURCES_MAC = "/../Resources/";

/** convenient access to TC_TRANS_RESOURCE_DIRNAME */
QT_STATIC_CONST QString TC_TRANS_RESOURCE_STRING =
				QString(":/").append(TC_TRANS_RESOURCE_DIRNAME);

/** directory for translations within the users home directory */
QT_STATIC_CONST QString APP_SETTINGS_DIRNAME = "/.abtransfers/";



/** \brief data class that is only used within the TranslationChooser class.
 *
 * This class is only used by the TranslationChooser to group the information
 * about a translation together.
 *
 * It should not be used by any other classes!
 */
class TranslationChooserData
{
public:
	TranslationChooserData(const QString &filename = QString(),
			       const QString &languageName = QString(),
			       double translationVersion = 0.0,
			       const QString &localeName = QString());
	~TranslationChooserData();

	QString filename;
	QString languageName;
	double translationVersion;
	QString localeName;

	bool isValid() const;
};

TranslationChooserData::TranslationChooserData(const QString &filename,
					       const QString &languageName,
					       double translationVersion,
					       const QString &localeName)
{
	this->filename = filename;
	this->languageName = languageName;
	this->translationVersion = translationVersion;
	this->localeName = localeName;
}

TranslationChooserData::~TranslationChooserData()
{
	//not needed yet
}

/** \brief checks whether the data is valid or not
 *
 * \returns true if the translation file is available.
 */
bool TranslationChooserData::isValid() const
{
	return QFile::exists(this->filename);
}



/** \brief constructor with a QLocale to set at start.
 */
TranslationChooser::TranslationChooser(QLocale locale /* = QLocale() */,
				       QObject *parent /* = NULL */ ) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "called";

	this->appFilename = QFileInfo(qApp->applicationFilePath()).fileName();
	this->activeTranslators.clear();
	this->langMenu = NULL;

	this->loadSupportedTranslations();
	this->createLanguageMenu();

	this->setLanguage(locale);
}

/** \brief constructor with a QString for the language name to set at start.
 */
TranslationChooser::TranslationChooser(QString language /* = QString() */,
				       QObject *parent /* = NULL */ ) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "called";

	this->appFilename = QFileInfo(qApp->applicationFilePath()).fileName();
	this->activeTranslators.clear();
	this->activeLanguageName = ""; //is updated by setLanguage();
	this->langMenu = NULL;

	this->loadSupportedTranslations();
	this->createLanguageMenu();

	this->setLanguage(language);
}

/** \brief uninstalls all installed QTranslators
 *
 * Also all created supported translations are freed.
 */
TranslationChooser::~TranslationChooser()
{
	qDebug() << Q_FUNC_INFO << "called";

	this->uninstallAllTranslators();

	foreach (QString key, this->supportedTranslations.keys()) {
		delete this->supportedTranslations.take(key);
	}

	delete this->langMenu;
}

//private
/** \brief searches the different locations for .qm files
 *
 * and adds them to the supported translations.
 */
void TranslationChooser::loadSupportedTranslations()
{
	QStringList qmLocations = this->qmFileLocations();

	foreach (QString location, qmLocations) {
		qDebug() << "Translations -"
			 << "searching:" << location;
		QDir dir(location);
		QString nameFilter = this->appFilename + QString("*.qm");
		QStringList fileNames = dir.entryList(QStringList(nameFilter),
						      QDir::Files, QDir::Name);

		QMutableStringListIterator i(fileNames);
		while (i.hasNext()) {
			i.next();
			QString qmFile = dir.filePath(i.value());
			this->addTranslation(qmFile);
		}
	}
}

//private
/** \brief add the supplied qmFile to the supported translations
 *
 * The \a qmFile is not added to the supported translations when the file
 * does not exists or when the language name is not set within the translation
 * file.
 *
 * Otherwise the \a qmFile is added. When a translation is already at the
 * support translations, then the \a qmFile is only added if it has a newer
 * version of the translation (the older version is removed).
 */
void TranslationChooser::addTranslation(const QString &qmFile)
{
	TranslationChooserData *tData = this->translationData(qmFile);

	if (tData == NULL || !tData->isValid()) {
		delete tData; //its save to delete NULL
		return;
	}

	const QString langName = tData->languageName;

	if (langName.isEmpty()) {
		qWarning() << "Translations -" << "\t"
			   << "Language name not set, could not use:"
			   << qmFile;
		delete tData;
		return;
	}

	TranslationChooserData *known;
	known = this->supportedTranslations.value(langName, NULL);
	if (known != NULL) {
		if (known->translationVersion >= tData->translationVersion) {
			delete tData;
			return; //nothing new
		} else {
			//the known translation is 'older' than the new one, we
			//remove the old. Therefore the new one could be used.
			qDebug() << "Translations -" << "\t"
				 << "found newer version for" << langName;
			this->supportedTranslations.remove(langName);
			delete known;
		}
	}

	//translation is unknown, add the qmFile to supportedTranslations

	qDebug() << "Translations -" << "\t"
		 << "using" << qmFile
		 << "for" << langName << "translations";

	this->supportedTranslations.insert(langName, tData);
}

//private
/** \brief returns a list with all locations that should be searched for qm files
 */
QStringList TranslationChooser::qmFileLocations() const
{
	QStringList locations;
	locations.clear();
	const QString appPath = qApp->applicationDirPath();
	const QString appName = this->appFilename;

	//files in the resources file
	locations.append(TC_TRANS_RESOURCE_STRING);

	//e.g.: /usr/share/APPNAME/
	locations.append(TC_SYSTEM_SHARED + appName + QString("/"));;
	 //e.g.: /usr/share/APPNAME/translation
	locations.append(TC_SYSTEM_SHARED + appName + QString("/") + TC_TRANSLATION_DIRNAME);

	locations.append(appPath); //app dir
	locations.append(appPath + QString("/") + TC_TRANSLATION_DIRNAME);

#if defined(Q_OS_MAC)
	locations.append(appPath + TC_RESOURCES_MAC);
	locations.append(appPath + TC_RESOURCES_MAC + TC_TRANSLATION_DIRNAME);
#endif

	//for convenience, the settings directory of the user
	locations.append(QDir::homePath() + APP_SETTINGS_DIRNAME);
	locations.append(QDir::homePath() + APP_SETTINGS_DIRNAME + TC_TRANSLATION_DIRNAME);

	return locations;
}

//static private
/** \brief returns the name of the language (in native spelling)
 *
 * The translator must set the translation for the string "LANGUAGE" to the
 * name of the language the translation is for.
 */
QString TranslationChooser::languageName(const QString &qmFile)
{
	QTranslator translator;
	translator.load(qmFile);

	//: The name of the language (in native spelling).
	//: This text is used for the menu entry name for this translation.
	return translator.translate("TranslationChooser", "LANGUAGE");
}

//static private
/** \brief returns the verion number supplied by the translator
 *
 * The translator must set the translation for the string "LANG_VERSION" to
 * the version number of the translation file.
 */
double TranslationChooser::languageVersion(const QString &qmFile)
{
	QTranslator translator;
	translator.load(qmFile);

	double version;
	bool convOK;
	//: The version of the translation. Must be convertible to a double!
	//: This number is used to distinguish between different translations
	//: for the same language. (The one with higher number is used)
	version = translator.translate("TranslationChooser", "LANG_VERSION").toDouble(&convOK);
	if (!convOK) {
		//conversion failed, possible if no version number supplied
		version = TC_DEFAULT_VERSION_NR; //assume default version number
	}

	return version;
}

//private
/** \brief parses the locale from the supplied filename
 *
 * \a returns "en_US" when \a qmFile with "program_en_US.qm" is supplied
 * \a returns "de" when \a qmFile with "program_de.qm" is supplied
 *
 * It does not matter if the full path or only the filename is passed to
 * \a qmFile, and the separation between the program name and the following
 * locale could be done by every character (but must be present).
 *
 * All the following Strings are possible and "de" would be returned:
 *	\li /usr/bin/program_de.qm
 *	\li /usr/bin/program.de.qm
 *	\li /bin/program-de.qm
 *	\li program-de.QM
 */
QString TranslationChooser::localeName(const QString &qmFile) const
{
	QString localeStr;
	localeStr = QFileInfo(qmFile).fileName().remove(this->appFilename);
	localeStr = localeStr.remove(".qm", Qt::CaseInsensitive);
	//there must be _one_ separation between program name and locale
	localeStr = localeStr.remove(0, 1);
	return localeStr;
}

//private
/** \brief creates a menu with all supported languages
 */
void TranslationChooser::createLanguageMenu()
{
	if (this->langMenu)
		delete this->langMenu;

	this->langMenu = new QMenu();
	QActionGroup *actGroup = new QActionGroup(this->langMenu);
	actGroup->setExclusive(true);

	foreach (const QString langName, this->supportedLanguages()) {
		QAction *action = actGroup->addAction(langName);
		action->setCheckable(true);
	}

	this->langMenu->addActions(actGroup->actions());

	connect(actGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(actionTriggered(QAction*)));
}

//private
/** \brief creates all relevant data for the supplied qmFile
 *
 * \attention The caller is responsible for deleting the returned pointer!
 *
 * NULL is returned when the file does not exist.
 */
TranslationChooserData *TranslationChooser::translationData(const QString &qmFile) const
{
	if (!QFile::exists(qmFile))
		return NULL;

	TranslationChooserData *transData = new TranslationChooserData();

	transData->filename = qmFile;
	transData->languageName = this->languageName(qmFile);
	transData->localeName = this->localeName(qmFile);
	transData->translationVersion = this->languageVersion(qmFile);

	return transData;
}

//private
/** \brief removes all active translations
 */
void TranslationChooser::uninstallAllTranslators()
{
	while (!this->activeTranslators.isEmpty()) {
		QTranslator *translator = this->activeTranslators.takeLast();
		qApp->removeTranslator(translator);
		delete translator;
	}
}

//public
/** \brief returns all supported languages (in native spelling).
 */
QStringList TranslationChooser::supportedLanguages()
{
	QStringList supported;
	//the default language should also be included
	if (!this->supportedTranslations.contains(TC_DEFAULT_LANGUAGE))
		supported.append(TC_DEFAULT_LANGUAGE);

	foreach (const TranslationChooserData *data, this->supportedTranslations.values())
		supported.append(data->languageName);

	return supported;
}

//public slot
/** \brief installs the translations for the given language
 */
void TranslationChooser::setLanguage(const QString &language)
{
	QString qtLocale = "";
	QString qmFile = "";
	const TranslationChooserData *tData;
	tData = this->supportedTranslations.value(language, NULL);

	//if the default language is selected, we only install a translation
	//if a translation file is available. Nevertheless we try to install
	//the default qt locale (Otherwise Strings from Qt would be in
	//English if German is the default language).

	if (tData == NULL && (language.toLower() != TC_DEFAULT_LANGUAGE.toLower())) {
		qWarning() << Q_FUNC_INFO
			   << "translations to" << language << "not supported!";
		return; //nothing to install
	}

	if (tData == NULL && (language.toLower() == TC_DEFAULT_LANGUAGE.toLower())) {
		qtLocale = TC_DEFAULT_LOCALE;
	}

	if (tData != NULL) {
		qtLocale = tData->localeName;
		qmFile = tData->filename;
	}

	this->activeLanguageName = language;

	this->uninstallAllTranslators();

	//Install translation for build in Qt Strings
	QTranslator *qtTranslator = new QTranslator();
	if (!qtTranslator->load("qt_" + qtLocale,
				QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		if (!qtLocale.startsWith("en")) {
			//the locale en is built in qt, so this could never be loaded
			qWarning() << Q_FUNC_INFO << "could not load qt"
				   << "translations for locale" << qtLocale;
		}
		delete qtTranslator;
	} else {
		qDebug() << Q_FUNC_INFO
			 << "installing qt translations for" << qtLocale;
		qApp->installTranslator(qtTranslator);
		this->activeTranslators.append(qtTranslator);
	}

	//Install translations for the application (if file exists)
	if (!qmFile.isEmpty()) {
		QTranslator *translator = new QTranslator();
		if (!translator->load(qmFile)) {
			qWarning() << Q_FUNC_INFO << "loading translations from"
				   << qmFile << "failed";
			delete translator;
		} else {
			qDebug() << Q_FUNC_INFO
				 << "installing translations from" << qmFile;
			qApp->installTranslator(translator);
			this->activeTranslators.append(translator);
		}
	}

	emit this->languageChanged(language);
}

//public slot
/** \brief installs the translations for the given locale
 *
 * \overload
 *
 * gets the locale name from the \a locale and searches all supported
 * translations for that locale name.
 *
 * The locale name is truncated at every "_" and the result is also searched.
 * So for "en_US", first "en_US" is searched and than "en".
 *
 * If a supported translation is found for the given locale, the search is
 * stopped and the function setLanguage(QString language) is called with
 * the supported language.
 */
void TranslationChooser::setLanguage(const QLocale &locale)
{
	QString language = "";
	QString localeStr = locale.name().toLower();

	QStringList localeStrList;
	localeStrList.append(localeStr);
	while (localeStr.contains("_")) {
		int dashPos = localeStr.lastIndexOf("_");
		localeStr.truncate(dashPos);
		localeStrList.append(localeStr);
	}

	foreach (const TranslationChooserData *data, this->supportedTranslations.values()) {
		foreach (const QString locStr, localeStrList) {
			if (data->localeName.toLower() == locStr) {
				language = data->languageName;
				break; //language found
			}
		}
		if (!language.isEmpty())
			break; //possible language found, cancel first foreach
	}

	this->setLanguage(language);
}

//private slot
/** \brief slot for the QActions supplied by the languageMenu()
 */
void TranslationChooser::actionTriggered(QAction *action)
{
	if (!action)
		return;

	this->setLanguage(action->text());
}

//public
/** \brief returns a menu with all supported translations
 *
 * All included actions are connected to the TranslationChooser class and
 * can switch the language without any further connections.
 */
QMenu *TranslationChooser::languageMenu() const
{
	return this->langMenu;
}

//public
/** \brief returns the currently used language name
 */
const QString &TranslationChooser::currentLanguage()
{
	return this->activeLanguageName;
}
