#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <qobject.h>

class QWidget;
class QImage;
class QString;
class QPixmap;

class Example : public QObject
{
	Q_OBJECT

public:
	Example(QWidget* parent);
	~Example();

	QImage toImage(const QPixmap& pix);

	QString joinTogether(const QValueList<QVariant>& in);

private:
	QString _result;
};

#endif
