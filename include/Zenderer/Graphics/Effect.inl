bool CEffect::Enable()
{
    return this->Bind();
}

bool CEffect::Disable()
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

bool CEffect::Bind()
{
    return m_Shader.Bind();
}

bool CEffect::Unbind()
{
    return m_Shader.Unbind();
}
