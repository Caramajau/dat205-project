
#ifdef _WIN32
extern "C" _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
#endif

#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>

#include <labhelper.h>
#include <imgui.h>

#include <perf.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "fbo.h"

#include <vector>
#include "perlinDisplay.h"
#include "proceduralTerrain.h"
#include "ProceduralConfig.h"
#include "water.h"
#include "waterFrameBuffers.h"

///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
float previousTime = 0.0f;
float deltaTime = 0.0f;
int windowWidth;
int windowHeight;

// Mouse input
bool g_isMouseDragging = false;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint shaderProgram;       // Shader for rendering the final image
GLuint simpleShaderProgram; // Shader used to draw the shadow map
GLuint backgroundProgram;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
float environment_multiplier = 1.5f;
GLuint environmentMap;
const std::string envmap_base_name = "001";

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
vec4 lightStationaryPosition = vec4(200.0f, 80.0f, 200.0f, 1.0f);
vec3 lightPosition = lightStationaryPosition;
vec3 point_light_color = vec3(1.f, 1.f, 1.f);

float point_light_intensity_multiplier = 10000.0f;

bool rotatePointLight = false;



///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(-70.0f, 50.0f, 70.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);
float cameraSpeed = 100.0f;

vec3 worldUp(0.0f, 1.0f, 0.0f);

bool hasEntered;
const float terrainOffset = 5.0f;

const float near = 1.0f;
const float far = 2000.0f;

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
labhelper::Model* fighterModel = nullptr;
labhelper::Model* landingpadModel = nullptr;
labhelper::Model* sphereModel = nullptr;

mat4 roomModelMatrix;
mat4 landingPadModelMatrix;
mat4 fighterModelMatrix;

PerlinDisplay perlinDisplay;
ProceduralTerrain proceduralTerrain;
std::vector<float> heightMapGrid;
ProceduralConfig config{};

Water water;
WaterFrameBuffers waterFBOs;

// Tiny offset to remove potential distortion artefacts near edges.
// NOTE: If the offset is too high, it can cause things that shouldn't be to not reflected to show.
float waterOffset = 1.0f;

bool displayWaterDebug = true;
GLuint activeWaterDebugTexture;

void loadShaders(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../project/simple.vert", "../project/simple.frag", is_reload);
	if(shader != 0)
	{
		simpleShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/background.vert", "../project/background.frag", is_reload);
	if(shader != 0)
	{
		backgroundProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/shading.vert", "../project/shading.frag", is_reload);
	if(shader != 0)
	{
		shaderProgram = shader;
	}

	perlinDisplay.loadShader(is_reload);
	proceduralTerrain.loadShader(is_reload, true);
	water.loadShader(is_reload);
	waterFBOs.loadShader(is_reload);
}

///////////////////////////////////////////////////////////////////////////////
/// This function is called once at the start of the program and never again
///////////////////////////////////////////////////////////////////////////////
void initialize()
{
	ENSURE_INITIALIZE_ONLY_ONCE();

	///////////////////////////////////////////////////////////////////////
	//		Load Shaders
	///////////////////////////////////////////////////////////////////////
	loadShaders(false);

	///////////////////////////////////////////////////////////////////////
	// Load models and set up model matrices
	///////////////////////////////////////////////////////////////////////
	fighterModel = labhelper::loadModelFromOBJ("../scenes/NewShip.obj");
	landingpadModel = labhelper::loadModelFromOBJ("../scenes/landingpad.obj");
	sphereModel = labhelper::loadModelFromOBJ("../scenes/sphere.obj");

	roomModelMatrix = mat4(1.0f);
	fighterModelMatrix = translate(15.0f * worldUp);
	landingPadModelMatrix = mat4(1.0f);

	///////////////////////////////////////////////////////////////////////
	// Load environment map
	///////////////////////////////////////////////////////////////////////
	environmentMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + ".hdr");

	heightMapGrid = createHeightMap(config);
	perlinDisplay.setGpuData(config, heightMapGrid);
	proceduralTerrain.setGpuData(config, heightMapGrid);

	waterFBOs.initialise();
	water.setGpuData(config, waterFBOs);
	activeWaterDebugTexture = waterFBOs.getReflectionTexture();
	waterFBOs.setGpuData(activeWaterDebugTexture);

	glEnable(GL_DEPTH_TEST); // enable Z-buffering
	glEnable(GL_CULL_FACE);  // enables backface culling
}

// NOTE: won't draw the light if it is unused.
void debugDrawLight(const glm::mat4& viewMatrix,
                    const glm::mat4& projectionMatrix,
                    const glm::vec3& worldSpaceLightPos)
{
	if (!config.usePointLight) {
		return;
	}

	mat4 modelMatrix = glm::translate(worldSpaceLightPos);
	glUseProgram(shaderProgram);
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * modelMatrix);
	labhelper::render(sphereModel);
}


