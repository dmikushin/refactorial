#include <qstring.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qheader.h>
#include <qlistview.h>
#include <qdir.h>
#include <qfileinfo.h>

#include "example.h"

//==============================================================================
Example::Example(QObject* parent, WFlags f)
	: QObject(parent),
	  _result(QString::null)
{
	QListView* lsv = new QListView();
	lsv->setResizeMode(QListView::LastColumn);
	lsv->header()->setResizeEnabled(false, 0);
}

//==============================================================================
Example::Example(QObject* parent, const char* name, WFlags f)
	: QObject(parent, name),
	  _result(QString::null)
{}

//==============================================================================
Example::Example(int foo)
	: QObject(),
	  _result(QString::number(foo))
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

//==============================================================================
QVariant Example::testBool(bool b)
{
	return QVariant(b, 0);
}

//==============================================================================
QDir Example::getPath(QFileInfo f)
{
	Test t;
	t.doArg(f.exists());
	t.doArg(f.exists(), 17, QString("hello world"));
	return f.dir(true);
}

//==============================================================================
void Test::doArg(bool)
{}

//==============================================================================
void Test::doArg(bool, int)
{}

//==============================================================================
void Test::doArg(bool, int, const QString&)
{}
