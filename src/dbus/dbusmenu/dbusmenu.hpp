#pragma once

#include <qbytearray.h>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qhash.h>
#include <qloggingcategory.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qqmllist.h>
#include <qquickimageprovider.h>
#include <qtmetamacros.h>
#include <qtypes.h>

#include "../../core/imageprovider.hpp"
#include "../../core/qsmenu.hpp"
#include "../properties.hpp"
#include "dbus_menu_types.hpp"

Q_DECLARE_LOGGING_CATEGORY(logDbusMenu);

class DBusMenuInterface;

namespace qs::dbus::dbusmenu {

// hack because docgen can't take namespaces in superclasses
using menu::QsMenuEntry;

class DBusMenu;
class DBusMenuPngImage;

///! Menu item shared by an external program.
/// Menu item shared by an external program via the
/// [DBusMenu specification](https://github.com/AyatanaIndicators/libdbusmenu/blob/master/libdbusmenu-glib/dbus-menu.xml).
class DBusMenuItem: public QsMenuEntry {
	Q_OBJECT;
	/// Handle to the root of this menu.
	Q_PROPERTY(DBusMenu* menuHandle READ menuHandle CONSTANT);
	QML_ELEMENT;
	QML_UNCREATABLE("DBusMenus can only be acquired from a DBusMenuHandle");

public:
	explicit DBusMenuItem(qint32 id, DBusMenu* menu, DBusMenuItem* parentMenu);

	/// Refreshes the menu contents.
	///
	/// Usually you shouldn't need to call this manually but some applications providing
	/// menus do not update them correctly. Call this if menus don't update their state.
	///
	/// The @@layoutUpdated(s) signal will be sent when a response is received.
	Q_INVOKABLE void updateLayout() const;

	[[nodiscard]] DBusMenu* menuHandle() const;

	[[nodiscard]] bool isSeparator() const override;
	[[nodiscard]] bool enabled() const override;
	[[nodiscard]] QString text() const override;
	[[nodiscard]] QString icon() const override;
	[[nodiscard]] menu::QsMenuButtonType::Enum buttonType() const override;
	[[nodiscard]] Qt::CheckState checkState() const override;
	[[nodiscard]] bool hasChildren() const override;

	[[nodiscard]] bool isShowingChildren() const;
	void setShowChildrenRecursive(bool showChildren);

	[[nodiscard]] QQmlListProperty<menu::QsMenuEntry> children() override;

	void updateProperties(const QVariantMap& properties, const QStringList& removed = {});
	void onChildrenUpdated();

	qint32 id = 0;
	QString mText;
	QVector<qint32> mChildren;
	bool mShowChildren = false;
	bool childrenLoaded = false;
	DBusMenu* menu = nullptr;

signals:
	void layoutUpdated();

private slots:
	void sendOpened() const;
	void sendClosed() const;
	void sendTriggered() const;

private:
	QString mCleanLabel;
	//QChar mnemonic;
	bool mEnabled = true;
	bool visible = true;
	bool mSeparator = false;
	QString iconName;
	DBusMenuPngImage* image = nullptr;
	menu::QsMenuButtonType::Enum mButtonType = menu::QsMenuButtonType::None;
	Qt::CheckState mCheckState = Qt::Unchecked;
	bool displayChildren = false;
	QVector<qint32> enabledChildren;
	DBusMenuItem* parentMenu = nullptr;

	static qsizetype childrenCount(QQmlListProperty<menu::QsMenuEntry>* property);
	static menu::QsMenuEntry* childAt(QQmlListProperty<menu::QsMenuEntry>* property, qsizetype index);
};

QDebug operator<<(QDebug debug, DBusMenuItem* item);

///! Handle to a DBusMenu tree.
/// Handle to a menu tree provided by a remote process.
class DBusMenu: public QObject {
	Q_OBJECT;
	Q_PROPERTY(DBusMenuItem* menu READ menu CONSTANT);
	QML_NAMED_ELEMENT(DBusMenuHandle);
	QML_UNCREATABLE("Menu handles cannot be directly created");

public:
	explicit DBusMenu(const QString& service, const QString& path, QObject* parent = nullptr);

	dbus::DBusPropertyGroup properties;
	dbus::DBusProperty<quint32> version {this->properties, "Version"};
	dbus::DBusProperty<QString> textDirection {this->properties, "TextDirection", "", false};
	dbus::DBusProperty<QString> status {this->properties, "Status"};
	dbus::DBusProperty<QStringList> iconThemePath {this->properties, "IconThemePath", {}, false};

	void prepareToShow(qint32 item, qint32 depth);
	void updateLayout(qint32 parent, qint32 depth);
	void removeRecursive(qint32 id);
	void sendEvent(qint32 item, const QString& event);

	DBusMenuItem rootItem {0, this, nullptr};
	QHash<qint32, DBusMenuItem*> items {std::make_pair(0, &this->rootItem)};

	[[nodiscard]] DBusMenuItem* menu();

private slots:
	void onLayoutUpdated(quint32 revision, qint32 parent);
	void onItemPropertiesUpdated(
	    const DBusMenuItemPropertiesList& updatedProps,
	    const DBusMenuItemPropertyNamesList& removedProps
	);

private:
	void updateLayoutRecursive(const DBusMenuLayout& layout, DBusMenuItem* parent, qint32 depth);

	DBusMenuInterface* interface = nullptr;
};

QDebug operator<<(QDebug debug, DBusMenu* menu);

class DBusMenuPngImage: public QsImageHandle {
public:
	explicit DBusMenuPngImage(QByteArray data, DBusMenuItem* parent)
	    : QsImageHandle(QQuickImageProvider::Image, parent)
	    , data(std::move(data)) {}

	QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

	QByteArray data;
};

} // namespace qs::dbus::dbusmenu
