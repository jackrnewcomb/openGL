// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>

int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Lab 3", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the "
                        "2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture any key being pressed (in our case, L or esc)
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    // Create and bind a vertex array
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Load the texture
    GLuint Texture = loadDDS("uvmap.DDS");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    bool res = loadAssImp("suzanne.obj", indices, indexed_vertices, indexed_uvs, indexed_normals);

    // Lets check if we successfully loaded suzanne
    if (!res)
    {
        fprintf(stderr, "Failed to load suzanne\n");
        return -1;
    }

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);

    // Set lighting info
    GLuint lightOnID = glGetUniformLocation(programID, "lightOn");
    glUniform1i(lightOnID, 1);

    // Vertex positions for a 10x10 rectangle on the z=0 plane
    static const GLfloat ground_vertices[] = {-5.0f, -5.0f, 0.0f, 5.0f, -5.0f, 0.0f,
                                              -5.0f, 5.0f,  0.0f, 5.0f, 5.0f,  0.0f};

    // UV texture coords
    static const GLfloat ground_uvs[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    // Vertex normals (all facing Z direction) for flat lighting across the rectangle
    static const GLfloat ground_normals[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    // Indices that create the triangles that make the rectangle
    static const GLuint ground_indices[] = {0, 1, 2, 2, 1, 3};

    GLuint groundVertexBuffer, groundUVBuffer, groundNormalBuffer, groundElementBuffer;
    glGenBuffers(1, &groundVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, groundVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &groundUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, groundUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_uvs), ground_uvs, GL_STATIC_DRAW);

    glGenBuffers(1, &groundNormalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, groundNormalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_normals), ground_normals, GL_STATIC_DRAW);

    glGenBuffers(1, &groundElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_indices), ground_indices, GL_STATIC_DRAW);

    GLuint greenTex;
    glGenTextures(1, &greenTex);
    glBindTexture(GL_TEXTURE_2D, greenTex);
    // single green pixel (RGB)
    unsigned char pixel[3] = {0, 204, 0};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    do
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Lighting things
        bool lastL = false;
        bool lightOn = true;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            if (!lastL)
            {
                lightOn = !lightOn; // toggle the light on or off
                GLuint lightOnID = glGetUniformLocation(programID, "lightOn");
                glUniform1i(lightOnID, lightOn);
            }
            lastL = true;
        }
        else
        {
            lastL = false;
        }

        // Compute the MVP matrix from keyboard
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // Do some work to draw the green rectangle
        {
            // Set up the ground transformation
            glm::mat4 groundModel = glm::mat4(1.0f);
            glm::mat4 groundMVP = ProjectionMatrix * ViewMatrix * groundModel;

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &groundMVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &groundModel[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            // Needed to ensure ground plane is visible from both sides
            glDisable(GL_CULL_FACE);

            // First attr. buffer, vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, groundVertexBuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Second attr. buffer, UVs
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, groundUVBuffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Third attr. buffer, normals
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, groundNormalBuffer);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Bind our green 1x1 texture to 0 so green color populates
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, greenTex);
            glUniform1i(TextureID, 0);

            // index and draw
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundElementBuffer);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

            // cleanup attributes
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);

            glEnable(GL_CULL_FACE); // re-enable cull face for monkeys
        }
        // Set up some values for drawing heads
        int numHeads = 8;
        float radius = 3.75f;    // trial and error to get ears to touch, looks right
        float chinOffset = 1.0f; // more trial and error

        // For each head...
        for (int i = 0; i < numHeads; i++)
        {
            float anglePerHead = 360 / numHeads;
            float angle = glm::radians(anglePerHead * i);

            // Position head in a circle around the origin
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            float z = chinOffset;

            glm::mat4 ModelMatrix = glm::mat4(1.0f);

            // Translate the head into position
            ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, z));

            // Rotate so the head faces radially outward
            ModelMatrix = glm::rotate(ModelMatrix, angle + glm::radians(90.0f), glm::vec3(0, 0, 1));

            // Rotate so their chins are touching green rectangle
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            // Bind our texture in Texture 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture);
            glUniform1i(TextureID, 0);

            // First attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Second attribute buffer : UVs
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Third attribute buffer : normals
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

            // Draw
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void *)0);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
