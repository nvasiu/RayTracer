#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
	{
		{
		Sphere sphere;
		sphere.Position = { 0.0f, 0.1f, -1.8f };
		sphere.Radius = 1.0f;
		sphere.SphereMaterial.Albedo = { 1.0f, 1.0f, 0.0f };
		sphere.SphereMaterial.Roughness = 0.6f;
		scene.Spheres.push_back(sphere);
		}
		{
		Sphere sphere;
		sphere.Position = { 0.0f, -101.0f, 0.0f };
		sphere.Radius = 100.0f;
		sphere.SphereMaterial.Albedo = { 0.0f, 0.3f, 1.0f };
		sphere.SphereMaterial.Roughness = 0.2f;
		scene.Spheres.push_back(sphere);
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings"); // SETTINGS WINDOW
		ImGui::Text("Render time: %.3fms", renderTime);
		if (ImGui::Button("Render")) {
			Render();
		}

		ImGui::Checkbox("Accumulate", &renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset")) {
			renderer.ResetFrameIndex();
		}
		ImGui::End();

		ImGui::Begin("Scene"); // SCENE WINDOW
		for (size_t i = 0; i < scene.Spheres.size(); i++) {

			// Creating multiple menus with the same name will treat them all as the same menu
			// Pushing a unique ID before creating the menus will separate them
			ImGui::PushID(i);

			Sphere& sphere = scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::ColorEdit3("Color", glm::value_ptr(sphere.SphereMaterial.Albedo), 0.1f);
			ImGui::DragFloat("Roughness", &sphere.SphereMaterial.Roughness, 0.1f);
			ImGui::DragFloat("Metallic", &sphere.SphereMaterial.Metallic, 0.1f);
			ImGui::Separator();

			ImGui::PopID(); // Pop ID after
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
		renderer.Render(scene);

		renderTime = timer.ElapsedMillis(); // Save the time it took to finish rendering
	}

private:
	Renderer renderer;
	uint32_t viewportWidth = 0, viewportHeight = 0;
	float renderTime = 0.0f;
	Scene scene;
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