#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <glm\glm.hpp>
#include <iostream>

using namespace std;

bool show_test_window = false;

//Particle arrays
float *partPosition;
float *partVelocity;
float *initialPartVelocity;
float *partTimeAlive;
float *partPrevPosition;

//General variables
double expected_frametime = 1.0 / 30;
float gravity = -9.81;

//Particle state
float maxTimeAlive = 1;
int maxPartPerSecond = 1000;
int partPerSecond = maxPartPerSecond *expected_frametime;
int deadParticles = 0;

//Euler variables
float initialVel;

//Calc movement bools
int selectedSolver;

//Own variables
int tail = 0;
int head = 0;
float maxEmitedParticles;
float currTimeAlive;

//XYZ velocity -------- 0 - 1 values | Verlet
float vMin = 0.03, vMax = 0.06, vY = 0.2;

//XYZ velocity -------- 0 - 10 velues | Euler
float vMinE = 1.5, vMaxE = 3, vYE = 8;

//Max random Value
float random = RAND_MAX;

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
		ImGui::RadioButton("Euler", &selectedSolver, 0); ImGui::SameLine();
		ImGui::RadioButton("Verlet", &selectedSolver, 1);
		ImGui::SliderInt("Particles per second", &maxPartPerSecond, 100, 1000);
		ImGui::SliderFloat("Seconds alive", &maxTimeAlive, 1, 10);
		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}


void PhysicsInit() {
	//Arrays initialization
	partPosition = new float[LilSpheres::maxParticles * 3];
	partVelocity = new float[LilSpheres::maxParticles * 3];
	initialPartVelocity = new float[LilSpheres::maxParticles * 3];
	partTimeAlive = new float[LilSpheres::maxParticles];
	partPrevPosition = new float[LilSpheres::maxParticles * 3];

	selectedSolver = 0;
	currTimeAlive = maxTimeAlive;
}

void PhysicsUpdate(float dt) {	

	partPerSecond = maxPartPerSecond *dt;
	//ImGui::Text("%d", partPerSecond);

	for (int i = 0; i < LilSpheres::maxParticles; ++i) {

		if (partTimeAlive[i] > 0) {

			if (selectedSolver == 1) {

				float temp[3];

				//Verlet prev position save
				temp[0] = partPosition[i * 3];
				temp[1] = partPosition[i * 3 + 1];
				temp[2] = partPosition[i * 3 + 2];

				//Verlet position update X,Y,Z
				partPosition[i * 3] = partPosition[i * 3] + (partPosition[i * 3] - partPrevPosition[i * 3]) + 0 * (dt*dt);
				partPosition[i * 3 + 1] = partPosition[i * 3 + 1] + (partPosition[i * 3 + 1] - partPrevPosition[i * 3 + 1]) + gravity * (dt*dt);
				partPosition[i * 3 + 2] = partPosition[i * 3 + 2] + (partPosition[i * 3 + 2] - partPrevPosition[i * 3 + 2]) + 0 * (dt*dt);

				//Verlet last position save X,Y,Z
				partPrevPosition[i * 3] = temp[0];
				partPrevPosition[i * 3 + 1] = temp[1];
				partPrevPosition[i * 3 + 2] = temp[2];

			}

			else {

				//Euler velocity update X,Y,Z
				partVelocity[i * 3] = initialPartVelocity[i * 3] + 0 * dt;
				partVelocity[i * 3 + 1] = initialPartVelocity[i * 3 + 1] + gravity * dt;
				partVelocity[i * 3 + 2] = initialPartVelocity[i * 3 + 2] + 0 * dt;

				//Verlet last position save X,Y,Z
				partPrevPosition[i * 3] = partPosition[i * 3];
				partPrevPosition[i * 3 + 1] = partPosition[i * 3 + 1];
				partPrevPosition[i * 3 + 2] = partPosition[i * 3 + 2];

				//Euler position update X,Y,Z
				partPosition[i * 3] = partPosition[i * 3] + partVelocity[i * 3] * dt;
				partPosition[i * 3 + 1] = partPosition[i * 3 + 1] + partVelocity[i * 3 + 1] * dt;
				partPosition[i * 3 + 2] = partPosition[i * 3 + 2] + partVelocity[i * 3 + 2] * dt;

				//Euler initial position update X,Y,Z
				initialPartVelocity[i * 3] = partVelocity[i * 3];
				initialPartVelocity[i * 3 + 1] = partVelocity[i * 3 + 1];
				initialPartVelocity[i * 3 + 2] = partVelocity[i * 3 + 2];

			}

			partTimeAlive[i] -= dt;
			//cout << partTimeAlive[i] << endl;

			if (partTimeAlive[i] <= 0) {
				head++;
				head = head % (LilSpheres::maxParticles);
				//Reset initial position
				partPosition[i * 3] = 0;
				partPosition[i * 3 + 1] = 5;
				partPosition[i * 3 + 2] = 0;

			}
		}
	}

	//Emisor font
	float emitedParticles = 0;
	maxEmitedParticles = partPerSecond;

	while (emitedParticles < maxEmitedParticles) {

		emitedParticles++;

		//Reset initial position
		partPosition[tail * 3] = 0;
		partPosition[tail * 3 + 1] = 5;
		partPosition[tail * 3 + 2] = 0;

		//Reset initial velocity
		initialPartVelocity[tail * 3] = ((float)rand() / random) * vMaxE - vMinE;
		initialPartVelocity[tail * 3 + 1] = ((float)rand() / random) * vYE;
		initialPartVelocity[tail * 3 + 2] = ((float)rand() / random) * vMaxE - vMinE;

		//Reset previous position
		partPrevPosition[tail * 3] = partPosition[tail * 3] + ((float)rand() / random) * vMax - vMin;
		partPrevPosition[tail * 3 + 1] = partPosition[tail * 3 + 1] - ((float)rand() / random) * vY;;
		partPrevPosition[tail * 3 + 2] = partPosition[tail * 3 + 2] + ((float)rand() / random) * vMax - vMin;

		tail++;
		tail = tail % (LilSpheres::maxParticles);

		//Reset timer
		partTimeAlive[tail] = maxTimeAlive;

	}
	
	//ImGui::Text("%d", head);
	//ImGui::Text("%d", tail);



	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partPosition);
}

void PhysicsCleanup() {
	//TODO
}