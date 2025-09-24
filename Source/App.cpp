#include "App.h"
#include "Frame.h"
#include "EncryptionScheme.h"

wxIMPLEMENT_APP(Application);

Application::Application()
{
	this->frame = nullptr;
}

/*virtual*/ Application::~Application()
{
}

/*virtual*/ bool Application::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	this->encryptionScheme.reset(new AESEncryptionScheme());

	this->frame = new Frame();
	this->frame->Show();

	return true;
}

/*virtual*/ int Application::OnExit()
{
	return 0;
}

EncryptionScheme* Application::GetEncryptionScheme()
{
	return this->encryptionScheme.get();
}

Frame* Application::GetFrame()
{
	return this->frame;
}