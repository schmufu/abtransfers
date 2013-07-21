/*
   This file originates from litecoin-qt's source code to be found at https://github.com/litecoin-project/litecoin.
   litecoin-qt states an MIT/X11 license which according to http://www.gnu.org/licenses/license-list.html is
           "a lax permissive non-copyleft free software license, compatible with the GNU GPL"
   and http://en.wikipedia.org/wiki/License_compatibility#GPL_compatibility says that code under MIT/X11 license
           "can be combined with a program under the GPL without conflict
            (the new combination would have the GPL applied to the whole)."
*/

#ifndef MACDOCKICONHANDLER_H
#define MACDOCKICONHANDLER_H

#include <QtCore/QObject>

class QMenu;
class QIcon;
class QWidget;
class objc_object;

#ifdef __OBJC__
@class DockIconClickEventHandler;
#else
class DockIconClickEventHandler;
#endif

/** Macintosh-specific dock icon handler.
 */
class MacDockIconHandler : public QObject
{
    Q_OBJECT
public:
    ~MacDockIconHandler();

    QMenu *dockMenu();
    void setIcon(const QIcon &icon);

    static MacDockIconHandler *instance();

    void handleDockIconClickEvent();

signals:
    void dockIconClicked();

public slots:

private:
    MacDockIconHandler();

    DockIconClickEventHandler *m_dockIconClickEventHandler;
    QWidget *m_dummyWidget;
    QMenu *m_dockMenu;
};

#endif // MACDOCKICONCLICKHANDLER_H
