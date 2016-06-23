#ifndef UI_H
#define UI_H

#include "myuifile_ui.h"

class UI : public MyUIFile_scr
{
	Q_OBJECT

public:
	UI(QWidget* parent = 0, const char* name = 0, WFlags f = 0);
	virtual ~UI();

	virtual QString screenKey(const QString& key = "") { return MyUIFile_scr::screenKey(key); }

private slots:
	void handleClicked();
};

#endif
