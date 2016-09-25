/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if USE(3D_GRAPHICS)

#include "ANGLEWebKitBridge.h"
#include <wtf/OwnArrayPtr.h>

namespace WebCore {

template<typename vectype>
bool readSymbols(const vectype *vec, Vector<ANGLEShaderSymbol> &symbols) {
    if(!vec) {
        return false;
    }
    for(typename vectype::const_iterator i = vec->begin(), end = vec->end(); i != end; ++i) {
        ANGLEShaderSymbol symbol;
        symbol.name = i->name.data();
        symbol.mappedName = i->mappedName.data();
        symbol.isArray = i->isArray();
        symbols.append(symbol);
        if (symbol.isArray) {
            // Add symbols for each array element.
            symbol.size = i->arraySize;
            symbol.isArray = false;
            for (int index = 0; index < symbol.size; index++) {
                String arrayBrackets = "[" + String::number(index) + "]";
                symbol.name = i->name.data() + arrayBrackets;
                symbol.mappedName = i->mappedName.data() + arrayBrackets;
                symbols.append(symbol);
            }
        }
    }
    return true;
}

ANGLEWebKitBridge::ANGLEWebKitBridge(ShShaderOutput shaderOutput, ShShaderSpec shaderSpec)
    : builtCompilers(false)
    , m_fragmentCompiler(0)
    , m_vertexCompiler(0)
    , m_shaderOutput(shaderOutput)
    , m_shaderSpec(shaderSpec)
{
    // This is a no-op if it's already initialized.
    ShInitialize();
}

ANGLEWebKitBridge::~ANGLEWebKitBridge()
{
    cleanupCompilers();
}

void ANGLEWebKitBridge::cleanupCompilers()
{
    if (m_fragmentCompiler)
        ShDestruct(m_fragmentCompiler);
    m_fragmentCompiler = 0;
    if (m_vertexCompiler)
        ShDestruct(m_vertexCompiler);
    m_vertexCompiler = 0;

    builtCompilers = false;
}
    
void ANGLEWebKitBridge::setResources(ShBuiltInResources resources)
{
    // Resources are (possibly) changing - cleanup compilers if we had them already
    cleanupCompilers();
    m_resources = resources;
}

bool ANGLEWebKitBridge::compileShaderSource(const char* shaderSource, ANGLEShaderType shaderType, String& translatedShaderSource, String& shaderValidationLog, Vector<ANGLEShaderSymbol>& symbols, int extraCompileOptions)
{
    if (!builtCompilers) {
        m_fragmentCompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, m_shaderSpec, m_shaderOutput, &m_resources);
        m_vertexCompiler = ShConstructCompiler(GL_VERTEX_SHADER, m_shaderSpec, m_shaderOutput, &m_resources);
        if (!m_fragmentCompiler || !m_vertexCompiler) {
            cleanupCompilers();
            return false;
        }

        builtCompilers = true;
    }
    
    ShHandle compiler;

    if (shaderType == SHADER_TYPE_VERTEX)
        compiler = m_vertexCompiler;
    else
        compiler = m_fragmentCompiler;

    const char* const shaderSourceStrings[] = { shaderSource };

    bool validateSuccess = ShCompile(compiler, shaderSourceStrings, 1, SH_OBJECT_CODE | SH_VARIABLES | extraCompileOptions);
    if (!validateSuccess) {
        shaderValidationLog = ShGetInfoLog(compiler).data();
        return false;
    }

    translatedShaderSource = ShGetObjectCode(compiler).data();
    
    if (!readSymbols(ShGetAttributes(compiler), symbols))
        return false;
    if (!readSymbols(ShGetUniforms(compiler), symbols))
        return false;

    return true;
}

}

#endif // USE(3D_GRAPHICS)
