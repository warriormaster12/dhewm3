#!/bin/bash
glslc simple_triangle.glsl -DSTAGE_VERTEX -o simple_triangle.vert.spv
glslc simple_triangle.glsl -DSTAGE_FRAGMENT -o simple_triangle.frag.spv

