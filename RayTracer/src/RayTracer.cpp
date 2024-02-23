#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings"); // SETTINGS WINDOW
		ImGui::Text("Render time: %.3fms", renderTime);
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // Push style before begin, pop after end
		ImGui::Begin("Viewport"); // VIEWPORT WINDOW

		viewportWidth = ImGui::GetContentRegionAvail().x;
		viewportHeight = ImGui::GetContentRegionAvail().y;

		auto finalImage = renderer.GetFinalImage();
		if (finalImage)
			ImGui::Image(finalImage->GetDescriptorSet(), 
				{ (float)finalImage->GetWidth(), (float)finalImage->GetHeight() }, // Renders the image with its own width/height, not the viewports
				ImVec2(0,1), ImVec2(1,0)); // Reverses the y vector so 0 is at the bottom

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() { // Render image in viewport
		Timer timer;

		renderer.OnResize(viewportWidth, viewportHeight);
		renderer.Render();

		renderTime = timer.ElapsedMillis(); // Save the time it took to finish rendering
	}

private:
	Renderer renderer;
	uint32_t viewportWidth = 0, viewportHeight = 0;
	float renderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}