/**
 * @file main.cpp
 * @brief Simulador de vuelo con HUD (Heads-Up Display) profesional y modelo F-16
 *
 * Sistema completo de renderizado 3D con:
 * - Terreno con texturizado triplanar y niebla
 * - Skybox para cielo envolvente
 * - Modelo F-16 cargado con Assimp
 * - Cámara en tercera persona con seguimiento
 * - HUD con altímetro de 7 segmentos
 * - Física básica de vuelo
 */

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "gfx/skybox/SkyboxRenderer.h"
#include "gfx/skybox/TextureCube.h"
#include "gfx/terrain/TerrainRenderer.h"
#include "gfx/core/Shader.h"
#include "gfx/geometry/Model.h"
#include "gfx/WaypointRenderer.h"
#include "hud/core/FlightHUD.h"
#include "flight/FlightData.h"

// ============================================================================
// CONSTANTES DE CONFIGURACIÓN
// ============================================================================

static const char *kWindowTitle = "Flight Simulator HUD - F16 Model";
static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

// ============================================================================
// ESTADO DEL AVIÓN
// ============================================================================
glm::vec3 planePos(0.0f, 50.0f, 0.0f);
glm::quat planeOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
float planeSpeed = 50.0f; // m/s

// ============================================================================
// ESTADO DE LA CÁMARA
// ============================================================================
glm::vec3 cameraPos;
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float cameraDistance = 15.0f;
float cameraPitch = 20.0f;
float cameraYaw = 0.0f;
bool firstPersonView = false; // false = 3ra persona, true = 1ra persona (POV)

// ============================================================================
// TIMING
// ============================================================================

float deltaTime = 0.0f; // Tiempo entre frames
float lastFrame = 0.0f; // Timestamp del frame anterior

// ============================================================================
// DATOS DE SIMULACIÓN
// ============================================================================

flight::FlightData flightData;		 // Datos del avión (velocidad, altitud, etc.)
hud::FlightHUD *globalHUD = nullptr; // Puntero global al HUD (para callbacks)

// ============================================================================
// SISTEMA DE WAYPOINTS
// ============================================================================

struct Waypoint {
	glm::vec3 position;
	std::string name;
	bool captured;  // Indica si el waypoint ya fue alcanzado
};

std::vector<Waypoint> waypoints;
int activeWaypointIndex = 0;
bool waypointSystemEnabled = false;
bool missionCompleted = false;
gfx::WaypointRenderer waypointRenderer;

