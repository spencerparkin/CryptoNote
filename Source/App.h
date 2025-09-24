#pragma once

#include <wx/app.h>
#include <memory>

class Frame;
class EncryptionScheme;

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

	EncryptionScheme* GetEncryptionScheme();
	Frame* GetFrame();

private:
	Frame* frame;
	std::unique_ptr<EncryptionScheme> encryptionScheme;
};

wxDECLARE_APP(Application);