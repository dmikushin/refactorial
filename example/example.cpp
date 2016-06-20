#include <qstring.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qimage.h>
#include <qpixmap.h>

#include "example.h"

//==============================================================================
Example::Example(QWidget* parent)
	: QObject(parent),
	  _result(QString::null)
{}

//==============================================================================
Example::~Example() {}

//==============================================================================
QImage Example::toImage(const QPixmap& pix)
{
	return pix.convertToImage();
}

//==============================================================================
QImage Example::toImage(const MyPixmap& pix)
{
	return pix.convertToImage();
}

//==============================================================================
QString Example::joinTogether(const QValueList<QVariant>& in)
{
	_result = "";

	for (QValueList<QVariant>::const_iterator itr = in.constBegin(); itr != in.constEnd(); ++itr)
	{
		QVariant v = (*itr);
		_result += QString(v.toString());
	}

	return _result;
}
