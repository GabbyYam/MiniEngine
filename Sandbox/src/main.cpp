// #include "Animation/Animation/Animation.hpp"
// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

// #include <filesystem>
// #include <Render/Shader/Shader.hpp>
// #include <Render/Camera/Camera.hpp>
// #include <Animation/Animator/Animator.hpp>
// #include <Render/Geometry/Model.hpp>

// #include <iostream>
// #include <memory>
// #include <spdlog/spdlog.h>

// using namespace suplex;

// void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// void processInput(GLFWwindow* window);

// // settings
// const unsigned int SCR_WIDTH  = 800;
// const unsigned int SCR_HEIGHT = 600;

// // camera
// Camera camera(45.0f, 0.1f, 100.0f);
// float  lastX      = SCR_WIDTH / 2.0f;
// float  lastY      = SCR_HEIGHT / 2.0f;
// bool   firstMouse = true;

// // timing
// float deltaTime = 0.0f;
// float lastFrame = 0.0f;

// int main()
// {
//     // glfw: initialize and configure
//     // ------------------------------
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
// #endif

//     // glfw window creation
//     // --------------------
//     GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//     if (window == NULL) {
//         std::cout << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }
//     glfwMakeContextCurrent(window);
//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//     glfwSetCursorPosCallback(window, mouse_callback);
//     glfwSetScrollCallback(window, scroll_callback);

//     // tell GLFW to capture our mouse
//     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

//     // glad: load all OpenGL function pointers
//     // ---------------------------------------
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         return -1;
//     }

//     // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//     stbi_set_flip_vertically_on_load(true);

//     // configure global opengl state
//     // -----------------------------
//     glEnable(GL_DEPTH_TEST);

//     // build and compile shaders
//     // -------------------------
//     auto shader = std::make_shared<Shader>("anim_model.vs", "anim_model.fs");

//     // load models
//     // -----------
//     std::string modelFilename;
//     std::string animationFilename;

//     auto model          = std::make_shared<Model>(modelFilename);
//     auto danceAnimation = std::make_shared<Animation>(animationFilename, model.get());
//     auto animator       = std::make_shared<Animator>(danceAnimation);

//     info("Load animation successful");

//     // draw in wireframe
//     //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//     // render loop
//     // -----------
//     while (!glfwWindowShouldClose(window)) {
//         // per-frame time logic
//         // --------------------
//         float currentFrame = glfwGetTime();
//         deltaTime          = currentFrame - lastFrame;
//         lastFrame          = currentFrame;

//         // input
//         // -----
//         processInput(window);
//         animator->UpdateAnimation(deltaTime);

//         // render
//         // ------
//         glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//         // don't forget to enable shader before setting uniforms
//         shader->Bind();

//         // view/projection transformations
//         auto const& proj = camera.GetProjection();
//         auto const& view = camera.GetView();
//         shader->SetMaterix4("view", glm::value_ptr(view));
//         shader->SetMaterix4("proj", glm::value_ptr(proj));

//         auto transforms = animator->GetFinalBoneMatrices();
//         for (int i = 0; i < transforms.size(); ++i)
//             shader->SetMaterix4("boneTransform[" + std::to_string(i) + "]", glm::value_ptr(transforms[i]));

//         // render the loaded model
//         glm::mat4 modelMatrix = glm::mat4(1.0f);
//         modelMatrix =
//             glm::translate(modelMatrix, glm::vec3(0.0f, -0.4f, 0.0f));    // translate it down so it's at the center of the scene
//         modelMatrix = glm::scale(modelMatrix, glm::vec3(.5f, .5f, .5f));  // it's a bit too big for our scene, so scale it down
//         shader->SetMaterix4("model", glm::value_ptr(modelMatrix));
//         for (auto& mesh : model->GetMeshes()) mesh.Render(shader);

//         // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//         // -------------------------------------------------------------------------------
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     // glfw: terminate, clearing all previously allocated GLFW resources.
//     // ------------------------------------------------------------------
//     glfwTerminate();
//     return 0;
// }

// // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// // ---------------------------------------------------------------------------------------------------------
// void processInput(GLFWwindow* window)
// {
//     if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

//     // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
//     // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
//     // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
//     // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
// }

// // glfw: whenever the window size changed (by OS or user resize) this callback function executes
// // ---------------------------------------------------------------------------------------------
// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
//     // make sure the viewport matches the new window dimensions; note that width and
//     // height will be significantly larger than specified on retina displays.
//     glViewport(0, 0, width, height);
// }

// // glfw: whenever the mouse moves, this callback is called
// // -------------------------------------------------------
// void mouse_callback(GLFWwindow* window, double xpos, double ypos)
// {
//     if (firstMouse) {
//         lastX      = xpos;
//         lastY      = ypos;
//         firstMouse = false;
//     }

//     float xoffset = xpos - lastX;
//     float yoffset = lastY - ypos;  // reversed since y-coordinates go from bottom to top

//     lastX = xpos;
//     lastY = ypos;

//     // camera.ProcessMouseMovement(xoffset, yoffset);
// }

// // glfw: whenever the mouse scroll wheel scrolls, this callback is called
// // ----------------------------------------------------------------------
// // void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll(yoffset); }

#include <iostream>

int main() {}