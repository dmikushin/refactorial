#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <qobject.h>

class QImage;
class QString;
class QPixmap;

class MyPixmap
{
public:
	QImage convertToImage() {
		return QImage();
	}
};

class Example : public QObject
{
	Q_OBJECT

public:
	Example(QObject* parent);
	Example(QObject* parent, const char* name);
	explicit Example(int foo);
	~Example();

	QImage toImage(const QPixmap& pix);
	QImage toImage(const MyPixmap& pix);

	QString joinTogether(const QValueList<QVariant>& in);

private:
	QString _result;
};

#endif