void drawBackground(const mat4& viewMatrix, const mat4& projectionMatrix)
{
	///////////////////////////////////////////////////////////////////////////
	// Bind the environment map(s) to unused texture units
	///////////////////////////////////////////////////////////////////////////
	// NOTE: Texture unit 6 is now used by other shaders, so environment map is
	// now set every time the background is drawn rather than just once before.
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(backgroundProgram);
	labhelper::setUniformSlow(backgroundProgram, "environment_multiplier", environment_multiplier);
	labhelper::setUniformSlow(backgroundProgram, "inv_PV", inverse(projectionMatrix * viewMatrix));
	labhelper::setUniformSlow(backgroundProgram, "camera_pos", cameraPosition);
	labhelper::drawFullScreenQuad();
}


///////////////////////////////////////////////////////////////////////////////
/// This function is used to draw the main objects on the scene
///////////////////////////////////////////////////////////////////////////////
void drawScene(GLuint currentShaderProgram,
               const mat4& viewMatrix,
               const mat4& projectionMatrix,
               const mat4& lightViewMatrix,
               const mat4& lightProjectionMatrix,
			   const vec4& waterPlane)
{
	glUseProgram(currentShaderProgram);
	// Light source
	vec4 viewSpaceLightPosition = viewMatrix * vec4(lightPosition, 1.0f);
	labhelper::setUniformSlow(currentShaderProgram, "point_light_color", point_light_color);
	labhelper::setUniformSlow(currentShaderProgram, "point_light_intensity_multiplier",
	                          point_light_intensity_multiplier);
	labhelper::setUniformSlow(currentShaderProgram, "viewSpaceLightPosition", vec3(viewSpaceLightPosition));
	labhelper::setUniformSlow(currentShaderProgram, "viewSpaceLightDir",
	                          normalize(vec3(viewMatrix * vec4(-lightPosition, 0.0f))));


	// Environment
	labhelper::setUniformSlow(currentShaderProgram, "environment_multiplier", environment_multiplier);

	// camera
	labhelper::setUniformSlow(currentShaderProgram, "viewInverse", inverse(viewMatrix));

	// water plane
	glUniform4f(glGetUniformLocation(currentShaderProgram, "waterPlane"),
		waterPlane.x, waterPlane.y, waterPlane.z, waterPlane.w);

	// landing pad
	labhelper::setUniformSlow(currentShaderProgram, "modelMatrix", landingPadModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * landingPadModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix", viewMatrix * landingPadModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
	                          inverse(transpose(viewMatrix * landingPadModelMatrix)));

	labhelper::render(landingpadModel);

	// Fighter
	labhelper::setUniformSlow(currentShaderProgram, "modelMatrix", fighterModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * fighterModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix", viewMatrix * fighterModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
	                          inverse(transpose(viewMatrix * fighterModelMatrix)));

	labhelper::render(fighterModel);

	perlinDisplay.submitToGpu(viewMatrix, projectionMatrix, waterPlane);
	proceduralTerrain.submitToGpu(viewMatrix, projectionMatrix, waterPlane, lightPosition, config);
}

// The camera for the reflection should be 2*d lower, where d is distance to water,
// and also have inverted pitch.
mat4 getReflectionViewMatrix(const vec3& cameraPosition, const vec3& cameraDirection, float waterLevel) {
	float distance = 2 * (cameraPosition.y - waterLevel);
	auto reflectionCameraPosition = vec3(cameraPosition.x, cameraPosition.y - distance, cameraPosition.z);
	auto invertedPitchCameraDirection = vec3(cameraDirection.x, -cameraDirection.y, cameraDirection.z);
	// NOTE: y is inverted in the shader
	return lookAt(reflectionCameraPosition, reflectionCameraPosition + invertedPitchCameraDirection, worldUp);
}

