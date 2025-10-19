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
float mouseSpeed = 0.005f;

void computeMatricesFromInputs()
{

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    glm::vec3 origin = glm::vec3(0, 0, 0);

    float orbitSpeed = 1.0f;

    // r = distance from origin
    float r = glm::length(position);

    // Compute current spherical angles
    float theta = atan2(position.x, position.z);
    float phi = asin(position.y / r);

    // Horizontal orbit
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        theta -= orbitSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        theta += orbitSpeed * deltaTime;

    // Vertical orbit
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        phi += orbitSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        phi -= orbitSpeed * deltaTime;

    // Radial zoom
    float zoomSpeed = 3.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        r -= zoomSpeed * deltaTime; // zoom in
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        r += zoomSpeed * deltaTime; // zoom out

    // Optional: prevent camera from getting too close or too far
    r = glm::clamp(r, 1.0f, 50.0f);

    // Convert back to Cartesian coordinates
    position.x = r * cos(phi) * sin(theta);
    position.y = r * sin(phi);
    position.z = r * cos(phi) * cos(theta);

    // Compute direction and up vector
    glm::vec3 direction = glm::normalize(origin - position);
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
    glm::vec3 up = glm::cross(direction, right);

    float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this.
                            // It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    ViewMatrix = glm::lookAt(position, // Camera is here
                             origin,   // and looks here : at the same position, plus "direction"
                             up        // Head is up (set to 0,-1,0 to look upside-down)
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}
