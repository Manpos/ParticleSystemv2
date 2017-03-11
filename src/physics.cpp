#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <glm\glm.hpp>
#include <iostream>

#define ELASTICITY_COEFICIENT  0.8

using namespace std;
using namespace glm;

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

vec3 preColisionVector;
float preColisionVectorModule;

float counter = 0;

struct Plane {
	glm::vec3 normal;
	float d;
};

Plane planeDown, planeLeft, planeRight, planeFront, planeBack, planeTop;

float tempx, tempy, tempz, prevTempX, prevTempY, prevTempZ;

float elasticity = ELASTICITY_COEFICIENT; // Range from 0 (no elasticity) to 1 (Maximum elsticity)


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
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

// Returns the distance from a position to the plane
float DistancePointPlane(float Xo, float Yo, float Zo, Plane plane) {
	float part1 = plane.normal.x * Xo + plane.normal.y * Yo + plane.normal.z * Zo + plane.d;

	float part2 = sqrt((plane.normal.x * plane.normal.x) + (plane.normal.y * plane.normal.y) + (plane.normal.z * plane.normal.z));
	return (part1 / part2);
}

// Returns the distance from a vector to the plane
float DistancePointPlane(vec3 vector, Plane plane) {
	float part1 = plane.normal.x * vector.x + plane.normal.y * vector.y + plane.normal.z * vector.z + plane.d;

	float part2 = sqrt((plane.normal.x * plane.normal.x) + (plane.normal.y * plane.normal.y) + (plane.normal.z * plane.normal.z));
	return (part1 / part2);
}