// ============================================================================
// DECLARACIÓN DE FUNCIONES
// ============================================================================

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void print_gl_version(void);
void initializeWaypoints();
void updateWaypointData();

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main()
{
	// ------------------------------------------------------------------------
	// 1. INICIALIZACIÓN DE GLFW Y VENTANA
	// ------------------------------------------------------------------------

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(
		kWindowWidth,
		kWindowHeight,
		kWindowTitle,
		nullptr,
		nullptr);

	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// ------------------------------------------------------------------------
	// 2. INICIALIZACIÓN DE GLAD (OpenGL Function Loader)
	// ------------------------------------------------------------------------

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	print_gl_version();

	// ------------------------------------------------------------------------
	// 3. CONFIGURACIÓN DE OPENGL
	// ------------------------------------------------------------------------

	glViewport(0, 0, kWindowWidth, kWindowHeight);
	glEnable(GL_DEPTH_TEST); // Habilitar test de profundidad para 3D

	// ------------------------------------------------------------------------
	// 4. CONFIGURACIÓN DE CALLBACKS
	// ------------------------------------------------------------------------

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// ------------------------------------------------------------------------
	// 5. CREACIÓN DE OBJETOS DE RENDERIZADO
	// ------------------------------------------------------------------------

	gfx::TextureCube cubemap;		  // Textura del skybox (6 caras)
	gfx::SkyboxRenderer skybox;		  // Renderizador del cielo
	gfx::TerrainRenderer terrain;	  // Renderizador del terreno
	gfx::TerrainParams terrainParams; // Parámetros del terreno
	hud::FlightHUD flightHUD;		  // Sistema de HUD
	gfx::Shader modelShader("shaders/model.vert", "shaders/model.frag");
	Model f16Model("assets/models/f16.glb");

	globalHUD = &flightHUD; // Guardar puntero global para callbacks

	// ------------------------------------------------------------------------
	// 6. INICIALIZACIÓN DE RECURSOS GRÁFICOS
	// ------------------------------------------------------------------------

	try
	{
		// Skybox: cargar atlas y compilar shaders
		if (!cubemap.loadFromAtlas("assets/textures/skybox/Cubemap_Sky_22-512x512.png", false))
		{
			std::cerr << "Failed to load cubemap atlas" << std::endl;
			return -1;
		}
		skybox.init();
		skybox.setCubemap(&cubemap);

		// Terreno: generar mesh, cargar texturas
		terrain.init();
		terrain.loadTextures("assets/textures/terrain");

		// Configurar parámetros del terreno
		terrainParams.groundY = 0.0f;		  // Nivel del piso
		terrainParams.tileScaleMacro = 0.05f; // Escala textura principal
		terrainParams.tileScaleDetail = 0.4f; // Escala textura de detalle
		terrainParams.detailStrength = 0.3f;  // Mezcla de detalle (0-1)
		terrainParams.fogDensity = 0.005f;	  // Niebla

		// HUD: compilar shaders, inicializar altímetro
		flightHUD.init(kWindowWidth, kWindowHeight);
		flightHUD.setLayout("classic");

		// Waypoint System: inicializar renderizador y waypoints
		waypointRenderer.init();
		initializeWaypoints();

		std::cout << "✓ All systems initialized successfully!" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "✗ Error initializing systems: " << e.what() << std::endl;
		return -1;
	}

	// ------------------------------------------------------------------------
	// 7. LOOP PRINCIPAL DE RENDERIZADO
	// ------------------------------------------------------------------------

	while (!glfwWindowShouldClose(window))
	{

		// --- Timing ---
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// --- Input ---
		processInput(window);

		// --- Actualización de lógica ---
		// Mover el avión hacia adelante en la dirección que apunta
		planePos += planeOrientation * glm::vec3(0, 0, -1) * planeSpeed * deltaTime;

		// Actualizar datos de vuelo para el HUD
		glm::vec3 euler = glm::eulerAngles(planeOrientation);
		flightData.altitude = planePos.y;
		flightData.airspeed = planeSpeed;
		flightData.pitch = -glm::degrees(euler.x);
		flightData.roll = glm::degrees(euler.z);
		flightData.heading = glm::degrees(euler.y);

		// Actualizar datos de waypoint
		updateWaypointData();

		// --- Manejo de resize de ventana ---
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		static int lastWidth = width, lastHeight = height;
		if (width != lastWidth || height != lastHeight)
		{
			flightHUD.setScreenSize(width, height);
			lastWidth = width;
			lastHeight = height;
		}

		// --- Limpiar buffers ---
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- Actualizar cámara para seguir al avión ---
		glm::vec3 forward = planeOrientation * glm::vec3(0, 0, -1);
		glm::vec3 up = planeOrientation * glm::vec3(0, 1, 0);

		glm::mat4 view;
		if (firstPersonView)
		{
			// Primera persona (POV) - cámara dentro del cockpit
			cameraPos = planePos + forward * 2.0f + up * 1.5f; // Adelante y arriba (cockpit)
			view = glm::lookAt(cameraPos, cameraPos + forward, up);
		}
		else
		{
			// Tercera persona - cámara detrás del avión
			cameraPos = planePos - forward * cameraDistance + up * 5.0f;
			view = glm::lookAt(cameraPos, planePos, up);
		}
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			(float)width / (float)height,
			0.1f,	// Near plane
			5000.0f // Far plane
		);

		// --- Renderizado 3D ---
		skybox.draw(view, projection);
		terrain.draw(view, projection, cameraPos, terrainParams);

		// --- Render F-16 ---
		modelShader.use();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		modelShader.setVec3("viewPos", cameraPos);

		// Set light properties - luz direccional desde arriba y adelante
		glm::vec3 sunDirection = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
		glm::vec3 lightPosition = planePos + sunDirection * 500.0f;
		modelShader.setVec3("lightPos", lightPosition);
		modelShader.setVec3("lightColor", glm::vec3(1.5f, 1.5f, 1.4f)); // Luz más intensa y cálida

		// === Corrección de orientación del modelo GLB ===
		// Definir ejes locales del modelo tal como viene del GLB
		glm::vec3 modelForward = glm::vec3(+1, 0, 0); // nariz apunta +X
		glm::vec3 modelUp = glm::vec3(0, 0, -1);	  // arriba del modelo -Z

		// Ejes deseados en el mundo
		glm::vec3 worldForward = glm::vec3(0, 0, -1); // queremos mirar hacia -Z
		glm::vec3 worldUp = glm::vec3(0, +1, 0);	  // arriba +Y

		// Paso 1: alinear la nariz (forward)
		glm::quat q1 = glm::rotation(glm::normalize(modelForward), glm::normalize(worldForward));

		// Paso 2: corregir el "roll" para que el up quede bien
		glm::vec3 upAfter = glm::normalize(q1 * modelUp);
		// Proyectamos "upAfter" al plano perpendicular a worldForward
		glm::vec3 axis = glm::normalize(worldForward);
		glm::vec3 upProjected = glm::normalize(upAfter - glm::dot(upAfter, axis) * axis);
		float cosang = glm::clamp(glm::dot(upProjected, worldUp), -1.0f, 1.0f);
		float angle = acosf(cosang);
		// Sentido de giro usando producto vectorial
		glm::vec3 crossv = glm::cross(upProjected, worldUp);
		float sign = (glm::dot(crossv, axis) < 0.f) ? -1.f : +1.f;
		glm::quat q2 = glm::angleAxis(sign * angle, axis);

		// Corrección final
		glm::mat4 Rcorr = glm::mat4_cast(q2 * q1);

		// Construir matriz del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, planePos);
		model = model * glm::mat4_cast(planeOrientation); // yaw/pitch/roll dinámicos
		model = model * Rcorr;							  // corrección fija del GLB
		model = glm::scale(model, glm::vec3(0.05f));
		modelShader.setMat4("model", model);

		f16Model.Draw(modelShader);

		// --- Renderizado de Waypoints 3D ---
		if (waypointSystemEnabled && !missionCompleted)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			for (size_t i = 0; i < waypoints.size(); ++i)
			{
				// Solo renderizar waypoints que NO han sido capturados
				if (!waypoints[i].captured)
				{
					bool isActive = (i == static_cast<size_t>(activeWaypointIndex));
					glm::vec4 color = isActive
						? glm::vec4(0.0f, 1.0f, 0.4f, 0.8f)  // Verde brillante para activo
						: glm::vec4(0.2f, 0.5f, 1.0f, 0.6f); // Azul tenue para próximos
					
					waypointRenderer.drawWaypoint(view, projection, waypoints[i].position, color, isActive);
				}
			}
			
			glDisable(GL_BLEND);
		}

		// --- Renderizado 2D (HUD overlay) ---
		// Solo mostrar HUD en primera persona (POV)
		if (firstPersonView)
		{
			flightHUD.update(flightData);
			flightHUD.render();
		}

		// --- Swap y eventos ---
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ------------------------------------------------------------------------
	// 8. LIMPIEZA Y CIERRE
	// ------------------------------------------------------------------------

	std::cout << "Cleaning up resources..." << std::endl;
	return 0;
}

