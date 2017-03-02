#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <glm\glm.hpp>

bool show_test_window = false;

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}


void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

//Particle arrays
float *partPosition;
float *partVelocity;
float *partTimeAlive;
float *partPrevPosition;
//General variables
float gravity = -9.81;
//Particle state

//Euler variables
float initialVel;

//Verlet variables
bool verlet;


void PhysicsInit() {
	//Arrays initialization
	partPosition = new float[LilSpheres::maxParticles * 3];
	partVelocity = new float[LilSpheres::maxParticles * 3];
	partTimeAlive = new float[LilSpheres::maxParticles];
	partPrevPosition = new float[LilSpheres::maxParticles * 3];

	//Particles initial position
	for (int i = 0; i < LilSpheres::maxParticles; ++i) {
		/*partPosition[i * 3 + 0] = ((float)rand() / RAND_MAX) * 10.f - 5.f;
		partPosition[i * 3 + 1] = ((float)rand() / RAND_MAX) * 10.f;
		partPosition[i * 3 + 2] = ((float)rand() / RAND_MAX) * 10.f - 5.f;*/

		partPosition[i * 3 + 0] = 0;
		partPosition[i * 3 + 1] = 5;
		partPosition[i * 3 + 2] = 0;

		partPrevPosition[i * 3] = partPosition[i * 3] + ((float)rand() / RAND_MAX) * 0.2f - 0.1f;
		partPrevPosition[i * 3 + 1] = partPosition[i * 3 + 1] - ((float)rand() / RAND_MAX) * 0.2f;;
		partPrevPosition[i * 3 + 2] = partPosition[i * 3 + 2] + ((float)rand() / RAND_MAX) * 0.2f - 0.1f;

		verlet = true;
	}

	//TODO
}

void PhysicsUpdate(float dt) {
	for (int i = 0; i < LilSpheres::maxParticles; ++i) {

		if (verlet) {

			//Verlet velocity update X,Y,Z
			partVelocity[i * 3] = ((partPosition[i * 3] + (partPosition[i * 3] - partPrevPosition[i * 3]) + 0 * (dt*dt))- partPosition[i * 3]) / dt;
			partVelocity[i * 3 + 1] = ((partPosition[i * 3 + 1] + (partPosition[i * 3 + 1] - partPrevPosition[i * 3 + 1]) + gravity * (dt*dt)) - partPosition[i * 3 +1]) / dt;
			partVelocity[i * 3 + 2] = ((partPosition[i * 3 + 2] + (partPosition[i * 3 + 2] - partPrevPosition[i * 3 + 2]) + 0 * (dt*dt)) - partPosition[i * 3 + 2]) / dt;

			//Verlet last position save X,Y,Z
			partPrevPosition[i * 3] = partPosition[i * 3];
			partPrevPosition[i * 3 + 1] = partPosition[i * 3 + 1];
			partPrevPosition[i * 3 + 2] = partPosition[i * 3 + 2];

			//Verlet position update X,Y,Z
			partPosition[i * 3] = partPosition[i * 3] + partVelocity[i * 3] * dt;
			partPosition[i * 3 + 1] += (partVelocity[i * 3 + 1] * dt);
			partPosition[i * 3 + 2] = partPosition[i * 3 + 2] + partVelocity[i * 3 + 2] * dt;

		}
		//partVelocity[i * 3] = partVelocity[i * 3] + dt;
		

	}

	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partPosition);
	//TODO

}

void PhysicsCleanup() {
	//TODO
}