// Calculate the module of a vector
float ModuleVector(vec3 vector) {
	return sqrt((vector.x + vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

// Makes a vector unitary
vec3 MakeUnitaryVector(vec3 vector) {
	vec3 unitary;
	unitary.x = vector.x / ModuleVector(vector);
	unitary.y = vector.y / ModuleVector(vector);
	unitary.z = vector.z / ModuleVector(vector);
	return unitary;
}


// Makes the general equation for a plane, by passing three points of the plane
Plane PlaneEquation(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
	vec3 AB, AC, normalVec;
	Plane planeEquation;
	float calculateDx, calculateDy, calculateDz;

	AB.x = (x2 - x1);
	AB.y = (y2 - y1);
	AB.z = (z2 - z1);

	AC.x = (x3 - x1);
	AC.y = (y3 - y1);
	AC.z = (z3 - z1);

	normalVec = cross(AB, AC);

	planeEquation.normal.x = normalVec.x;
	planeEquation.normal.y = normalVec.y;
	planeEquation.normal.z = normalVec.z;

	calculateDx = normalVec.x * x1;
	calculateDy = normalVec.y * y1;
	calculateDz = normalVec.z * z1;

	planeEquation.d = calculateDx + calculateDy + calculateDz;

	return planeEquation;
}

// Calculate a dot product giving a vector and three coords of a point
float DotProduct(vec3 x, float y1, float y2, float y3) {
	return ((x.x * y1) + (x.y * y2) + (x.z * y3));
}

// Calculate if the particle is under the plane
void Bounce(int i, Plane plane) {

	if ((DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2])
		+ DistancePointPlane(partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2], plane))
		* (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPrevPosition[i * 3 + 2])
			+ DistancePointPlane(partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2], plane))
		<= 0) {

		//partPosition[i * 3 + 1] = 0;	
		//ImGui::Text("Anterior: %f	Actual: %f", partPrevPosition[i * 3 + 1], partPosition[i * 3 + 1]);
		/*std::cout << partPrevPosition[i * 3 + 1] << std::endl;
		std::cout << partPosition[i * 3 + 1] << std::endl;*/

		if (selectedSolver == 1) {
			
			/*tempx = partPosition[i * 3] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.x;
			tempy = partPosition[i * 3 + 1] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.x;
			tempz = partPosition[i * 3 + 2] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.x;

			prevTempX = partPrevPosition[i * 3] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.x;
			prevTempY = partPrevPosition[i * 3 + 1] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.x;
			prevTempZ = partPrevPosition[i * 3 + 2] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2])) * (1.0f + elasticity) * plane.normal.z;
			*/
			tempx = partPosition[i * 3] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.x;
			tempy = partPosition[i * 3 + 1] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.y;
			tempz = partPosition[i * 3 + 2] - (DotProduct(plane.normal, partPosition[i * 3], partPosition[i * 3 + 1], partPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.z;
			
			prevTempX = partPrevPosition[i * 3] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.x;
			prevTempY = partPrevPosition[i * 3 + 1] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.y;
			prevTempZ = partPrevPosition[i * 3 + 2] - (DotProduct(plane.normal, partPrevPosition[i * 3], partPrevPosition[i * 3 + 1], partPrevPosition[i * 3 + 2]) + plane.d) * (1.0f + elasticity) * plane.normal.z;
			
			partPosition[i * 3] = tempx;
			partPosition[i * 3 + 1] = tempy;
			partPosition[i * 3 + 2] = tempz;

			partPrevPosition[i * 3] = prevTempX;
			partPrevPosition[i * 3 + 1] = prevTempY;
			partPrevPosition[i * 3 + 2] = prevTempZ;
		}
		
		else {


			prevTempX = initialPartVelocity[i * 3] - (1 + elasticity) * (plane.normal.x * initialPartVelocity[i * 3]) * plane.normal.x;
			prevTempY = initialPartVelocity[i * 3 + 1] - (1 + elasticity) * (plane.normal.y * initialPartVelocity[i * 3 + 1]) * plane.normal.y;
			prevTempZ = initialPartVelocity[i * 3 + 2] - (1 + elasticity) * (plane.normal.z * initialPartVelocity[i * 3 + 2]) * plane.normal.z;

			tempx = partVelocity[i * 3] - (plane.normal.x * partVelocity[i * 3]) * (1 + elasticity) * plane.normal.x;
			tempy = partVelocity[i * 3 + 1] - (plane.normal.y * partVelocity[i * 3 + 1]) * (1 + elasticity) * plane.normal.y;
			tempz = partVelocity[i * 3 + 2] - (plane.normal.z * partVelocity[i * 3 + 2]) * (1 + elasticity) *  plane.normal.z;

			initialPartVelocity[i * 3] = prevTempX;
			initialPartVelocity[i * 3 + 1] = prevTempY;
			initialPartVelocity[i * 3 + 2] = prevTempZ;

			partVelocity[i * 3] = tempx;
			partVelocity[i * 3 + 1] = tempy;
			partVelocity[i * 3 + 2] = tempz;

			partPosition[i * 3] = partPrevPosition[i * 3];
			partPosition[i * 3 + 1] = partPrevPosition[i * 3 + 1];
			partPosition[i * 3 + 2] = partPrevPosition[i * 3 + 2];
		}
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

	// Store plane equation
	planeDown = { glm::vec3(0.f, 1.f, 0.f), 0.f };//PlaneEquation(-5.f, 0.f, -5.f, 5.f, 0.f, -5.f, 5.f, 0.f, 5.f);
	planeLeft = { glm::vec3(1.f, 0.0f, 0.f), 10.f };
	planeRight = { glm::vec3(1.f, 0.0f, 0.f), -10.f };
	planeBack = { glm::vec3(0.f, 0.0f, 1.f), 5.f };
	planeFront = { glm::vec3(0.f, 0.0f, 1.f), 5.f };
	planeTop = { glm::vec3(0.f, 1.0f, 0.f), -20.0f };
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


			Bounce(i, planeDown);
			Bounce(i, planeTop);

			Bounce(i, planeLeft);
			Bounce(i, planeRight);

			Bounce(i, planeBack);
			Bounce(i, planeFront);


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
	ImGui::Text("%f", DistancePointPlane(partPrevPosition[0 * 3], partPrevPosition[0 * 3 + 1], partPrevPosition[0 * 3 + 2], planeRight));
	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partPosition);
}

void PhysicsCleanup() {
	//TODO
}