// ============================================================================
// CALLBACKS
// ============================================================================

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	float rotSpeed = 1.5f * deltaTime;

	// Pitch
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(rotSpeed, glm::vec3(1, 0, 0)) * planeOrientation;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(-rotSpeed, glm::vec3(1, 0, 0)) * planeOrientation;
	}
	// Yaw
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(rotSpeed, glm::vec3(0, 1, 0)) * planeOrientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(-rotSpeed, glm::vec3(0, 1, 0)) * planeOrientation;
	}
	// Roll
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(rotSpeed, glm::vec3(0, 0, 1)) * planeOrientation;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		planeOrientation = glm::angleAxis(-rotSpeed, glm::vec3(0, 0, 1)) * planeOrientation;
	}

	// Speed
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		planeSpeed += 20.0f * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		planeSpeed -= 20.0f * deltaTime;
	}
	if (planeSpeed < 10.0f)
	{
		planeSpeed = 10.0f;
	}

	// Camera view toggle
	static bool vKeyWasPressed = false;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		if (!vKeyWasPressed)
		{
			firstPersonView = !firstPersonView;
			vKeyWasPressed = true;
		}
	}
	else
	{
		vKeyWasPressed = false;
	}

	// Waypoint system toggle (N key)
	static bool nKeyWasPressed = false;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		if (!nKeyWasPressed)
		{
			waypointSystemEnabled = !waypointSystemEnabled;
			std::cout << "Sistema de waypoints: "
					  << (waypointSystemEnabled ? "ACTIVADO" : "DESACTIVADO") << std::endl;
			nKeyWasPressed = true;
		}
	}
	else
	{
		nKeyWasPressed = false;
	}

	// Next waypoint (M key) - Solo si no está completada la misión
	static bool mKeyWasPressed = false;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		if (!mKeyWasPressed && waypointSystemEnabled && !waypoints.empty() && !missionCompleted)
		{
			// Marcar el actual como capturado y avanzar
			if (activeWaypointIndex < static_cast<int>(waypoints.size()))
			{
				waypoints[activeWaypointIndex].captured = true;
				std::cout << "Waypoint " << waypoints[activeWaypointIndex].name << " saltado manualmente" << std::endl;
			}
			mKeyWasPressed = true;
		}
	}
	else
	{
		mKeyWasPressed = false;
	}

	// Restart mission (R key)
	static bool rKeyWasPressed = false;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		if (!rKeyWasPressed)
		{
			initializeWaypoints();
			std::cout << "Misión reiniciada!" << std::endl;
			rKeyWasPressed = true;
		}
	}
	else
	{
		rKeyWasPressed = false;
	}

	// Movement - SPACE to move forward
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		// Movement is always active when SPACE is held
		// The plane moves in the direction it's pointing
	}
}

