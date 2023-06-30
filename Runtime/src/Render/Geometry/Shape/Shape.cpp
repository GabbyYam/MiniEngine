#include "Shape.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stdint.h>

namespace suplex {
    namespace utils {
        enum class Shape { Cube, Quad, Sphere };

        void RenderQuad(const std::shared_ptr<Shader>& shader)
        {
            static unsigned int quadVAO = 0;
            static unsigned int quadVBO = 0;

            shader->Bind();
            // debug("Draw Quad");
            if (quadVAO == 0) {
                float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
                };
                // setup plane VAO
                glGenVertexArrays(1, &quadVAO);
                glGenBuffers(1, &quadVBO);
                glBindVertexArray(quadVAO);
                glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            }
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }

        void RenderCube(const std::shared_ptr<Shader>& shader)
        {
            static unsigned int cubeVAO = 0;
            static unsigned int cubeVBO = 0;

            shader->Bind();
            // initialize (if necessary)
            if (cubeVAO == 0) {
                debug("Draw Cube");

                float vertices[] = {
                    // back face
                    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
                    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,    // top-right
                    1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,   // bottom-right
                    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,    // top-right
                    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
                    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,   // top-left
                    // front face
                    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
                    1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,   // bottom-right
                    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,    // top-right
                    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,    // top-right
                    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,   // top-left
                    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
                    // left face
                    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,    // top-right
                    -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top-left
                    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
                    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
                    -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // bottom-right
                    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,    // top-right
                                                                         // right face
                    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,      // top-left
                    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,    // bottom-right
                    1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,     // top-right
                    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,    // bottom-right
                    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,      // top-left
                    1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,     // bottom-left
                    // bottom face
                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // top-right
                    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,   // top-left
                    1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,    // bottom-left
                    1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,    // bottom-left
                    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // bottom-right
                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // top-right
                    // top face
                    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top-left
                    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,    // bottom-right
                    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top-right
                    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,    // bottom-right
                    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top-left
                    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f    // bottom-left
                };
                glGenVertexArrays(1, &cubeVAO);
                glGenBuffers(1, &cubeVBO);
                // fill buffer
                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                // link vertex attributes
                glBindVertexArray(cubeVAO);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
            // render Cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        void RenderSphere(const std::shared_ptr<Shader>& shader)
        {
            static unsigned int sphereVAO = 0;
            static unsigned int indexCount;
            shader->Bind();
            // debug("Draw Sphere");
            if (sphereVAO == 0) {
                glGenVertexArrays(1, &sphereVAO);

                unsigned int vbo, ebo;
                glGenBuffers(1, &vbo);
                glGenBuffers(1, &ebo);

                std::vector<glm::vec3>    positions;
                std::vector<glm::vec2>    uv;
                std::vector<glm::vec3>    normals;
                std::vector<unsigned int> indices;

                const unsigned int X_SEGMENTS = 64;
                const unsigned int Y_SEGMENTS = 64;
                const float        PI         = 3.14159265359f;
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                        float xSegment = (float)x / (float)X_SEGMENTS;
                        float ySegment = (float)y / (float)Y_SEGMENTS;
                        float xPos     = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                        float yPos     = std::cos(ySegment * PI);
                        float zPos     = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                        positions.push_back(glm::vec3(xPos, yPos, zPos));
                        uv.push_back(glm::vec2(xSegment, ySegment));
                        normals.push_back(glm::vec3(xPos, yPos, zPos));
                    }
                }

                bool oddRow = false;
                for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
                    if (!oddRow)  // even rows: y == 0, y == 2; and so on
                    {
                        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                            indices.push_back(y * (X_SEGMENTS + 1) + x);
                            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                        }
                    }
                    else {
                        for (int x = X_SEGMENTS; x >= 0; --x) {
                            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                            indices.push_back(y * (X_SEGMENTS + 1) + x);
                        }
                    }
                    oddRow = !oddRow;
                }
                indexCount = static_cast<unsigned int>(indices.size());

                std::vector<float> data;
                for (unsigned int i = 0; i < positions.size(); ++i) {
                    data.push_back(positions[i].x);
                    data.push_back(positions[i].y);
                    data.push_back(positions[i].z);
                    if (normals.size() > 0) {
                        data.push_back(normals[i].x);
                        data.push_back(normals[i].y);
                        data.push_back(normals[i].z);
                    }
                    if (uv.size() > 0) {
                        data.push_back(uv[i].x);
                        data.push_back(uv[i].y);
                    }
                }
                glBindVertexArray(sphereVAO);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
                unsigned int stride = (3 + 2 + 3) * sizeof(float);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
            }

            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
        }

        void RenderGird(const std::shared_ptr<Shader>& shader)
        {
            static uint32_t gridVAO = 0;
            static uint32_t gridVBO = 0;
            static float    slices  = 100.0f;
            static uint32_t length;
            static float    space = 1000.0f;

            if (gridVAO == 0) {
                std::vector<glm::vec3>  vertices;
                std::vector<glm::uvec4> indices;

                for (int j = 0; j <= slices; ++j) {
                    for (int i = 0; i <= slices; ++i) {
                        float x = (float)i / slices;
                        float z = (float)j / slices;
                        x       = (x - 0.5f) * 0.5f;
                        z       = (z - 0.5f) * 0.5f;
                        vertices.push_back(glm::vec3(x * space, 0, z * space));
                    }
                }

                for (int j = 0; j < slices; ++j) {
                    for (int i = 0; i < slices; ++i) {
                        int row1 = j * (slices + 1);
                        int row2 = (j + 1) * (slices + 1);

                        indices.push_back(glm::uvec4(row1 + i, row1 + i + 1, row1 + i + 1, row2 + i + 1));
                        indices.push_back(glm::uvec4(row2 + i + 1, row2 + i, row2 + i, row1 + i));
                    }
                }

                glGenVertexArrays(1, &gridVAO);
                glBindVertexArray(gridVAO);

                glGenBuffers(1, &gridVBO);
                glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                GLuint ibo;
                glGenBuffers(1, &ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec4), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

                glBindVertexArray(0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                length = (GLuint)indices.size() * 4;
            }
            glBindVertexArray(gridVAO);
            glDrawElements(GL_LINES, length, GL_UNSIGNED_INT, NULL);
            glBindVertexArray(0);
        }
    }  // namespace utils
}  // namespace suplex