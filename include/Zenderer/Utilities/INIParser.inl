inline int CINIParser::GetValuei(const string_t& key) const
{
    return std::stoi(this->GetValue(key));
}

inline bool CINIParser::GetValueb(const string_t& key) const
{
    return (toupper(key) == "TRUE" ||
            this->GetValuei(key) != 0);
}

inline real_t CINIParser::GetValuer(const string_t& key) const
{
    return std::stod(this->GetValue(key));
}
