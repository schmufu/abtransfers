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



/** The application name. When TC_APPNAME is empty the filename of the
 *  application executeable is used (TARGET from .pro file), otherwise the
 *  name defined here.
 */
static const QString TC_APPNAME = QString::fromUtf8("abtransfers");

/** default language (tr() in the source is used with this language) */
static const QString TC_DEFAULT_LANGUAGE = QString::fromUtf8("Deutsch");
/** default locale for TC_DEFAULT_LANGUAGE */
static const QString TC_DEFAULT_LOCALE = QString::fromUtf8("de_DE");
/** default translation verison number (if none is supplied by the translator) */
static const double TC_DEFAULT_VERSION_NR = 0.0;

/** directory in the resources under which the *.qm files are */
static const QString TC_TRANS_RESOURCE_DIRNAME = QString::fromUtf8("translation/");
/** addional directory for nearly every path */
static const QString TC_TRANSLATION_DIRNAME = QString::fromUtf8("translation/");
/** directory for system wide shared data (mainly used under Linux) */
static const QString TC_SYSTEM_SHARED = QString::fromUtf8("/usr/share/");
/** directory for Mac OS resources within an Application package
 *  (only supported unter Mac OS) */
static const QString TC_RESOURCES_MAC = QString::fromUtf8("/../Resources/");

/** convenient access to TC_TRANS_RESOURCE_DIRNAME */
static const QString TC_TRANS_RESOURCE_STRING =
		QString::fromUtf8(":/").append(TC_TRANS_RESOURCE_DIRNAME);

/** directory for translations within the users home directory */
static const QString APP_SETTINGS_DIRNAME = QString::fromUtf8("/.abtransfers/");


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
			       const QString &localeName = QString(),
			       const QString &appVersion = QString());
	~TranslationChooserData();

	QString filename;
	QString languageName;
	double translationVersion;
	QString translationAppVersion;
	QString localeName;

	bool isValid() const;
};

TranslationChooserData::TranslationChooserData(const QString &filename,
					       const QString &languageName,
					       double translationVersion,
					       const QString &localeName,
					       const QString &appVersion)
{
	this->filename = filename;
	this->languageName = languageName;
	this->translationVersion = translationVersion;
	this->localeName = localeName;
	this->translationAppVersion = appVersion;
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



/** \brief constructor with a QLocale to set
 */
TranslationChooser::TranslationChooser(QLocale locale /* = QLocale() */,
				       QObject *parent /* = NULL */ ) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "called";

	if (TC_APPNAME.isEmpty())
		this->appFilename = QFileInfo(qApp->applicationFilePath()).fileName();
	else
		this->appFilename = TC_APPNAME;

	this->activeTranslators.clear();
	this->langMenu = NULL;

	//the calling order is important!
	this->loadSupportedTranslations();
	this->setLanguage(locale);
	this->createLanguageMenu();
}

/** \brief constructor with a QString for the language name to set
 */
