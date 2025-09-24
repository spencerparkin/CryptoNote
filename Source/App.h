#pragma once

#include <wx/app.h>

class Frame;

/**
 * 
 */
class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit() override;
	virtual int OnExit() override;

private:
	Frame* frame;
};

wxDECLARE_APP(Application);