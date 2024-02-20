#pragma once

#include <qobject.h>
#include <qqmlintegration.h>
#include <qqmllist.h>
#include <qquickwindow.h>
#include <qscreen.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>
#include <qwindow.h>

#include "proxywindow.hpp"

class Anchors {
	Q_GADGET;
	Q_PROPERTY(bool left MEMBER mLeft);
	Q_PROPERTY(bool right MEMBER mRight);
	Q_PROPERTY(bool top MEMBER mTop);
	Q_PROPERTY(bool bottom MEMBER mBottom);

public:
	bool mLeft = false;
	bool mRight = false;
	bool mTop = false;
	bool mBottom = false;
};

class Margins {
	Q_GADGET;
	Q_PROPERTY(qint32 left MEMBER mLeft);
	Q_PROPERTY(qint32 right MEMBER mRight);
	Q_PROPERTY(qint32 top MEMBER mTop);
	Q_PROPERTY(qint32 bottom MEMBER mBottom);

public:
	qint32 mLeft = 0;
	qint32 mRight = 0;
	qint32 mTop = 0;
	qint32 mBottom = 0;
};

namespace ExclusionMode { // NOLINT
Q_NAMESPACE;
QML_ELEMENT;

enum Enum {
	/// Respect the exclusion zone of other shell layers and optionally set one
	Normal = 0,
	/// Ignore exclusion zones of other shell layers. You cannot set an exclusion zone in this mode.
	Ignore = 1,
	/// Decide the exclusion zone based on the window dimensions and anchors.
	///
	/// Will attempt to reseve exactly enough space for the window and its margins if
	/// exactly 3 anchors are connected.
	Auto = 2,
};
Q_ENUM_NS(Enum);

} // namespace ExclusionMode

///! Decorationless window attached to screen edges by anchors.
/// Decorationless window attached to screen edges by anchors.
///
/// #### Example
/// The following snippet creates a white bar attached to the bottom of the screen.
///
/// ```qml
/// ShellWindow {
///   anchors {
///     left: true
///     bottom: true
///     right: true
///   }
///
///   Text {
///     anchors.horizontalCenter: parent.horizontalCenter
///     anchors.verticalCenter: parent.verticalCenter
///     text: "Hello!"
///   }
/// }
/// ```
class ProxyShellWindow: public ProxyWindowBase {
	// clang-format off
	Q_OBJECT;
	/// Anchors attach a shell window to the sides of the screen.
	/// By default all anchors are disabled to avoid blocking the entire screen due to a misconfiguration.
	///
	/// > [!INFO] When two opposite anchors are attached at the same time, the corrosponding dimension
	/// > (width or height) will be forced to equal the screen width/height.
	/// > Margins can be used to create anchored windows that are also disconnected from the monitor sides.
	Q_PROPERTY(Anchors anchors READ anchors WRITE setAnchors NOTIFY anchorsChanged);
	/// The amount of space reserved for the shell layer relative to its anchors.
	///
	/// > [!INFO] Some systems will require exactly 3 anchors to be attached for the exclusion zone to take
	/// > effect.
	Q_PROPERTY(qint32 exclusionZone READ exclusiveZone WRITE setExclusiveZone NOTIFY exclusionZoneChanged);
	/// Defaults to `ExclusionMode.Normal`.
	Q_PROPERTY(ExclusionMode::Enum exclusionMode READ exclusionMode WRITE setExclusionMode NOTIFY exclusionModeChanged);
	/// Offsets from the sides of the screen.
	///
	/// > [!INFO] Only applies to edges with anchors
	Q_PROPERTY(Margins margins READ margins WRITE setMargins NOTIFY marginsChanged);
	// clang-format on

public:
	explicit ProxyShellWindow(QObject* parent = nullptr): ProxyWindowBase(parent) {}

	QQmlListProperty<QObject> data();

	virtual void setAnchors(Anchors anchors) = 0;
	[[nodiscard]] virtual Anchors anchors() const = 0;

	virtual void setExclusiveZone(qint32 zone) = 0;
	[[nodiscard]] virtual qint32 exclusiveZone() const = 0;

	virtual void setExclusionMode(ExclusionMode::Enum exclusionMode) = 0;
	[[nodiscard]] virtual ExclusionMode::Enum exclusionMode() const = 0;

	virtual void setMargins(Margins margins) = 0;
	[[nodiscard]] virtual Margins margins() const = 0;

signals:
	void anchorsChanged();
	void marginsChanged();
	void exclusionZoneChanged();
	void exclusionModeChanged();

protected:
	ExclusionMode::Enum mExclusionMode = ExclusionMode::Normal;
	qint32 mExclusionZone = 0;
	Anchors mAnchors;
	Margins mMargins;
};