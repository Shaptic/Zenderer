inline int zParser::GetValuei(const string_t& key) const
{
    return std::stoi(this->GetValue(key));
}

inline bool zParser::GetValueb(const string_t& key) const
{
    return (toupper(key) == "TRUE" ||
            this->GetValuei(key) != 0);
}

inline real_t zParser::GetValuer(const string_t& key) const
{
    return std::stod(this->GetValue(key));
}
