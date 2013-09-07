template<typename T>
obj::zEntity& zScene::AddPrimitive()
{
    obj::zEntity& Ref = this->AddEntity();
    T* pNew = new T(m_Assets);
    Ref.AddPrimitive(pNew->Create());
    delete pNew;
    return Ref;
}
