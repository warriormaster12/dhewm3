#!/bin/bash
glslc simple_triangle.glsl -fshader-stage=vertex -DSTAGE_VERTEX -o simple_triangle.vert.spv
glslc simple_triangle.glsl -fshader-stage=fragment -DSTAGE_FRAGMENT -o simple_triangle.frag.spv

