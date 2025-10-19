// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow *window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp.
                           // This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
    return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{
    return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second

void computeMatricesFromInputs()
{
    static double lastTime = glfwGetTime();
    static float theta = 0.0f; // horizontal angle
    static float phi = 0.0f;   // vertical angle

    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    glm::vec3 origin = glm::vec3(0, 0, 0);

    float orbitSpeed = 1.5f;
    float zoomSpeed = 3.0f;

    // Handle input
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        theta += orbitSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        theta -= orbitSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        phi += orbitSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        phi -= orbitSpeed * deltaTime;

    // Keep phi continuous, allow full orbit without flipping
    const float PI = 3.14159265f;
    if (phi > PI)
        phi -= 2 * PI;
    if (phi < -PI)
        phi += 2 * PI;

    // Calculate radius (zoom)
    static float r = glm::length(position);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        r -= zoomSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        r += zoomSpeed * deltaTime;
    r = glm::clamp(r, 1.0f, 50.0f);

    // Compute Cartesian coordinates from angles
    position.x = r * cos(phi) * sin(theta);
    position.y = r * sin(phi);
    position.z = r * cos(phi) * cos(theta);

    // Always look at origin
    glm::vec3 direction = glm::normalize(origin - position);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    up = glm::cross(direction, right);

    // Projection & View
    float FoV = 45.0f;
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    ViewMatrix = glm::lookAt(position, origin, up);

    lastTime = currentTime;
}
