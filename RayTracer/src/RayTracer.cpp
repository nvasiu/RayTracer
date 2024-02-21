#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

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

		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }); // Renders the image with its own width/height, not the viewports

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Render() { // Render image in viewport
		Timer timer;

		if (!image || viewportWidth != image->GetWidth() || viewportHeight != image->GetHeight() ) {
			// RGBA is 4 bytes or 32 bits (1 byte per channel) so imageData is stored as a 32 bit ints
			// RGBA format is 0xAABBGGRR
			image = std::make_shared<Image>(viewportWidth, viewportHeight, ImageFormat::RGBA);
			
			delete[] imageData;
			imageData = new uint32_t[viewportWidth * viewportHeight];
		}

		for (uint32_t i = 0; i < viewportWidth * viewportHeight; i++) {
			imageData[i] = Random::UInt();
			imageData[i] |= 0xff000000; // Set alpha to 1
		}

		image->SetData(imageData);

		renderTime = timer.ElapsedMillis(); // Save the time it took to finish rendering
	}

private:
	std::shared_ptr<Image> image;
	uint32_t viewportWidth = 0, viewportHeight = 0;
	uint32_t* imageData = nullptr;
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