//
//  Pipeline.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 3/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Pipeline.h"
#include <math.h>
#include <QDebug>

using namespace gpu;

Pipeline::Pipeline():
    _program(),
    _states()
{
}

Pipeline::~Pipeline()
{
}

Pipeline* Pipeline::create(const ShaderPointer& program, const States& states) {
    Pipeline* pipeline = new Pipeline();
    pipeline->_program = program;
    pipeline->_states = states;

    return pipeline;
}
