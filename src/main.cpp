#include "pch.h"
#include "engine.h"
#include "particle.h"

int main()
{

	ig::ApplicationConfig cfg{};
	cfg.fullscreen = false;
	cfg.size = WorldRect.size() + Vector2i{32, 32};
	cfg.title = "LiqSim";

	ig::Application app{ cfg };
	Engine::start();

	ig::Window &main_window = app.get_primary_window();
	//ig::Window &window = app.create_window(500, 350, "LiqSim", false);

	main_window.set_draw2d_callback(Engine::draw);


	while (!main_window.should_close())
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));

		main_window.clear();
		main_window.poll();
		Engine::update();
		main_window.render();

	}

}
