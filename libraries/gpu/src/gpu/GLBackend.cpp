//
//  GLBackend.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 10/27/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLBackendShared.h"

GLBackend::CommandCall GLBackend::_commandCalls[Batch::NUM_COMMANDS] = 
{
    (&::gpu::GLBackend::do_draw),
    (&::gpu::GLBackend::do_drawIndexed),
    (&::gpu::GLBackend::do_drawInstanced),
    (&::gpu::GLBackend::do_drawIndexedInstanced),

    (&::gpu::GLBackend::do_setInputFormat),
    (&::gpu::GLBackend::do_setInputBuffer),
    (&::gpu::GLBackend::do_setIndexBuffer),

    (&::gpu::GLBackend::do_setModelTransform),
    (&::gpu::GLBackend::do_setViewTransform),
    (&::gpu::GLBackend::do_setProjectionTransform),

    (&::gpu::GLBackend::do_setPipeline),
    (&::gpu::GLBackend::do_setUniformBuffer),
    (&::gpu::GLBackend::do_setUniformTexture),

    (&::gpu::GLBackend::do_glEnable),
    (&::gpu::GLBackend::do_glDisable),

    (&::gpu::GLBackend::do_glEnableClientState),
    (&::gpu::GLBackend::do_glDisableClientState),

    (&::gpu::GLBackend::do_glCullFace),
    (&::gpu::GLBackend::do_glAlphaFunc),

    (&::gpu::GLBackend::do_glDepthFunc),
    (&::gpu::GLBackend::do_glDepthMask),
    (&::gpu::GLBackend::do_glDepthRange),

    (&::gpu::GLBackend::do_glBindBuffer),

    (&::gpu::GLBackend::do_glBindTexture),
    (&::gpu::GLBackend::do_glActiveTexture),

    (&::gpu::GLBackend::do_glDrawBuffers),

    (&::gpu::GLBackend::do_glUseProgram),
    (&::gpu::GLBackend::do_glUniform1f),
    (&::gpu::GLBackend::do_glUniform2f),
    (&::gpu::GLBackend::do_glUniform4fv),
    (&::gpu::GLBackend::do_glUniformMatrix4fv),

    (&::gpu::GLBackend::do_glDrawArrays),
    (&::gpu::GLBackend::do_glDrawRangeElements),

    (&::gpu::GLBackend::do_glColorPointer),
    (&::gpu::GLBackend::do_glNormalPointer),
    (&::gpu::GLBackend::do_glTexCoordPointer),
    (&::gpu::GLBackend::do_glVertexPointer),

    (&::gpu::GLBackend::do_glVertexAttribPointer),
    (&::gpu::GLBackend::do_glEnableVertexAttribArray),
    (&::gpu::GLBackend::do_glDisableVertexAttribArray),

    (&::gpu::GLBackend::do_glColor4f),
};

GLBackend::GLBackend() :
    _input(),
    _transform(),
    _pipeline()
{
    initTransform();
}

GLBackend::~GLBackend() {
    killTransform();
}

void GLBackend::render(Batch& batch) {

    uint32 numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();

    for (unsigned int i = 0; i < numCommands; i++) {
        CommandCall call = _commandCalls[(*command)];
        (this->*(call))(batch, *offset);
        command++;
        offset++;
    }
}

void GLBackend::renderBatch(Batch& batch) {
    GLBackend backend;
    backend.render(batch);
}

void GLBackend::checkGLError() {
    GLenum error = glGetError();
    if (!error) {
        return;
    }
    else {
        switch (error) {
        case GL_INVALID_ENUM:
            qDebug() << "An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_INVALID_VALUE:
            qDebug() << "A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
            break;
        case GL_INVALID_OPERATION:
            qDebug() << "The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            qDebug() << "The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_OUT_OF_MEMORY:
            qDebug() << "There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
            break;
        case GL_STACK_UNDERFLOW:
            qDebug() << "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            break;
        case GL_STACK_OVERFLOW:
            qDebug() << "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
            break;
        }
    }
}

void GLBackend::do_draw(Batch& batch, uint32 paramOffset) {
    updateInput();
    updateTransform();
    updatePipeline();

    Primitive primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
    GLenum mode = _primitiveToGLmode[primitiveType];
    uint32 numVertices = batch._params[paramOffset + 1]._uint;
    uint32 startVertex = batch._params[paramOffset + 0]._uint;

    glDrawArrays(mode, startVertex, numVertices);
    CHECK_GL_ERROR();
}

