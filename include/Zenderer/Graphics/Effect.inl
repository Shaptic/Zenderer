bool CEffect::Enable() const
{
    return this->Bind();
}

bool CEffect::Disable() const
{
    return this->Unbind();
}

const string_t& CEffect::GetError() const
{
    return m_Shader.GetError();
}

void CEffect::SetType(const EffectType Type)
{
    m_type = Type;
}

bool CEffect::Bind() const
{
    return m_Shader.Bind();
}

bool CEffect::Unbind() const
{
    return m_Shader.Unbind();
}
