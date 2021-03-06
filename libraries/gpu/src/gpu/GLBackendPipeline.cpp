//
//  GLBackendPipeline.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 3/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLBackendShared.h"

#include "Format.h"

using namespace gpu;

GLBackend::GLPipeline::GLPipeline() :
    _program(nullptr)
{}

GLBackend::GLPipeline::~GLPipeline() {
    _program = nullptr;
}

GLBackend::GLPipeline* GLBackend::syncGPUObject(const Pipeline& pipeline) {
    GLPipeline* object = Backend::getGPUObject<GLBackend::GLPipeline>(pipeline);

    // If GPU object already created then good
    if (object) {
        return object;
    }

    return nullptr;
}

void GLBackend::do_setPipeline(Batch& batch, uint32 paramOffset) {
    PipelinePointer pipeline = batch._pipelines.get(batch._params[paramOffset + 0]._uint);

    if (pipeline == _pipeline._pipeline) {
        return;
    }
   
    auto pipelineObject = syncGPUObject((*pipeline));
    if (!pipelineObject) {
        return;
    }
    
    _pipeline._pipeline = pipeline;
    _pipeline._program = pipelineObject->_program->_program;
    _pipeline._invalidProgram = true;
}

void GLBackend::do_setUniformBuffer(Batch& batch, uint32 paramOffset) {
    GLuint slot = batch._params[paramOffset + 3]._uint;
    BufferPointer uniformBuffer = batch._buffers.get(batch._params[paramOffset + 2]._uint);
    GLintptr rangeStart = batch._params[paramOffset + 1]._uint;
    GLsizeiptr rangeSize = batch._params[paramOffset + 0]._uint;

#if (GPU_FEATURE_PROFILE == GPU_CORE)
    GLuint bo = getBufferID(*uniformBuffer);
    glBindBufferRange(GL_UNIFORM_BUFFER, slot, bo, rangeStart, rangeSize);
#else
    GLfloat* data = (GLfloat*) (uniformBuffer->getData() + rangeStart);
    glUniform4fv(slot, rangeSize / sizeof(GLfloat[4]), data);
 
    // NOT working so we ll stick to the uniform float array until we move to core profile
    // GLuint bo = getBufferID(*uniformBuffer);
    //glUniformBufferEXT(_shader._program, slot, bo);
#endif
    CHECK_GL_ERROR();
}

void GLBackend::do_setUniformTexture(Batch& batch, uint32 paramOffset) {
    GLuint slot = batch._params[paramOffset + 1]._uint;
    TexturePointer uniformTexture = batch._textures.get(batch._params[paramOffset + 0]._uint);

    GLuint to = getTextureID(uniformTexture);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, to);

    CHECK_GL_ERROR();
}


void GLBackend::updatePipeline() {
    if (_pipeline._invalidProgram) {
        glUseProgram(_pipeline._program);
        CHECK_GL_ERROR();

        _pipeline._invalidProgram = true;
    }
}