void GLBackend::do_drawIndexed(Batch& batch, uint32 paramOffset) {
    updateInput();
    updateTransform();
    updatePipeline();

    Primitive primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
    GLenum mode = _primitiveToGLmode[primitiveType];
    uint32 numIndices = batch._params[paramOffset + 1]._uint;
    uint32 startIndex = batch._params[paramOffset + 0]._uint;

    GLenum glType = _elementTypeToGLType[_input._indexBufferType];

    glDrawElements(mode, numIndices, glType, reinterpret_cast<GLvoid*>(startIndex + _input._indexBufferOffset));
    CHECK_GL_ERROR();
}

void GLBackend::do_drawInstanced(Batch& batch, uint32 paramOffset) {
    CHECK_GL_ERROR();
}

void GLBackend::do_drawIndexedInstanced(Batch& batch, uint32 paramOffset) {
    CHECK_GL_ERROR();
}

// TODO: As long as we have gl calls explicitely issued from interface
// code, we need to be able to record and batch these calls. THe long 
// term strategy is to get rid of any GL calls in favor of the HIFI GPU API

#define ADD_COMMAND_GL(call) _commands.push_back(COMMAND_##call); _commandOffsets.push_back(_params.size());

//#define DO_IT_NOW(call, offset) runLastCommand();
#define DO_IT_NOW(call, offset) 


