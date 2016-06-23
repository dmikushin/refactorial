#include "ui.h"

//==============================================================================
UI::UI(QWidget* parent, const char* name, WFlags f)
	: MyUIFile_scr(parent, name, f)
{
	connect(lblFoo, SIGNAL(clicked()), this, SLOT(handleClicked()));
}

//==============================================================================
void UI::handleClicked()
{
	// do stuff
}
