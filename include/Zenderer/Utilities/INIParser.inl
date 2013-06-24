inline int CINIParser::GetValuei(const string_t& key) const
{
    return atoi(this->GetValue(key).c_str());
}

inline bool CINIParser::GetValueb(const string_t& key) const
{
    return (toupper(key) == "TRUE" ||
            this->GetValuei(key) != 0);
}

inline float CINIParser::GetValuef(const string_t& key) const
{
    return atof(this->GetValue(key).c_str());
}
