#include "iconimageprovider.hpp"

#include <qcolor.h>
#include <qicon.h>
#include <qlogging.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qstring.h>

QPixmap
IconImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) {
	QString iconName;
	QString path;
	auto splitIdx = id.indexOf("?path=");
	if (splitIdx != -1) {
		iconName = id.sliced(0, splitIdx);
		path = id.sliced(splitIdx + 6);
		qWarning() << "Searching custom icon paths is not yet supported. Icon path will be ignored for"
		           << id;
	} else {
		iconName = id;
	}

	auto icon = QIcon::fromTheme(iconName);

	auto targetSize = requestedSize.isValid() ? requestedSize : QSize(100, 100);
	if (targetSize.width() == 0 || targetSize.height() == 0) targetSize = QSize(2, 2);
	auto pixmap = icon.pixmap(targetSize.width(), targetSize.height());

	if (pixmap.isNull()) {
		qWarning() << "Could not load icon" << id << "at size" << targetSize << "from request";
		pixmap = IconImageProvider::missingPixmap(targetSize);
	}

	if (size != nullptr) *size = pixmap.size();
	return pixmap;
}

QPixmap IconImageProvider::missingPixmap(const QSize& size) {
	auto width = size.width() % 2 == 0 ? size.width() : size.width() + 1;
	auto height = size.height() % 2 == 0 ? size.height() : size.height() + 1;
	if (width < 2) width = 2;
	if (height < 2) height = 2;

	auto pixmap = QPixmap(width, height);
	pixmap.fill(QColorConstants::Black);
	auto painter = QPainter(&pixmap);

	auto halfWidth = width / 2;
	auto halfHeight = height / 2;
	auto purple = QColor(0xd900d8);
	painter.fillRect(halfWidth, 0, halfWidth, halfHeight, purple);
	painter.fillRect(0, halfHeight, halfWidth, halfHeight, purple);
	return pixmap;
}

QString IconImageProvider::requestString(const QString& icon, const QString& path) {
	auto req = "image://icon/" + icon;

	if (!path.isEmpty()) {
		req += "?path=" + path;
	}

	return req;
}
