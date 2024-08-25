#pragma once

#include "Global.hpp"

namespace aEngine {

namespace Render {

// OpenGL buffer object
class Buffer {
public:
  Buffer() { glGenBuffers(1, &ID); }
  // Bind the buffer to target, setup the filled data in it.
  template <typename T>
  void SetDataAs(GLenum TARGET_BUFFER_NAME, const std::vector<T> &data,
                 GLenum usage = GL_STATIC_DRAW) {
    glBindBuffer(TARGET_BUFFER_NAME, ID);
    glBufferData(TARGET_BUFFER_NAME, data.size() * sizeof(T),
                 (void *)data.data(), usage);
  }
  // Update the data in this buffer as described in offset. Make sure the buffer
  // has enough space for update.
  template <typename T>
  void UpdateDataAs(GLenum TARGET_BUFFER_NAME, const std::vector<T> &data,
                    size_t offset) {
    glBindBuffer(TARGET_BUFFER_NAME, ID);
    glBufferSubData(TARGET_BUFFER_NAME, offset, data.size() * sizeof(T),
                    data.data());
  }
  void BindAs(GLenum TARGET_BUFFER_NAME) {
    glBindBuffer(TARGET_BUFFER_NAME, ID);
  }
  // Bind SSBO, UBO to the bindingPoint of some shader, so that
  // these buffers can be accessed in this shader.
  void BindToPointAs(GLenum TARGET_BUFFER_NAME, GLuint bindingPoint) {
    glBindBufferBase(TARGET_BUFFER_NAME, bindingPoint, ID);
  }
  // Unbind the buffer from target
  void UnbindAs(GLenum TARGET_BUFFER_NAME) {
    glBindBuffer(TARGET_BUFFER_NAME, 0);
  }
  // Free the buffer from GPU
  void Delete() {
    if (glIsBuffer(ID))
      glDeleteBuffers(1, &ID);
  }
  GLuint GetID() const { return ID; }

private:
  GLuint ID;
};

// OpenGL vertex array object
class VAO {
public:
  VAO() { glGenVertexArrays(1, &ID); }

  void Bind() const { glBindVertexArray(ID); }

  void Unbind() const { glBindVertexArray(0); }

  void Delete() const { glDeleteVertexArrays(1, &ID); }

  void LinkAttrib(Buffer &vbo, GLuint layout, GLint size, GLenum type,
                  GLsizei stride, void *offset) {
    vbo.BindAs(GL_ARRAY_BUFFER);
    glVertexAttribPointer(layout, size, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    vbo.UnbindAs(GL_ARRAY_BUFFER);
  }
  GLuint GetID() const { return ID; }

private:
  GLuint ID;
};

}; // namespace Render

}; // namespace aEngine