void print_gl_version(void)
{
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);

	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;
}

// ============================================================================
// FUNCIONES DEL SISTEMA DE WAYPOINTS
// ============================================================================

void initializeWaypoints()
{
	waypoints.clear();
	missionCompleted = false;
	
	// Circuito de waypoints ampliado para mejor maniobra (distancias ~1500-2000m)
	waypoints.push_back({glm::vec3(1500.0f, 120.0f, 0.0f), "WPT-1", false});
	waypoints.push_back({glm::vec3(1500.0f, 150.0f, -1500.0f), "WPT-2", false});
	waypoints.push_back({glm::vec3(0.0f, 200.0f, -2000.0f), "WPT-3", false});
	waypoints.push_back({glm::vec3(-1500.0f, 150.0f, -1500.0f), "WPT-4", false});
	waypoints.push_back({glm::vec3(-1800.0f, 120.0f, 0.0f), "WPT-5", false});
	waypoints.push_back({glm::vec3(0.0f, 100.0f, 0.0f), "HOME", false});
	
	activeWaypointIndex = 0;
	waypointSystemEnabled = true;
	
	std::cout << "\n========================================" << std::endl;
	std::cout << "      🎯 MISIÓN INICIADA 🎯" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "Waypoints: " << waypoints.size() << " cargados" << std::endl;
	std::cout << "Objetivo: Pasar por todos los waypoints" << std::endl;
	std::cout << "\nControles:" << std::endl;
	std::cout << "  V - Cambiar vista POV/3ra persona" << std::endl;
	std::cout << "  N - Activar/Desactivar waypoints" << std::endl;
	std::cout << "  M - Saltar waypoint actual" << std::endl;
	std::cout << "  R - Reiniciar misión" << std::endl;
	std::cout << "========================================\n" << std::endl;
}

void updateWaypointData()
{
	if (!waypointSystemEnabled || waypoints.empty() || missionCompleted)
	{
		flightData.hasActiveWaypoint = false;
		return;
	}
	
	// Buscar el primer waypoint no capturado
	int nextWaypointIndex = -1;
	for (size_t i = 0; i < waypoints.size(); ++i)
	{
		if (!waypoints[i].captured)
		{
			nextWaypointIndex = i;
			break;
		}
	}
	
	// Si no hay más waypoints, misión completada
	if (nextWaypointIndex == -1)
	{
		if (!missionCompleted)
		{
			missionCompleted = true;
			std::cout << "\n" << std::endl;
			std::cout << "========================================" << std::endl;
			std::cout << "   ✅ MISI\u00d3N COMPLETADA CON \u00c9XITO!   " << std::endl;
			std::cout << "========================================" << std::endl;
			std::cout << "Todos los waypoints han sido alcanzados." << std::endl;
			std::cout << "\n" << std::endl;
		}
		flightData.hasActiveWaypoint = false;
		return;
	}
	
	activeWaypointIndex = nextWaypointIndex;
	const Waypoint& currentWaypoint = waypoints[activeWaypointIndex];
	flightData.targetWaypoint = currentWaypoint.position;
	flightData.hasActiveWaypoint = true;
	
	// Calcular distancia
	glm::vec3 toWaypoint = currentWaypoint.position - planePos;
	flightData.waypointDistance = glm::length(toWaypoint);
	
	// Calcular bearing (rumbo al waypoint)
	// Proyección en plano horizontal (XZ)
	glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);
	
	if (glm::length(toWaypointXZ) > 0.01f)
	{
		// Calcular bearing: 0° = norte (-Z), sentido horario
		float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180.0f / M_PI);
		if (bearing < 0.0f) bearing += 360.0f;
		flightData.waypointBearing = bearing;
	}
	
	// Captura automática de waypoint (radio ampliado para waypoints más distantes)
	const float WAYPOINT_CAPTURE_RADIUS = 80.0f;
	if (flightData.waypointDistance < WAYPOINT_CAPTURE_RADIUS)
	{
		waypoints[activeWaypointIndex].captured = true;
		
		// Contar waypoints restantes
		int remaining = 0;
		for (const auto& wp : waypoints)
		{
			if (!wp.captured) remaining++;
		}
		
		std::cout << "✓ Waypoint " << currentWaypoint.name << " alcanzado! ";
		if (remaining > 0)
		{
			std::cout << "Waypoints restantes: " << remaining << std::endl;
		}
		else
		{
			std::cout << "\u00daltimo waypoint!" << std::endl;
		}
	}
}