///////////////////////////////////////////////////////////////////////////////
/// This function will be called once per frame, so the code to set up
/// the scene for rendering should go here
///////////////////////////////////////////////////////////////////////////////
void display(void)
{
	labhelper::perf::Scope s( "Display" );

	///////////////////////////////////////////////////////////////////////////
	// Check if window size has changed and resize buffers as needed
	///////////////////////////////////////////////////////////////////////////
	{
		int w;
		int h;
		SDL_GetWindowSize(g_window, &w, &h);
		if(w != windowWidth || h != windowHeight)
		{
			windowWidth = w;
			windowHeight = h;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// setup matrices
	///////////////////////////////////////////////////////////////////////////
	mat4 projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), near, far);
	mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);

	lightPosition = rotatePointLight ? vec3(rotate(currentTime, worldUp) * lightStationaryPosition) : lightStationaryPosition;
	mat4 lightViewMatrix = lookAt(lightPosition, vec3(0.0f), worldUp);
	mat4 lightProjMatrix = perspective(radians(45.0f), 1.0f, 25.0f, 100.0f);

	///////////////////////////////////////////////////////////////////////////
	// Draw from camera
	///////////////////////////////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		labhelper::perf::Scope s( "Background" );
		drawBackground(viewMatrix, projMatrix);
	}
	{
		labhelper::perf::Scope s( "Scene" );
		// Nothing should be culled so a 0 vector is sent, the dot product will be 0.
		drawScene( shaderProgram, viewMatrix, projMatrix, lightViewMatrix, lightProjMatrix, vec4(0) );
	}
	debugDrawLight(viewMatrix, projMatrix, vec3(lightPosition));

	glEnable(GL_CLIP_DISTANCE0);

	// Reflection
	waterFBOs.bindReflectionFrameBuffer();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 reflectionViewMatrix = getReflectionViewMatrix(cameraPosition, cameraDirection, config.waterLevel);

	// Render scene to reflection frame buffer
	{
		labhelper::perf::Scope s("Background");
		drawBackground(reflectionViewMatrix, projMatrix);
	}
	{
		labhelper::perf::Scope s("Scene");
		auto waterPlane = glm::vec4(0, 1, 0, -config.waterLevel + waterOffset);
		drawScene(shaderProgram, reflectionViewMatrix, projMatrix, lightViewMatrix, lightProjMatrix, waterPlane);
	}
	debugDrawLight(reflectionViewMatrix, projMatrix, vec3(lightPosition));

	waterFBOs.unbindCurrentFrameBuffer(windowWidth, windowHeight);

	// Refraction
	waterFBOs.bindRefractionFrameBuffer();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render scene to reflection frame buffer
	{
		labhelper::perf::Scope s("Background");
		drawBackground(viewMatrix, projMatrix);
	}
	{
		labhelper::perf::Scope s("Scene");
		auto waterPlane = glm::vec4(0, -1, 0, config.waterLevel);
		drawScene(shaderProgram, viewMatrix, projMatrix, lightViewMatrix, lightProjMatrix, waterPlane);
	}
	debugDrawLight(viewMatrix, projMatrix, vec3(lightPosition));

	waterFBOs.unbindCurrentFrameBuffer(windowWidth, windowHeight);

	water.submitToGpu(viewMatrix, projMatrix, deltaTime, cameraPosition, near, far, lightPosition, config);
	if (displayWaterDebug) {
		waterFBOs.submitToGpu();
	}
}

// Get terrain height for the camera, does interpolation similar to how it was done for the perlin noise.
// Assumes 1:1 scale between camera position and grid (which is currently true)
float getTerrainHeight(float worldX, float worldZ, const std::vector<float>& grid, int gridWidth) {
	auto x0 = (int)worldX;
	auto z0 = (int)worldZ;
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	float sx = worldX - (int)worldX;
	float sz = worldZ - (int)worldZ;

	// Sample the 4 corners
	float h00 = grid[z0 * gridWidth + x0];
	float h10 = grid[z0 * gridWidth + x1];
	float h01 = grid[z1 * gridWidth + x0];
	float h11 = grid[z1 * gridWidth + x1];

	float topHeight = noSmooth(sx);
	topHeight = lerp(h00, h10, topHeight);

	float bottomHeight = noSmooth(sx);
	bottomHeight = lerp(h01, h11, bottomHeight);

	float finalHeight = noSmooth(sz);
	return lerp(topHeight, bottomHeight, finalHeight);
}

