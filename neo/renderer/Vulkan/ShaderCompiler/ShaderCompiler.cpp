#include "ShaderCompiler.h"

#include <iostream>
#include <fstream>
#include <sstream>


#ifdef NDEBUG
	const bool forceShaderRecompile = false;
#else 
	const bool forceShaderRecompile = true;
#endif

bool glslangInitialized = false;

class idShaderStageBuilderLocal : public idShaderStageBuilder {
private:
    bool CompileToSPV(const std::string& path);
};
idShaderStageBuilderLocal shaderbuilderLocal;
idShaderStageBuilder* shaderBuilder = &shaderbuilderLocal;

  
TBuiltInResource HandleResources()
{
    TBuiltInResource DefaultTBuiltInResource {};
    DefaultTBuiltInResource.maxLights =  32;
    DefaultTBuiltInResource.maxClipPlanes =  6;
    DefaultTBuiltInResource.maxTextureUnits =  32;
    DefaultTBuiltInResource.maxTextureCoords =  32;
    DefaultTBuiltInResource.maxVertexAttribs =  64;
    DefaultTBuiltInResource.maxVertexUniformComponents =  4096;
    DefaultTBuiltInResource.maxVaryingFloats =  64;
    DefaultTBuiltInResource.maxVertexTextureImageUnits =  32;
    DefaultTBuiltInResource.maxCombinedTextureImageUnits =  80;
    DefaultTBuiltInResource.maxTextureImageUnits =  32;
    DefaultTBuiltInResource.maxFragmentUniformComponents =  4096;
    DefaultTBuiltInResource.maxDrawBuffers =  32;
    DefaultTBuiltInResource.maxVertexUniformVectors =  128;
    DefaultTBuiltInResource.maxVaryingVectors =  8;
    DefaultTBuiltInResource.maxFragmentUniformVectors =  16;
    DefaultTBuiltInResource.maxVertexOutputVectors =  16;
    DefaultTBuiltInResource.maxFragmentInputVectors =  15;
    DefaultTBuiltInResource.minProgramTexelOffset =  -8;
    DefaultTBuiltInResource.maxProgramTexelOffset =  7;
    DefaultTBuiltInResource.maxClipDistances =  8;
    DefaultTBuiltInResource.maxComputeWorkGroupCountX =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupCountY =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupCountZ =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeX =  1024;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeY =  1024;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeZ =  64;
    DefaultTBuiltInResource.maxComputeUniformComponents =  1024;
    DefaultTBuiltInResource.maxComputeTextureImageUnits =  16;
    DefaultTBuiltInResource.maxComputeImageUniforms =  8;
    DefaultTBuiltInResource.maxComputeAtomicCounters =  8;
    DefaultTBuiltInResource.maxComputeAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxVaryingComponents =  60;
    DefaultTBuiltInResource.maxVertexOutputComponents =  64;
    DefaultTBuiltInResource.maxGeometryInputComponents =  64;
    DefaultTBuiltInResource.maxGeometryOutputComponents =  128;
    DefaultTBuiltInResource.maxFragmentInputComponents =  128;
    DefaultTBuiltInResource.maxImageUnits =  8;
    DefaultTBuiltInResource.maxCombinedImageUnitsAndFragmentOutputs =  8;
    DefaultTBuiltInResource.maxCombinedShaderOutputResources =  8;
    DefaultTBuiltInResource.maxImageSamples =  0;
    DefaultTBuiltInResource.maxVertexImageUniforms =  0;
    DefaultTBuiltInResource.maxTessControlImageUniforms =  0;
    DefaultTBuiltInResource.maxTessEvaluationImageUniforms =  0;
    DefaultTBuiltInResource.maxGeometryImageUniforms =  0;
    DefaultTBuiltInResource.maxFragmentImageUniforms =  8;
    DefaultTBuiltInResource.maxCombinedImageUniforms =  8;
    DefaultTBuiltInResource.maxGeometryTextureImageUnits =  16;
    DefaultTBuiltInResource.maxGeometryOutputVertices =  256;
    DefaultTBuiltInResource.maxGeometryTotalOutputComponents =  1024;
    DefaultTBuiltInResource.maxGeometryUniformComponents =  1024;
    DefaultTBuiltInResource.maxGeometryVaryingComponents =  64;
    DefaultTBuiltInResource.maxTessControlInputComponents =  128;
    DefaultTBuiltInResource.maxTessControlOutputComponents =  128;
    DefaultTBuiltInResource.maxTessControlTextureImageUnits =  16;
    DefaultTBuiltInResource.maxTessControlUniformComponents =  1024;
    DefaultTBuiltInResource.maxTessControlTotalOutputComponents =  4096;
    DefaultTBuiltInResource.maxTessEvaluationInputComponents =  128;
    DefaultTBuiltInResource.maxTessEvaluationOutputComponents =  128;
    DefaultTBuiltInResource.maxTessEvaluationTextureImageUnits =  16;
    DefaultTBuiltInResource.maxTessEvaluationUniformComponents =  1024;
    DefaultTBuiltInResource.maxTessPatchComponents =  120;
    DefaultTBuiltInResource.maxPatchVertices =  32;
    DefaultTBuiltInResource.maxTessGenLevel =  64;
    DefaultTBuiltInResource.maxViewports =  16;
    DefaultTBuiltInResource.maxVertexAtomicCounters =  0;
    DefaultTBuiltInResource.maxTessControlAtomicCounters =  0;
    DefaultTBuiltInResource.maxTessEvaluationAtomicCounters =  0;
    DefaultTBuiltInResource.maxGeometryAtomicCounters =  0;
    DefaultTBuiltInResource.maxFragmentAtomicCounters =  8;
    DefaultTBuiltInResource.maxCombinedAtomicCounters =  8;
    DefaultTBuiltInResource.maxAtomicCounterBindings =  1;
    DefaultTBuiltInResource.maxVertexAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxTessControlAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxTessEvaluationAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxGeometryAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxFragmentAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxCombinedAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxAtomicCounterBufferSize =  16384;
    DefaultTBuiltInResource.maxTransformFeedbackBuffers =  4;
    DefaultTBuiltInResource.maxTransformFeedbackInterleavedComponents =  64;
    DefaultTBuiltInResource.maxCullDistances =  8;
    DefaultTBuiltInResource.maxCombinedClipAndCullDistances =  8;
    DefaultTBuiltInResource.maxSamples =  4;
    DefaultTBuiltInResource.maxMeshOutputVerticesNV =  256;
    DefaultTBuiltInResource.maxMeshOutputPrimitivesNV =  512;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeX_NV =  32;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeY_NV=  1;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeZ_NV=  1;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeX_NV=  32;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeY_NV=  1;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeZ_NV=  1;
    DefaultTBuiltInResource.maxMeshViewCountNV=  4;
    DefaultTBuiltInResource.limits.nonInductiveForLoops =  1;
    DefaultTBuiltInResource.limits.whileLoops =  1;
    DefaultTBuiltInResource.limits.doWhileLoops =  1;
    DefaultTBuiltInResource.limits.generalUniformIndexing =  1;
    DefaultTBuiltInResource.limits.generalAttributeMatrixVectorIndexing =  1;
    DefaultTBuiltInResource.limits.generalVaryingIndexing =  1;
    DefaultTBuiltInResource.limits.generalSamplerIndexing =  1;
    DefaultTBuiltInResource.limits.generalVariableIndexing =  1;
    DefaultTBuiltInResource.limits.generalConstantMatrixVectorIndexing =  1;

    return DefaultTBuiltInResource;
}

