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
double expected_frametime = 1.0 / 30;
float gravity = -9.81;
int partPerSecond = 100*expected_frametime;
//Particle state
float maxTime = 3;
//Euler variables
float initialVel;

//Verlet variables
bool verlet;
//Own variables
int startPart = 0;
int lastPart = partPerSecond;
//XYZ velocity -------- 0 - 1 values
float vMin = 0.03, vMax = 0.06, vY = 0.2;
float random = RAND_MAX;


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

		partPrevPosition[i * 3] = partPosition[i * 3] + ((float)rand() /random) * vMax - vMin;
		partPrevPosition[i * 3 + 1] = partPosition[i * 3 + 1] - ((float)rand() / random) * vY;
		partPrevPosition[i * 3 + 2] = partPosition[i * 3 + 2] + ((float)rand() / random) * vMax - vMin;

		partTimeAlive[i] = 0;

		verlet = true;
	}

	//TODO
}

void PhysicsUpdate(float dt) {
	for (int i = startPart; i <= lastPart; ++i) {

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

		partTimeAlive[i] += dt;

		if (partTimeAlive[i] > maxTime) {
			partPosition[i * 3] = 0;
			partPosition[i * 3 + 1] = 5;
			partPosition[i * 3 + 2] = 0;

			partPrevPosition[i * 3] = partPosition[i * 3] + ((float)rand() / random) * vMax - vMin;
			partPrevPosition[i * 3 + 1] = partPosition[i * 3 + 1] - ((float)rand() / random) * vY;;
			partPrevPosition[i * 3 + 2] = partPosition[i * 3 + 2] + ((float)rand() / random) * vMax - vMin;

			partTimeAlive[i] = 0;
		}
	}

	LilSpheres::updateParticles(startPart, lastPart, partPosition);

	ImGui::Text("First part time: %f", partTimeAlive[0]);
	ImGui::Text("First part time: %d", lastPart);

	lastPart += partPerSecond;

	if (lastPart > LilSpheres::maxParticles) {
		lastPart = LilSpheres::maxParticles - 1;
	}

	

	//TODO

}

void PhysicsCleanup() {
	//TODO
}