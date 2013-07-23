template<typename T>
obj::CEntity& CScene::AddPrimitive()
{
    obj::CEntity& Ref = this->AddEntity();
    T* pNew = new T(m_Assets);
    Ref.AddPrimitive(pNew->Create());
    delete pNew;
    return Ref;
}
