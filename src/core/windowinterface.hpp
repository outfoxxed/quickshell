#pragma once

#include <qcolor.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qqmllist.h>
#include <qquickitem.h>
#include <qtmetamacros.h>
#include <qtypes.h>

#include "qmlscreen.hpp"
#include "region.hpp"
#include "reload.hpp"

class ProxyWindowBase;
class QsWindowAttached;

///! Base class of Quickshell windows
/// Base class of Quickshell windows
/// ### Attached properties
/// `QSWindow` can be used as an attached object of anything that subclasses @@QtQuick.Item$.
/// It provides the following properties
/// - `window` - the `QSWindow` object.
/// - `contentItem` - the `contentItem` property of the window.
class WindowInterface: public Reloadable {
	Q_OBJECT;
	// clang-format off
	Q_PROPERTY(QQuickItem* contentItem READ contentItem CONSTANT);
	/// If the window should be shown or hidden. Defaults to true.
	Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged);
	/// If the window is currently shown. You should generally prefer [visible](#prop.visible).
	///
	/// This property is useful for ensuring windows spawn in a specific order, and you should
	/// not use it in place of [visible](#prop.visible).
	Q_PROPERTY(bool backingWindowVisible READ isBackingWindowVisible NOTIFY backingWindowVisibleChanged);
	Q_PROPERTY(qint32 width READ width WRITE setWidth NOTIFY widthChanged);
	Q_PROPERTY(qint32 height READ height WRITE setHeight NOTIFY heightChanged);
	/// The screen that the window currently occupies.
	///
	/// This may be modified to move the window to the given screen.
	Q_PROPERTY(QuickshellScreenInfo* screen READ screen WRITE setScreen NOTIFY screenChanged);
	/// Opaque property that will receive an update when factors that affect the window's position
	/// and transform changed.
	///
	/// This property is intended to be used to force a binding update,
	/// along with map[To|From]Item (which is not reactive).
	Q_PROPERTY(QObject* windowTransform READ windowTransform NOTIFY windowTransformChanged);
	/// The background color of the window. Defaults to white.
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged);
	/// The clickthrough mask. Defaults to null.
	///
	/// If non null then the clickable areas of the window will be determined by the provided region.
	///
	/// ```qml
	/// ShellWindow {
	///   // The mask region is set to `rect`, meaning only `rect` is clickable.
	///   // All other clicks pass through the window to ones behind it.
	///   mask: Region { item: rect }
	///
	///   Rectangle {
	///     id: rect
	///
	///     anchors.centerIn: parent
	///     width: 100
	///     height: 100
	///   }
	/// }
	/// ```
	///
	/// If the provided region's intersection mode is `Combine` (the default),
	/// then the region will be used as is. Otherwise it will be applied on top of the window region.
	///
	/// For example, setting the intersection mode to `Xor` will invert the mask and make everything in
	/// the mask region not clickable and pass through clicks inside it through the window.
	///
	/// ```qml
	/// ShellWindow {
	///   // The mask region is set to `rect`, but the intersection mode is set to `Xor`.
	///   // This inverts the mask causing all clicks inside `rect` to be passed to the window
	///   // behind this one.
	///   mask: Region { item: rect; intersection: Intersection.Xor }
	///
	///   Rectangle {
	///     id: rect
	///
	///     anchors.centerIn: parent
	///     width: 100
	///     height: 100
	///   }
	/// }
	/// ```
	Q_PROPERTY(PendingRegion* mask READ mask WRITE setMask NOTIFY maskChanged);
	Q_PROPERTY(QQmlListProperty<QObject> data READ data);
	// clang-format on
	Q_CLASSINFO("DefaultProperty", "data");
	QML_NAMED_ELEMENT(QsWindow);
	QML_UNCREATABLE("uncreatable base class");
	QML_ATTACHED(QsWindowAttached);

public:
	explicit WindowInterface(QObject* parent = nullptr): Reloadable(parent) {}

	[[nodiscard]] virtual ProxyWindowBase* proxyWindow() const = 0;
	[[nodiscard]] virtual QQuickItem* contentItem() const = 0;

	[[nodiscard]] virtual bool isVisible() const = 0;
	[[nodiscard]] virtual bool isBackingWindowVisible() const = 0;
	virtual void setVisible(bool visible) = 0;

	[[nodiscard]] virtual qint32 width() const = 0;
	virtual void setWidth(qint32 width) = 0;

	[[nodiscard]] virtual qint32 height() const = 0;
	virtual void setHeight(qint32 height) = 0;

	[[nodiscard]] virtual QuickshellScreenInfo* screen() const = 0;
	virtual void setScreen(QuickshellScreenInfo* screen) = 0;

	[[nodiscard]] QObject* windowTransform() const { return nullptr; } // NOLINT

	[[nodiscard]] virtual QColor color() const = 0;
	virtual void setColor(QColor color) = 0;

	[[nodiscard]] virtual PendingRegion* mask() const = 0;
	virtual void setMask(PendingRegion* mask) = 0;

	[[nodiscard]] virtual QQmlListProperty<QObject> data() = 0;

	static QsWindowAttached* qmlAttachedProperties(QObject* object);

signals:
	void windowConnected();
	void visibleChanged();
	void backingWindowVisibleChanged();
	void widthChanged();
	void heightChanged();
	void screenChanged();
	void windowTransformChanged();
	void colorChanged();
	void maskChanged();
};

class QsWindowAttached: public QObject {
	Q_OBJECT;
	Q_PROPERTY(QObject* window READ window CONSTANT);
	Q_PROPERTY(QQuickItem* contentItem READ contentItem CONSTANT);
	QML_ANONYMOUS;

public:
	[[nodiscard]] virtual QObject* window() const = 0;
	[[nodiscard]] virtual QQuickItem* contentItem() const = 0;

protected:
	explicit QsWindowAttached(QObject* parent): QObject(parent) {}
};
