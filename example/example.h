#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <qobject.h>

class QImage;
class QString;
class QPixmap;
class QFileInfo;
class QDir;

class MyPixmap
{
public:
	MyPixmap() {};
	MyPixmap(const QPixmap& /*pix*/) {};
	MyPixmap(QPixmap* /*pix*/ = 0) {};

	QImage convertToImage() const {
		return QImage();
	}
};

class Test
{
public:
	void doArg(bool b);
	void doArg(bool b, int i);
	void doArg(bool b, int i, const QString& s);
};

class Example : public QObject
{
	Q_OBJECT

public:
	Example(QObject* parent, WFlags f = 0);
	Example(QObject* parent, const char* name, WFlags f = 0);
	explicit Example(int foo);
	~Example();

	QImage toImage(const QPixmap& pix);
	QImage toImage(const MyPixmap& pix);

	QString joinTogether(const QValueList<QVariant>& in);

	QVariant testBool(bool b);
	QDir getPath(QFileInfo f);

private:
	QString _result;
};

#endif