///////////////////////////////////////////////////////////////////////////////
/// This function is used to update the scene according to user input
///////////////////////////////////////////////////////////////////////////////
bool handleEvents(void)
{
	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;

	SDL_SetRelativeMouseMode(hasEntered ? SDL_TRUE : SDL_FALSE);

	while(SDL_PollEvent(&event))
	{
		labhelper::processEvent( &event );

		if(event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
		{
			quitEvent = true;
		}
		if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_g)
		{
			if ( labhelper::isGUIvisible() )
			{
				labhelper::hideGUI();
			}
			else
			{
				labhelper::showGUI();
			}
		}
		if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT
		   && (!labhelper::isGUIvisible() || !ImGui::GetIO().WantCaptureMouse))
		{
			g_isMouseDragging = true;
		}

		if(!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
		{
			// When you have entered the world you always want to drag the camera.
			g_isMouseDragging = hasEntered;
		}

		if(event.type == SDL_MOUSEMOTION && g_isMouseDragging)
		{
			// More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
			float rotationSpeed = 0.1f;
			mat4 yaw = rotate(rotationSpeed * deltaTime * -event.motion.xrel, worldUp);
			mat4 pitch = rotate(rotationSpeed * deltaTime * -event.motion.yrel,
			                    normalize(cross(cameraDirection, worldUp)));
			cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t* state = SDL_GetKeyboardState(nullptr);
	vec3 cameraRight = cross(cameraDirection, worldUp);

	if (state[SDL_SCANCODE_W])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraDirection;
	}
	if (state[SDL_SCANCODE_S])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraDirection;
	}
	if (state[SDL_SCANCODE_A])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraRight;
	}
	if (state[SDL_SCANCODE_D])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraRight;
	}
	if (state[SDL_SCANCODE_Q])
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if (state[SDL_SCANCODE_E])
	{
		cameraPosition += cameraSpeed * deltaTime * worldUp;
	}

	// Account for the terrain being [0, width) and [0, length)
	if (hasEntered
		&& 0 <= cameraPosition.x && cameraPosition.x < config.width - 1
		&& 0 <= cameraPosition.z && cameraPosition.z < config.length - 1) {
		
		cameraPosition.y = getTerrainHeight(
			cameraPosition.x, 
			cameraPosition.z, 
			heightMapGrid, 
			config.width
		) * config.heightScale + config.terrainLevel + terrainOffset;
	}

	if (state[SDL_SCANCODE_Z]) {
		hasEntered = false;
		cameraSpeed = 100.0f;
	}
	return quitEvent;
}

