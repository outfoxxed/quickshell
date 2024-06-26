#pragma once

#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qqmllist.h>
#include <qtmetamacros.h>
#include <qtypes.h>

#include "doc.hpp"

namespace qs::menu {

class QsMenuButtonType: public QObject {
	Q_OBJECT;
	QML_ELEMENT;
	QML_SINGLETON;

public:
	enum Enum {
		/// This menu item does not have a checkbox or a radiobutton associated with it.
		None = 0,
		/// This menu item should draw a checkbox.
		CheckBox = 1,
		/// This menu item should draw a radiobutton.
		RadioButton = 2,
	};
	Q_ENUM(Enum);

	Q_INVOKABLE static QString toString(QsMenuButtonType::Enum value);
};

class QsMenuEntry: public QObject {
	Q_OBJECT;
	/// If this menu item should be rendered as a separator between other items.
	///
	/// No other properties have a meaningful value when `isSeparator` is true.
	Q_PROPERTY(bool isSeparator READ isSeparator NOTIFY isSeparatorChanged);
	Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged);
	/// Text of the menu item.
	Q_PROPERTY(QString text READ text NOTIFY textChanged);
	/// Url of the menu item's icon or `""` if it doesn't have one.
	///
	/// This can be passed to [Image.source](https://doc.qt.io/qt-6/qml-qtquick-image.html#source-prop)
	/// as shown below.
	///
	/// ```qml
	/// Image {
	///   source: menuItem.icon
	///   // To get the best image quality, set the image source size to the same size
	///   // as the rendered image.
	///   sourceSize.width: width
	///   sourceSize.height: height
	/// }
	/// ```
	Q_PROPERTY(QString icon READ icon NOTIFY iconChanged);
	/// If this menu item has an associated checkbox or radiobutton.
	Q_PROPERTY(QsMenuButtonType::Enum buttonType READ buttonType NOTIFY buttonTypeChanged);
	/// The check state of the checkbox or radiobutton if applicable, as a
	/// [Qt.CheckState](https://doc.qt.io/qt-6/qt.html#CheckState-enum).
	Q_PROPERTY(Qt::CheckState checkState READ checkState NOTIFY checkStateChanged);
	/// If this menu item has children that can be accessed through a [QsMenuOpener](../qsmenuopener).
	Q_PROPERTY(bool hasChildren READ hasChildren NOTIFY hasChildrenChanged);
	QML_ELEMENT;
	QML_UNCREATABLE("QsMenuEntry cannot be directly created");

public:
	explicit QsMenuEntry(QObject* parent = nullptr): QObject(parent) {}

	/// Display a platform menu at the given location relative to the parent window.
	Q_INVOKABLE void display(QObject* parentWindow, qint32 relativeX, qint32 relativeY);

	[[nodiscard]] virtual bool isSeparator() const { return false; }
	[[nodiscard]] virtual bool enabled() const { return true; }
	[[nodiscard]] virtual QString text() const { return ""; }
	[[nodiscard]] virtual QString icon() const { return ""; }
	[[nodiscard]] virtual QsMenuButtonType::Enum buttonType() const { return QsMenuButtonType::None; }
	[[nodiscard]] virtual Qt::CheckState checkState() const { return Qt::Unchecked; }
	[[nodiscard]] virtual bool hasChildren() const { return false; }

	void ref();
	void unref();

	[[nodiscard]] virtual QQmlListProperty<QsMenuEntry> children();

	static QQmlListProperty<QsMenuEntry> emptyChildren(QObject* parent);

signals:
	/// Send a trigger/click signal to the menu entry.
	void triggered();

	QSDOC_HIDE void opened();
	QSDOC_HIDE void closed();

	void isSeparatorChanged();
	void enabledChanged();
	void textChanged();
	void iconChanged();
	void buttonTypeChanged();
	void checkStateChanged();
	void hasChildrenChanged();
	QSDOC_HIDE void childrenChanged();

private:
	static qsizetype childCount(QQmlListProperty<QsMenuEntry>* property);
	static QsMenuEntry* childAt(QQmlListProperty<QsMenuEntry>* property, qsizetype index);

	qsizetype refcount = 0;
};

///! Provides access to children of a QsMenuEntry
class QsMenuOpener: public QObject {
	Q_OBJECT;
	/// The menu to retrieve children from.
	Q_PROPERTY(QsMenuEntry* menu READ menu WRITE setMenu NOTIFY menuChanged);
	/// The children of the given menu.
	Q_PROPERTY(QQmlListProperty<QsMenuEntry> children READ children NOTIFY childrenChanged);
	QML_ELEMENT;

public:
	explicit QsMenuOpener(QObject* parent = nullptr): QObject(parent) {}

	[[nodiscard]] QsMenuEntry* menu() const;
	void setMenu(QsMenuEntry* menu);

	[[nodiscard]] QQmlListProperty<QsMenuEntry> children();

signals:
	void menuChanged();
	void childrenChanged();

private slots:
	void onMenuDestroyed();

private:
	QsMenuEntry* mMenu = nullptr;
};

} // namespace qs::menu
