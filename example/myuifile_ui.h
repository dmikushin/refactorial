#ifndef MYUIFILE_UI_H
#define MYUIFILE_UI_H

#include <qlabel.h>

class MyUIFile_scr : public QWidget
{
public:
	MyUIFile_scr(QWidget* parent = 0, const char* name = 0, WFlags f = 0);

	virtual QString screenKey(const QString& key = "") { return key; }

	QLabel* lblFoo;
};

#endif