// Helper function for the question mark GUI component
// Directly taken from the ImGui demo:
// https://github.com/pthom/imgui/blob/imgui_bundle/imgui_demo.cpp#L278
// (Or just look in the imgui_demo.cpp)
// (Chose to do this, since the ImGui version this project uses has limited tooltip support)
static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function is to hold the general GUI logic
///////////////////////////////////////////////////////////////////////////////
void gui()
{
	// ----------------- Set variables --------------------------
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
	            ImGui::GetIO().Framerate);
	// ----------------------------------------------------------
	
	if (ImGui::Button("Enter Terrain")) {
		hasEntered = true;
		cameraPosition = vec3(0, heightMapGrid[0] * config.heightScale + config.terrainLevel + terrainOffset, 0);
		cameraSpeed = 10.0f;
	}

	ImGui::Text("The following options require a reload");
	if (ImGui::CollapsingHeader("General Terrain Options")) {
		// Slider int seems to only support half the range (still gives many seed options anyways)
		ImGui::SliderInt("Seed", &config.seed, INT_MIN / 2, INT_MAX / 2);
		ImGui::SameLine();
		HelpMarker("An integer to create different terrains by using a XOR operation with it");

		ImGui::SliderInt("Width", &config.width, 2, 1000);
		ImGui::SameLine();
		HelpMarker("Terrain size in the x-axis");

		ImGui::SliderInt("Length", &config.length, 2, 1000);
		ImGui::SameLine();
		HelpMarker("Terrain size in the z-axis");

		ImGui::SliderInt("Grid Size", &config.gridSize, 1, 1000);
		ImGui::SameLine();
		HelpMarker("Zoom at which to sample the noise, increasing it means you sample over a smaller region thus zooming in and vice versa.");
	}

	if (ImGui::CollapsingHeader("fBM Options")) {
		ImGui::SliderInt("Octaves", &config.octaveCount, 1, 12);
		ImGui::SameLine();
		HelpMarker("Layers of noise, more allows for more amount of detail");

		ImGui::SliderFloat("Lacunarity", &config.lacunarity, 0.0f, 10.0f);
		ImGui::SameLine();
		HelpMarker("How much the frequency increases between octaves");

		ImGui::SliderFloat("Peristence", &config.persistence, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("How much of the amplitude that persists between octaves");
	}

	// Have to convert temporarily to integer, (reinterpret_cast should be fine for enum).
	if (ImGui::CollapsingHeader("Interpolation Options")) {
		ImGui::Text("Smoothing Method");
		ImGui::SameLine();
		HelpMarker("Which method to use for smoothing before interpolating the dot products in the noise");
		ImGui::RadioButton("No Smooth", reinterpret_cast<int*>(&config.smoothType), static_cast<int>(SmoothType::NoSmooth));
		ImGui::RadioButton("Smoothstep", reinterpret_cast<int*>(&config.smoothType), static_cast<int>(SmoothType::SmoothStep));
		ImGui::RadioButton("Smootherstep", reinterpret_cast<int*>(&config.smoothType), static_cast<int>(SmoothType::SmootherStep));

		ImGui::Checkbox("Use Incorrect Interpolation?", &config.useIncorrectLerp);
		ImGui::SameLine();
		HelpMarker("Incorrect lerp logic that leads to wacky generation");
	}

	if (ImGui::CollapsingHeader("Erosion Options")) {
		ImGui::SliderFloat("Erosion Strength", &config.erosionStrength, 0.0f, 10.0f);
		ImGui::SameLine();
		HelpMarker("The erosion strength, higher values cause more detail suppression for slopes and vice versa");

		ImGui::Text("Erosion Method");
		ImGui::SameLine();
		HelpMarker("Which method to use for erosion");
		ImGui::RadioButton("Rational", reinterpret_cast<int*>(&config.erosionType), static_cast<int>(ErosionType::Rational));
		ImGui::SameLine();
		HelpMarker("1 / (1 + k * x), where k is erosion strength");
		ImGui::RadioButton("Exponential", reinterpret_cast<int*>(&config.erosionType), static_cast<int>(ErosionType::Exponential));
		ImGui::SameLine();
		HelpMarker("e^(-k * x), where k is erosion strength");
	}

	if (ImGui::CollapsingHeader("Domain Warping Options")) {
		ImGui::SliderInt("Domain Warping Level", &config.warpLevel, 0, 2);
		ImGui::SameLine();
		HelpMarker("The amount of levels of domain warping, more causes the terrain to look more warped");

		ImGui::SliderFloat("Domain Warping Amplitude", &config.warpAmplitude, 0.0f, 8.0f);
		ImGui::SameLine();
		HelpMarker("The amplitude of the domain warping, how strongly it affects the terrain");
	}
	if (ImGui::Button("Reload Terrain")) {
		heightMapGrid = createHeightMap(config);

		perlinDisplay.setGpuData(config, heightMapGrid);
		proceduralTerrain.setGpuData(config, heightMapGrid);
		water.setGpuData(config, waterFBOs);
	}
	ImGui::NewLine();

	ImGui::Text("The following options are updated immediately");
	if (ImGui::CollapsingHeader("General Terrain Display Options")) {
		ImGui::SliderFloat("Terrain Level", &config.terrainLevel, -200, 0);
		ImGui::SameLine();
		HelpMarker("The level at which the height map offsets from");

		ImGui::SliderFloat("Height Scale", &config.heightScale, 0.1f, 256.0f);
		ImGui::SameLine();
		HelpMarker("How large the heights from the height map should be displayed as");
	}

	if (ImGui::CollapsingHeader("Lighting Options")) {
		ImGui::Checkbox("Use Point Light?", &config.usePointLight);
		ImGui::SameLine();
		HelpMarker("Use point light instead of default directional");

		// NOTE: Ideally you'd grey out and disable the irrelevant controls, but that seemed
		// quite difficult with the ImGUI version this uses. Now it hides them instead
		// as a workaround.
		if (config.usePointLight) {
			ImGui::Checkbox("Rotate Point Light?", &rotatePointLight);
			ImGui::SameLine();
			HelpMarker("Rotate the light around");

			ImGui::Text("Point Light Position");
			ImGui::SameLine();
			HelpMarker("Position for the point light");
			ImGui::SliderFloat("X", &lightStationaryPosition.x, 0.0f, 300.0f);
			ImGui::SliderFloat("Y", &lightStationaryPosition.y, 0.0f, 100.0f);
			ImGui::SliderFloat("Z", &lightStationaryPosition.z, 0.0f, 300.0f);
		}
		else {
			ImGui::Text("Sun Direction");
			ImGui::SameLine();
			HelpMarker("Direction for the directional light");
			ImGui::SliderFloat("X", &config.sunDirection.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Y", &config.sunDirection.y, -1.0f, 1.0f);
			ImGui::SliderFloat("Z", &config.sunDirection.z, -1.0f, 1.0f);
		}
	}

	if (ImGui::CollapsingHeader("Texture Options")) {
		ImGui::Checkbox("Use Neighbours For Normals?", &config.useNeighbours);
		ImGui::SameLine();
		HelpMarker("Computes normals using a four-sample cross filter, rather than the cross product of derivatives");

		ImGui::SliderFloat("Texture Zoom", &config.textureZoom, 0.01f, 8.0f);
		ImGui::SameLine();
		HelpMarker("Zoom level for the texture sampling");

		ImGui::SliderFloat("Triplanar Blending Factor", &config.triplanarBlendFactor, 0.0f, 64.0f);
		ImGui::SameLine();
		HelpMarker("How sharp the blending should be between the triplanar textures");

		ImGui::SliderFloat("Grass Threshold", &config.grassThreshold, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("Threshold for rock/grass blending, slopes below this value are fully grass");

		ImGui::SliderFloat("Rock Threshold", &config.rockThreshold, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("Threshold for rock/grass blending, slopes above this value are fully rock");

		ImGui::SliderFloat("Sand Threshold", &config.sandThreshold, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("At what threshold to start showing sand compared to the base grass/rock");

		ImGui::SliderFloat("Sand Level Offset", &config.sandLevelOffset, 0.0f, 5.0f);
		ImGui::SameLine();
		HelpMarker("How far the sand should go beyond the water level");

		ImGui::SliderFloat("Snow Threshold", &config.snowThreshold, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("At what threshold to start showing snow compared to the base grass/rock");

		ImGui::SliderFloat("Snow Level Start Offset", &config.snowStartLevelOffset, 0.0f, 80.0f);
		ImGui::SameLine();
		HelpMarker("At what level the snow should start from (relative to the water level)");

		ImGui::SliderFloat("Snow Level End Offset", &config.snowEndLevelOffset, 0.0f, 5.0f);
		ImGui::SameLine();
		HelpMarker("The level where the snow takes full effect on the terrain (the influence between is smoothen)");
	}

	if (ImGui::CollapsingHeader("Water Options")) {
		ImGui::SliderFloat("Water Level", &config.waterLevel, -100.0f, 10.0f);
		ImGui::SameLine();
		HelpMarker("The level at which the water exists");

		ImGui::SliderFloat("Water Tiling", &config.waterTiling, 0.01f, 0.16f);
		ImGui::SameLine();
		HelpMarker("How zoomed the water \"tiles\" should be");

		ImGui::SliderFloat("Wave Speed", &config.waterWaveSpeed, -1.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("How fast the water waves should move");

		ImGui::SliderFloat("Wave Strength", &config.waterWaveStrength, -1.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing distortion caused by waves");

		ImGui::SliderFloat("Shine Damper", &config.waterShineDamper, 0.0f, 100.0f);
		ImGui::SameLine();
		HelpMarker("Factor for controlling how sharp or spread out the specular highlights are");

		ImGui::SliderFloat("Reflectivity", &config.waterReflectivity, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing specular highlights caused by waves");

		ImGui::SliderFloat("Border Transparency Factor", &config.waterBorderTransparencyFactor, 0.0f, 100.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing how much depth should affect transparency");

		ImGui::SliderFloat("Distortion Dampening", &config.waterDistortionDampening, 0.0f, 100.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing how much depth should affect distortion dampening");

		ImGui::SliderFloat("Highlight Dampening", &config.waterHighlightDampening, 0.0f, 100.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing how much depth should affect specular highlight dampening");

		ImGui::SliderFloat("Fresnel Modifier", &config.waterFresnelModifier, 0.0f, 20.0f);
		ImGui::SameLine();
		HelpMarker("Factor for adjusting how the Fresnel effect should split, i.e. if it should be more refractive or reflective");

		ImGui::SliderFloat("Normal Flatten Factor", &config.waterNormalFlattenFactor, 0.0f, 10.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing how flat the water is preceived by scaling the normal map y component");

		// NOTE: ColorEdit3, since alpha is overridden in the shader.
		ImGui::SliderFloat("Murky Colour Factor", &config.waterMurkyColourFactor, 0.0f, 100.0f);
		ImGui::SameLine();
		HelpMarker("Factor for increasing or decreasing how much depth should make the water get a murky colour");

		ImGui::ColorEdit3("Murky Colour", glm::value_ptr(config.waterMurkyColour), ImGuiColorEditFlags_Float);
		ImGui::SameLine();
		HelpMarker("The colour for murky water");

		ImGui::SliderFloat("Water Tint Factor", &config.waterTintFactor, 0.0f, 1.0f);
		ImGui::SameLine();
		HelpMarker("Factor to adjust how tinted the water is");

		ImGui::ColorEdit3("Tint Colour", glm::value_ptr(config.waterTintColour), ImGuiColorEditFlags_Float);
		ImGui::SameLine();
		HelpMarker("The colour that the water can be tinted by, is blue by default");

		ImGui::SliderFloat("Water Offset", &waterOffset, 0.0f, 2.0f);
		ImGui::SameLine();
		HelpMarker("Small offset to avoid potential distortion artefacts near edges (usually already avoided with the Border Transparency Factor and Distortion Dampening)");

		ImGui::NewLine();
		ImGui::Checkbox("Enable Water Debug Display?", &displayWaterDebug);
		ImGui::SameLine();
		HelpMarker("Debug display to see what is rendered to the different textures used for the water");

		if (displayWaterDebug) {
			bool shouldDebugDisplayUpdate =
				ImGui::RadioButton("Reflection Texture", reinterpret_cast<int*>(&activeWaterDebugTexture), static_cast<int>(waterFBOs.getReflectionTexture()))
				|| ImGui::RadioButton("Refraction Texture", reinterpret_cast<int*>(&activeWaterDebugTexture), static_cast<int>(waterFBOs.getRefractionTexture()))
				|| ImGui::RadioButton("Refraction Depth Texture", reinterpret_cast<int*>(&activeWaterDebugTexture), static_cast<int>(waterFBOs.getRefractionDepthTexture()));

			if (shouldDebugDisplayUpdate) {
				waterFBOs.setGpuData(activeWaterDebugTexture);
			}
		}
	}
	ImGui::NewLine();
	if (ImGui::Button("Reset Terrain")) {
		config.reset();

		heightMapGrid = createHeightMap(config);

		perlinDisplay.setGpuData(config, heightMapGrid);
		proceduralTerrain.setGpuData(config, heightMapGrid);
		water.setGpuData(config, waterFBOs);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	labhelper::perf::drawEventsWindow();
}

int main(int argc, char* argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Project");

	initialize();

	bool stopRendering = false;
	auto startTime = std::chrono::system_clock::now();

	while(!stopRendering)
	{
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		previousTime = currentTime;
		currentTime = timeSinceStart.count();
		deltaTime = currentTime - previousTime;

		// check events (keyboard among other)
		stopRendering = handleEvents();

		// Inform imgui of new frame
		labhelper::newFrame( g_window );

		// render to window
		display();

		// Render overlay GUI.
		gui();

		// Finish the frame and render the GUI
		labhelper::finishFrame();

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);
	}
	// Free Models
	labhelper::freeModel(fighterModel);
	labhelper::freeModel(landingpadModel);
	labhelper::freeModel(sphereModel);

	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;
}
