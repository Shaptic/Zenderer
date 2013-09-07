bool zEffect::Enable() const
{
    return this->Bind();
}

bool zEffect::Disable() const
{
    return this->Unbind();
}

const string_t& zEffect::GetError() const
{
    return m_Shader.GetError();
}

void zEffect::SetType(const EffectType Type)
{
    m_type = Type;
}

GLuint zEffect::GetObjectHandle() const
{
    return m_Shader.GetShaderObject();
}

bool zEffect::Bind() const
{
    return m_Shader.Bind();
}

bool zEffect::Unbind() const
{
    return m_Shader.Unbind();
}