bool idShaderStageBuilderLocal::CompileToSPV(const std::string& path) {
    std::string spvFileName = path + ".spv";
    //Check that spv file already exists
    std::ifstream fileSpv(spvFileName);
    if (fileSpv.fail() || forceShaderRecompile == true)
    {
        if (!glslangInitialized)
        {
            glslang::InitializeProcess();
            glslangInitialized = true;
        }
        //Load GLSL into a string
        std::ifstream fileGlsl(path);

        if (!fileGlsl.is_open()) 
        {
            common->Error("Failed to load shader: %s", path.data());
            abort();
        }

        std::string InputGLSL((std::istreambuf_iterator<char>(fileGlsl)),
                                std::istreambuf_iterator<char>());

        const char* InputCString = InputGLSL.c_str();

        EShLanguage ShaderType = GetShaderStage(GetSuffix(filename));
        glslang::TShader Shader(ShaderType);
        
        Shader.setStrings(&InputCString, 1);
        

        int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
        glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
        glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

        Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

        TBuiltInResource Resources;
        Resources = HandleResources();
        EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);

        const int DefaultVersion = 100;

        DirStackFileIncluder Includer;

        //Get Path of File
        std::string Path = GetFilePath(filename);
        Includer.pushExternalLocalDirectory(Path);

        std::string PreprocessedGLSL;

        if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer)) 
        {
            ENGINE_CORE_ERROR("GLSL Preprocessing Failed for: {0}", filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);
        if (!Shader.parse(&Resources, 100, false, messages))
        {
            ENGINE_CORE_ERROR("GLSL Parsing Failed for: {0}",filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        glslang::TProgram Program;
        Program.addShader(&Shader);

        if(!Program.link(messages))
        {
            ENGINE_CORE_ERROR("GLSL Linking Failed for: {0} ", filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        std::vector<unsigned int> SpirV;
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);
        glslang::OutputSpvBin(SpirV, SpirV_filename.c_str());  
        ENGINE_CORE_INFO("file " + filename + " compiled to " + SpirV_filename);
        return SpirV_filename;
}