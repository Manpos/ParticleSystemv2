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

//Emitter's parameters
int selectedSolver;
int selectedEmitter;
float emitterPosition[3];
float cascadeWidth = 5;

//Own variables
int tail = 0;
int head = 0;
float maxEmitedParticles;
float currTimeAlive;

//XYZ initial velocity
float initVel[3];



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
		ImGui::RadioButton("Font", &selectedEmitter, 0); ImGui::SameLine();
		ImGui::RadioButton("Cascade", &selectedEmitter, 1);
		ImGui::SliderFloat("Cascade Width", &cascadeWidth, 1, 10);
		ImGui::InputFloat3("Emitter position", emitterPosition);
		ImGui::InputFloat3("Initial velocity", initVel);
		ImGui::SliderInt("Parts per second", &maxPartPerSecond, 100, 3000);
		ImGui::SliderFloat("Life time", &maxTimeAlive, 1, 10);
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

	emitterPosition[0] = 0;
	emitterPosition[1] = 5;
	emitterPosition[2] = 0;

	initVel[0] = 0.03;
	initVel[1] = 0.2;
	initVel[2] = 0.03;

	selectedSolver = 0;
	currTimeAlive = maxTimeAlive;
}

void PhysicsUpdate(float dt) {	

	partPerSecond = maxPartPerSecond *dt;

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

			if (partTimeAlive[i] <= 0) {
				head++;
				head = head % (LilSpheres::maxParticles);
				//Reset initial position
				partPosition[i * 3] = emitterPosition[0];
				partPosition[i * 3 + 1] = emitterPosition[1];
				partPosition[i * 3 + 2] = emitterPosition[2];

			}
		}
	}

	//Emisor font
	float emitedParticles = 0;
	maxEmitedParticles = partPerSecond;

	if (selectedEmitter == 0) {

		while (emitedParticles < maxEmitedParticles) {

			emitedParticles++;

			//Reset initial position
			partPosition[tail * 3] = emitterPosition[0];
			partPosition[tail * 3 + 1] = emitterPosition[1];
			partPosition[tail * 3 + 2] = emitterPosition[2];

			//Reset previous position
			partPrevPosition[tail * 3] = partPosition[tail * 3] + ((float)rand() / random) * initVel[0]*2 - initVel[0];
			partPrevPosition[tail * 3 + 1] = partPosition[tail * 3 + 1] - ((float)rand() / random) * initVel[1]*2 - initVel[1];
			partPrevPosition[tail * 3 + 2] = partPosition[tail * 3 + 2] + ((float)rand() / random) * initVel[2]*2 - initVel[2];

			//Reset initial velocity
			initialPartVelocity[tail * 3] = (partPosition[tail * 3] - partPrevPosition[tail * 3])/ dt;
			initialPartVelocity[tail * 3 + 1] = (partPosition[tail * 3 + 1] - partPrevPosition[tail * 3 + 1]) / dt;
			initialPartVelocity[tail * 3 + 2] = (partPosition[tail * 3 + 2] - partPrevPosition[tail * 3 + 2]) / dt;

			tail++;
			tail = tail % (LilSpheres::maxParticles);

			//Reset timer
			partTimeAlive[tail] = maxTimeAlive;

		}
	}
	
	else {

		while (emitedParticles < maxEmitedParticles) {

			emitedParticles++;

			//Reset initial position
			partPosition[tail * 3] = emitterPosition[0] + ((float)rand() / random) * cascadeWidth - cascadeWidth/2;
			partPosition[tail * 3 + 1] = emitterPosition[1];
			partPosition[tail * 3 + 2] = emitterPosition[2];

			//Reset previous position
			partPrevPosition[tail * 3] = partPosition[tail * 3] + ((float)rand() / random) * 0;
			partPrevPosition[tail * 3 + 1] = partPosition[tail * 3 + 1] - ((float)rand() / random) * 0;
			partPrevPosition[tail * 3 + 2] = partPosition[tail * 3 + 2] + ((float)rand() / random) * initVel[2] - initVel[2];

			//Reset initial velocity
			initialPartVelocity[tail * 3] = (partPosition[tail * 3] - partPrevPosition[tail * 3]) / dt;
			initialPartVelocity[tail * 3 + 1] = (partPosition[tail * 3 + 1] - partPrevPosition[tail * 3 + 1]) / dt;
			initialPartVelocity[tail * 3 + 2] = (partPosition[tail * 3 + 2] - partPrevPosition[tail * 3 + 2]) / dt;

			tail++;
			tail = tail % (LilSpheres::maxParticles);

			//Reset timer
			partTimeAlive[tail] = maxTimeAlive;

		}
	}
	
	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partPosition);
}

void PhysicsCleanup() {
	//TODO
}