void Batch::_glEnable(GLenum cap) {
    ADD_COMMAND_GL(glEnable);

    _params.push_back(cap);

    DO_IT_NOW(_glEnable, 1);
}
void GLBackend::do_glEnable(Batch& batch, uint32 paramOffset) {
    glEnable(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDisable(GLenum cap) {
    ADD_COMMAND_GL(glDisable);

    _params.push_back(cap);

    DO_IT_NOW(_glDisable, 1);
}
void GLBackend::do_glDisable(Batch& batch, uint32 paramOffset) {
    glDisable(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glEnableClientState(GLenum array) {
    ADD_COMMAND_GL(glEnableClientState);

    _params.push_back(array);

    DO_IT_NOW(_glEnableClientState, 1);
}
void GLBackend::do_glEnableClientState(Batch& batch, uint32 paramOffset) {
    glEnableClientState(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDisableClientState(GLenum array) {
    ADD_COMMAND_GL(glDisableClientState);

    _params.push_back(array);

    DO_IT_NOW(_glDisableClientState, 1);
}
void GLBackend::do_glDisableClientState(Batch& batch, uint32 paramOffset) {
    glDisableClientState(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glCullFace(GLenum mode) {
    ADD_COMMAND_GL(glCullFace);

    _params.push_back(mode);

    DO_IT_NOW(_glCullFace, 1);
}
void GLBackend::do_glCullFace(Batch& batch, uint32 paramOffset) {
    glCullFace(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glAlphaFunc(GLenum func, GLclampf ref) {
    ADD_COMMAND_GL(glAlphaFunc);

    _params.push_back(ref);
    _params.push_back(func);

    DO_IT_NOW(_glAlphaFunc, 2);
}
void GLBackend::do_glAlphaFunc(Batch& batch, uint32 paramOffset) {
    glAlphaFunc(
        batch._params[paramOffset + 1]._uint,
        batch._params[paramOffset + 0]._float);
    CHECK_GL_ERROR();
}

void Batch::_glDepthFunc(GLenum func) {
    ADD_COMMAND_GL(glDepthFunc);

    _params.push_back(func);

    DO_IT_NOW(_glDepthFunc, 1);
}
void GLBackend::do_glDepthFunc(Batch& batch, uint32 paramOffset) {
    glDepthFunc(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDepthMask(GLboolean flag) {
    ADD_COMMAND_GL(glDepthMask);

    _params.push_back(flag);

    DO_IT_NOW(_glDepthMask, 1);
}
void GLBackend::do_glDepthMask(Batch& batch, uint32 paramOffset) {
    glDepthMask(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDepthRange(GLclampd zNear, GLclampd zFar) {
    ADD_COMMAND_GL(glDepthRange);

    _params.push_back(zFar);
    _params.push_back(zNear);

    DO_IT_NOW(_glDepthRange, 2);
}
void GLBackend::do_glDepthRange(Batch& batch, uint32 paramOffset) {
    glDepthRange(
        batch._params[paramOffset + 1]._double,
        batch._params[paramOffset + 0]._double);
    CHECK_GL_ERROR();
}

void Batch::_glBindBuffer(GLenum target, GLuint buffer) {
    ADD_COMMAND_GL(glBindBuffer);

    _params.push_back(buffer);
    _params.push_back(target);

    DO_IT_NOW(_glBindBuffer, 2);
}
void GLBackend::do_glBindBuffer(Batch& batch, uint32 paramOffset) {
    glBindBuffer(
        batch._params[paramOffset + 1]._uint,
        batch._params[paramOffset + 0]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glBindTexture(GLenum target, GLuint texture) {
    ADD_COMMAND_GL(glBindTexture);

    _params.push_back(texture);
    _params.push_back(target);

    DO_IT_NOW(_glBindTexture, 2);
}
void GLBackend::do_glBindTexture(Batch& batch, uint32 paramOffset) {
    glBindTexture(
        batch._params[paramOffset + 1]._uint,
        batch._params[paramOffset + 0]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glActiveTexture(GLenum texture) {
    ADD_COMMAND_GL(glActiveTexture);

    _params.push_back(texture);

    DO_IT_NOW(_glActiveTexture, 1);
}
void GLBackend::do_glActiveTexture(Batch& batch, uint32 paramOffset) {
    glActiveTexture(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDrawBuffers(GLsizei n, const GLenum* bufs) {
    ADD_COMMAND_GL(glDrawBuffers);

    _params.push_back(cacheData(n * sizeof(GLenum), bufs));
    _params.push_back(n);

    DO_IT_NOW(_glDrawBuffers, 2);
}
void GLBackend::do_glDrawBuffers(Batch& batch, uint32 paramOffset) {
    glDrawBuffers(
        batch._params[paramOffset + 1]._uint,
        (const GLenum*)batch.editData(batch._params[paramOffset + 0]._uint));
    CHECK_GL_ERROR();
}

void Batch::_glUseProgram(GLuint program) {
    ADD_COMMAND_GL(glUseProgram);

    _params.push_back(program);

    DO_IT_NOW(_glUseProgram, 1);
}
void GLBackend::do_glUseProgram(Batch& batch, uint32 paramOffset) {
    
    _pipeline._program = batch._params[paramOffset]._uint;
    // for this call we still want to execute the glUseProgram in the order of the glCOmmand to avoid any issue
    _pipeline._invalidProgram = false;
    glUseProgram(_pipeline._program);

    CHECK_GL_ERROR();
}

void Batch::_glUniform1f(GLint location, GLfloat v0) {
    ADD_COMMAND_GL(glUniform1f);

    _params.push_back(v0);
    _params.push_back(location);

    DO_IT_NOW(_glUniform1f, 1);
}
void GLBackend::do_glUniform1f(Batch& batch, uint32 paramOffset) {
    glUniform1f(
        batch._params[paramOffset + 1]._int,
        batch._params[paramOffset + 0]._float);
    CHECK_GL_ERROR();
}

void Batch::_glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
    ADD_COMMAND_GL(glUniform2f);

    _params.push_back(v1);
    _params.push_back(v0);
    _params.push_back(location);

    DO_IT_NOW(_glUniform2f, 1);
}
void GLBackend::do_glUniform2f(Batch& batch, uint32 paramOffset) {
    glUniform2f(
        batch._params[paramOffset + 2]._int,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    CHECK_GL_ERROR();
}

void Batch::_glUniform4fv(GLint location, GLsizei count, const GLfloat* value) {
    ADD_COMMAND_GL(glUniform4fv);

    const int VEC4_SIZE = 4 * sizeof(float);
    _params.push_back(cacheData(count * VEC4_SIZE, value));
    _params.push_back(count);
    _params.push_back(location);

    DO_IT_NOW(_glUniform4fv, 3);
}
void GLBackend::do_glUniform4fv(Batch& batch, uint32 paramOffset) {
    glUniform4fv(
        batch._params[paramOffset + 2]._int,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.editData(batch._params[paramOffset + 0]._uint));

    CHECK_GL_ERROR();
}

void Batch::_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    ADD_COMMAND_GL(glUniformMatrix4fv);

    const int MATRIX4_SIZE = 16 * sizeof(float);
    _params.push_back(cacheData(count * MATRIX4_SIZE, value));
    _params.push_back(transpose);
    _params.push_back(count);
    _params.push_back(location);

    DO_IT_NOW(_glUniformMatrix4fv, 4);
}
void GLBackend::do_glUniformMatrix4fv(Batch& batch, uint32 paramOffset) {
    glUniformMatrix4fv(
        batch._params[paramOffset + 3]._int,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.editData(batch._params[paramOffset + 0]._uint));
    CHECK_GL_ERROR();
}

void Batch::_glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    ADD_COMMAND_GL(glDrawArrays);

    _params.push_back(count);
    _params.push_back(first);
    _params.push_back(mode);

    DO_IT_NOW(_glDrawArrays, 3);
}
void GLBackend::do_glDrawArrays(Batch& batch, uint32 paramOffset) {
    glDrawArrays(
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch._params[paramOffset + 0]._int);
    CHECK_GL_ERROR();
}

void Batch::_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) {
    ADD_COMMAND_GL(glDrawRangeElements);

    _params.push_back(cacheResource(indices));
    _params.push_back(type);
    _params.push_back(count);
    _params.push_back(end);
    _params.push_back(start);
    _params.push_back(mode);

    DO_IT_NOW(_glDrawRangeElements, 6);
}
void GLBackend::do_glDrawRangeElements(Batch& batch, uint32 paramOffset) {
    glDrawRangeElements(
        batch._params[paramOffset + 5]._uint,
        batch._params[paramOffset + 4]._uint,
        batch._params[paramOffset + 3]._uint,
        batch._params[paramOffset + 2]._int,
        batch._params[paramOffset + 1]._uint,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}

void Batch::_glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    ADD_COMMAND_GL(glColorPointer);

    _params.push_back(cacheResource(pointer));
    _params.push_back(stride);
    _params.push_back(type);
    _params.push_back(size);

    DO_IT_NOW(_glColorPointer, 4);
}
void GLBackend::do_glColorPointer(Batch& batch, uint32 paramOffset) {
    glColorPointer(
        batch._params[paramOffset + 3]._int,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}

void Batch::_glNormalPointer(GLenum type, GLsizei stride, const void *pointer) {
    ADD_COMMAND_GL(glNormalPointer);

    _params.push_back(cacheResource(pointer));
    _params.push_back(stride);
    _params.push_back(type);

    DO_IT_NOW(_glNormalPointer, 3);
}
void GLBackend::do_glNormalPointer(Batch& batch, uint32 paramOffset) {
    glNormalPointer(
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}

void Batch::_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    ADD_COMMAND_GL(glTexCoordPointer);

    _params.push_back(cacheResource(pointer));
    _params.push_back(stride);
    _params.push_back(type);
    _params.push_back(size);

    DO_IT_NOW(_glTexCoordPointer, 4);
}
void GLBackend::do_glTexCoordPointer(Batch& batch, uint32 paramOffset) {
    glTexCoordPointer(
        batch._params[paramOffset + 3]._int,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}

void Batch::_glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    ADD_COMMAND_GL(glVertexPointer);

    _params.push_back(cacheResource(pointer));
    _params.push_back(stride);
    _params.push_back(type);
    _params.push_back(size);

    DO_IT_NOW(_glVertexPointer, 4);
}
void GLBackend::do_glVertexPointer(Batch& batch, uint32 paramOffset) {
    glVertexPointer(
        batch._params[paramOffset + 3]._int,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}


void Batch::_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
    ADD_COMMAND_GL(glVertexAttribPointer);

    _params.push_back(cacheResource(pointer));
    _params.push_back(stride);
    _params.push_back(normalized);
    _params.push_back(type);
    _params.push_back(size);
    _params.push_back(index);

    DO_IT_NOW(_glVertexAttribPointer, 6);
}
void GLBackend::do_glVertexAttribPointer(Batch& batch, uint32 paramOffset) {
    glVertexAttribPointer(
        batch._params[paramOffset + 5]._uint,
        batch._params[paramOffset + 4]._int,
        batch._params[paramOffset + 3]._uint,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._int,
        batch.editResource(batch._params[paramOffset + 0]._uint)->_pointer);
    CHECK_GL_ERROR();
}

void Batch::_glEnableVertexAttribArray(GLint location) {
    ADD_COMMAND_GL(glEnableVertexAttribArray);

    _params.push_back(location);

    DO_IT_NOW(_glEnableVertexAttribArray, 1);
}
void GLBackend::do_glEnableVertexAttribArray(Batch& batch, uint32 paramOffset) {
    glEnableVertexAttribArray(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glDisableVertexAttribArray(GLint location) {
    ADD_COMMAND_GL(glDisableVertexAttribArray);

    _params.push_back(location);

    DO_IT_NOW(_glDisableVertexAttribArray, 1);
}
void GLBackend::do_glDisableVertexAttribArray(Batch& batch, uint32 paramOffset) {
    glDisableVertexAttribArray(batch._params[paramOffset]._uint);
    CHECK_GL_ERROR();
}

void Batch::_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    ADD_COMMAND_GL(glColor4f);

    _params.push_back(alpha);
    _params.push_back(blue);
    _params.push_back(green);
    _params.push_back(red);

    DO_IT_NOW(_glColor4f, 4);
}
void GLBackend::do_glColor4f(Batch& batch, uint32 paramOffset) {
    glColor4f(
        batch._params[paramOffset + 3]._float,
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    CHECK_GL_ERROR();
}

