#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

#include "draw.hpp"

int main() {
    glfwInit();

    // Define window hints to create an offscreen context
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window object with the specified hints
    GLFWwindow* window = glfwCreateWindow(64, 64, "Offscreen Context", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the context current
    glfwMakeContextCurrent(window);

    // Draw a red square to the offscreen context
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(-0.5f, 0.5f);
    glEnd();

    // Get the pixel data with a PBO
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, 64 * 64 * 4, NULL, GL_STREAM_READ);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    GLubyte* pixels = (GLubyte*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    // Call setpixel with the pixel data
    // ...
    // Print the first pixel
    // std::cout << (int) pixels[0] << " " << (int) pixels[1] << " " << (int) pixels[2] << " " << (int) pixels[3] << std::endl;
    draw(pixels);

    // Unmap and delete the PBO
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers(1, &pbo);

    // Terminate GLFW
    glfwTerminate();

    return 0;
}