#include "Application.h"
#include "Frame.h"

wxIMPLEMENT_APP(Application);

Application::Application()
{
	this->frame = nullptr;
	this->rootNode = nullptr;
}

/*virtual*/ Application::~Application()
{
	delete this->rootNode;
}

/*virtual*/ bool Application::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(nullptr, wxDefaultPosition, wxSize(800, 800));
	this->frame->Show();

	return true;
}

/*virtual*/ int Application::OnExit(void)
{
	return 0;
}