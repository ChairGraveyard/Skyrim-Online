#include <EnetServer.h>
#include <Engine\Game.h>
#include <time.h>

namespace Logic
{ 
	namespace Engine
	{
		Interfaces::IController* TheController = nullptr;

		void InitializeGame()
		{
			if (TheController)
			{
				return;
			}

			EnetServer::Initialize();

			IDirect3D9* pDevice;
			g_pIDirect3DDevice9->GetDirect3D(&pDevice);

			Logic::Engine::Input::TheInputManager = new Logic::Engine::Input::InputManager;
			TheIInputHook->SetListener(Logic::Engine::Input::TheInputManager); // Set listener.

			Logic::Overlay::TheGUI = new Logic::Overlay::GUI(); // Initialize the GUI.

			Signal<void(IDirect3DDevice9*)>* onPresent = &g_pIDirect3DDevice9->OnPresent; 
			Signal<void(IDirect3DDevice9*)>* onReset = &g_pIDirect3DDevice9->OnReset;
			
			onPresent->Add(std::bind(&Logic::Overlay::GUI::OnRender, Logic::Overlay::TheGUI, std::placeholders::_1));
			onReset->Add(std::bind(&Logic::Overlay::GUI::OnLostDevice, Logic::Overlay::TheGUI, std::placeholders::_1));

			Logic::Overlay::TheChat = new Logic::Overlay::Chat(Logic::Overlay::TheGUI->getGUI()); // Initialize the chat.

			TheController = new Logic::Engine::Controllers::SkyrimController();

			boost::thread ConnectionThread(Logic::Engine::ConnectionUpdate);
		}

		void Update()
		{
			TheController->Update();
		}

		void ConnectionUpdate()
		{
			while (true)
			{
				TheController->GetWorld()->Update();

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}
	}
}