TranslationChooser::TranslationChooser(QString language /* = QString() */,
				       QObject *parent /* = NULL */ ) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "called";

	if (TC_APPNAME.isEmpty())
		this->appFilename = QFileInfo(qApp->applicationFilePath()).fileName();
	else
		this->appFilename = TC_APPNAME;

	this->activeTranslators.clear();
	this->activeLanguageName = QString::fromUtf8(""); //updated by setLanguage();
	this->langMenu = NULL;

	//the calling order is important!
	this->loadSupportedTranslations();
	this->setLanguage(language);
	this->createLanguageMenu();
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
	QStringList qmLocations = this->fileLocations();

	foreach (QString location, qmLocations) {
		qDebug() << "Translations -"
			 << "searching:" << location;
		QDir dir(location);
		QString nameFilter = this->appFilename + QString::fromUtf8("*.qm");
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
QStringList TranslationChooser::fileLocations() const
{
	QStringList locations;
	locations.clear();
	const QString appPath = qApp->applicationDirPath();
	const QString appName = this->appFilename;

	//files in the resources file
	locations.append(TC_TRANS_RESOURCE_STRING);

	//e.g.: /usr/share/APPNAME/
	locations.append(TC_SYSTEM_SHARED + appName + QString::fromUtf8("/"));;
	 //e.g.: /usr/share/APPNAME/translation
	locations.append(TC_SYSTEM_SHARED + appName + QString::fromUtf8("/") +
			 TC_TRANSLATION_DIRNAME);

	locations.append(appPath); //app dir
	locations.append(appPath + QString::fromUtf8("/") +
			 TC_TRANSLATION_DIRNAME);

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

//static private
/** \brief returns the version number for which this translation should be used.
 *
 * The translator must set the "APP_VERSION" to the program version for which
 * the translation should be used. This version number is returned by this
 * function.
 */
QString TranslationChooser::languageAppVersion(const QString &qmFile)
{
	QTranslator translator;
	translator.load(qmFile);

	QString appVersion;
	//: The version of the application for which this translation is.
	//: This number is used to verify that the translation is for the
	//: used version of the application.
	appVersion = translator.translate("TranslationChooser", "APP_VERSION");

	if (appVersion == QString::fromUtf8("APP_VERSION")) {
		//translator did not set any version
		appVersion = QString(); //use an empty strings
	}

	return appVersion;
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
	localeStr = localeStr.remove(QString::fromUtf8(".qm"), Qt::CaseInsensitive);
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
		action->setChecked(langName == this->activeLanguageName);
	}

	this->langMenu->addActions(actGroup->actions());

	connect(actGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(actionTriggered(QAction*)));
}

//private
/** \brief sets the translations for Qt strings to \a locale
 */
void TranslationChooser::installQtTranslation(const QString &locale)
{
	QTranslator *qtTranslator = new QTranslator();
	QString libDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	if (!qtTranslator->load(QString::fromUtf8("qt_") + locale, libDir)) {
		if (!locale.startsWith(QString::fromUtf8("en"))) {
			//the locale en is built in qt, this could not be loaded
			qWarning() << Q_FUNC_INFO << "could not load qt"
				   << "translations for locale" << locale;
		}
		delete qtTranslator;
	} else { //load successfull
		qDebug() << Q_FUNC_INFO
			 << "installing qt translations for" << locale;
		qApp->installTranslator(qtTranslator);
		this->activeTranslators.append(qtTranslator);
	}
}

//private
/** \brief loads the translations for the application from \a qmFile (if existent)
 */
void TranslationChooser::installAppTranslation(const QString &qmFile)
{
	if (qmFile.isEmpty())
		return; //translation not possible

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
	transData->translationAppVersion = this->languageAppVersion(qmFile);

	return transData;
}

//private
/** \brief returns the TranslationChooserData for the currently active
 *	   translation.
 *
 * If there is no TranslationChooserData for the current language, NULL is
 * returned.
 */
const TranslationChooserData *TranslationChooser::activeTranslationChooserData() const
{
	const QString langName = this->currentLanguage();
	const TranslationChooserData *tdata =
			this->supportedTranslations.value(langName, NULL);

	return tdata;
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
QStringList TranslationChooser::supportedLanguages() const
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
 *
 * If the default language is supplied, a translation is only performed if a
 * translation file is available. Nevertheless the Qt translations are taking
 * place (Otherwise Strings from Qt would be in English if German is the
 * default language).
 */
void TranslationChooser::setLanguage(const QString &language)
{
	QString qtLocale = QString();
	QString qmFile = QString();
	const TranslationChooserData *tData;
	tData = this->supportedTranslations.value(language, NULL);

	bool defaultLang = language.toLower() == TC_DEFAULT_LANGUAGE.toLower();

	if (tData) {
		qtLocale = tData->localeName;
		qmFile = tData->filename;
	} else { //tData == NULL
		if (defaultLang) {
			qtLocale = TC_DEFAULT_LOCALE;
		} else {
			qWarning() << Q_FUNC_INFO << "translations to"
				   << language << "not supported!";
			return; //nothing to install
		}
	}

	this->activeLanguageName = language;

	this->uninstallAllTranslators();

	this->installQtTranslation(qtLocale);
	this->installAppTranslation(qmFile);

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
 *
 * If no locale string matches the next try is to match against the language
 * code from the locale. For 'en_US' this is 'en'. If a supported language file
 * name locale start with 'en', then this file is used for translation.
 * This mostly only takes places when no language is selected by the user
 * (first start) or when this function is explicitly called.
 */
void TranslationChooser::setLanguage(const QLocale &locale)
{
	QString language = TC_DEFAULT_LANGUAGE;
	QString localeStr = locale.name().toLower();
	QString languageCode;

	qDebug() << Q_FUNC_INFO << "localeStr =" << localeStr;

	QStringList localeStrList;
	localeStrList.append(localeStr);
	while (localeStr.contains(QString::fromUtf8("_"))) {
		int pos = localeStr.lastIndexOf(QString::fromUtf8("_"));
		localeStr.truncate(pos);
		qDebug() << Q_FUNC_INFO << "localeStr =" << localeStr;
		localeStrList.append(localeStr);
	}

	foreach (const QString locStr, localeStrList) {
		//search all xx_XX names first, then the truncated
		foreach (const TranslationChooserData *data,
			 this->supportedTranslations.values()) {
			if (data->localeName.toLower() == locStr) {
				language = data->languageName;
				goto LANGUAGE_FOUND; //break both foreach
			}
		}
	}

	//No exact translation found. Try to find a locale that starts with
	//the language code (the first match is used)
	languageCode = localeStrList.last(); //last is the shortest!
	foreach (const TranslationChooserData *data,
		 this->supportedTranslations.values()) {
		if (data->localeName.startsWith(languageCode)) {
			language = data->languageName;
			goto LANGUAGE_FOUND;
		}
	}

	LANGUAGE_FOUND:
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
const QString &TranslationChooser::currentLanguage() const
{
	return this->activeLanguageName;
}

//public
/** \brief returns the version of the translation file.
 */
const QString TranslationChooser::currentLanguageVersion() const
{
	const TranslationChooserData *tdata = this->activeTranslationChooserData();

	if (!tdata)
		return QString();

	return QString::fromUtf8("%1").arg(tdata->translationVersion);
}

//public
/** \brief returns the application version for which this translation should
 *         be used.
 *
 * This version string could be compared with the currently running application
 * version. If they are different the loaded translation could be invalid on
 * some string translations.
 */
const QString TranslationChooser::currentLanguageAppVersion() const
{
	const TranslationChooserData *tdata = this->activeTranslationChooserData();

	if (!tdata)
		return QString();

	return tdata->translationAppVersion;
}

//public
/** \brief returns the currently used path and filename of the .qm file
 */
const QString TranslationChooser::currentLanguageFile() const
{
	const TranslationChooserData *tdata = this->activeTranslationChooserData();

	if (!tdata)
		return QString();

	return tdata->filename;
}


#if defined(TRANSLATIONCHOOSER_ENABLE_HELPTEXT)
//public
/** \brief returns the help text filename for the selected language
 *
 * If no filename was set by the translator the default (german) is returned.
 */
QString TranslationChooser::helpTextFilename() const
{
	static const QString defVal(TC_TRANS_RESOURCE_STRING +
				    QString::fromUtf8("abtransfers-helptext_de.html"));

	//: The filename of the help text.
	//: The same directories as for the qm-files are searched!
	QString helpFilename = tr("HELPTEXTFILENAME");

	if (helpFilename == QString::fromUtf8("HELPTEXTFILENAME")) {
		//no translation supplied, use default (german)
		return defVal;
	}

	//search backwards through the allowed locations
	QStringList locations = this->fileLocations();
	for (int i=locations.size()-1; i>=0; i--) {
		QDir dir(locations.at(i));
		QStringList helpFiles = dir.entryList(QStringList(helpFilename),
						      QDir::Files, QDir::Name);
		//return the first file found
		if (!helpFiles.isEmpty()) {
			return dir.filePath(helpFiles.first());
		}
	}

	//if no help was found we return the default
	return defVal;
}
#endif
