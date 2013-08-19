#include <cstdlib>
#include <cstring>
#include <cassert>

#include <string>
#include <iostream>

#include <sys/timeb.h>

#define TIME_IT(name, expr)         \
    std::cout << name << "... ";    \
    start = now();                  \
    for(size_t i = 0; i < BIG; ++i) \
    {                               \
        expr;                       \
    }                               \
    std::cout << now() - start << "ms.\n";

static const uint8_t SSO_SIZE = 11;

inline uint32_t mangle(const char* const str)
{
    uint8_t l = strnlen(str, 4);
    uint32_t m = 0;
    for(uint8_t i = 1; i <= l; ++i)
        m |= str[i-1] << (32 - 8 * i);
    return m;
}

inline uint8_t unmangle(char* buffer, uint32_t mangled)
{
    uint8_t i = 0;
    for( ; i <= 4; ++i)
    {
        uint8_t x = 32 - 8 * (i - 1);
        char c = (mangled & (0xFF << x)) >> x;
        if(c == '\0') break;
        buffer[i] = c;
    }
    
    return i;
}

class CString
{
public:
    CString() : 
        m_SSO(), m_string(nullptr), m_start(nullptr), m_end(nullptr),
        m_sso_len(0), m_size(0), m_length(0) {}

    CString(const char* string) : 
        m_SSO(), m_string(nullptr), m_start(nullptr), m_end(nullptr),
        m_sso_len(0), m_size(0), m_length(0)
    {
        // Calculate length and copy to SSO buffer, if possible.
        // If we exceed the size, leave junk in buffer, but indicate
        // that it's free, and continue.
        size_t len = 0;
        for( ; string[len] != '\0'; ++len)
        {
            if(len < SSO_SIZE)
            {
                m_SSO[len] = string[len];
                m_sso_len  = len;
            }
            else
            {
                m_sso_len = 0;
                break;
            }
        }
        
        // If the SSO was too small, we need to copy it to a heap buffer.
        if(m_sso_len == 0)
        {
            // Finish getting the length;
            for( ; string[len] != '\0'; ++len);
            
            m_length = len;
            m_size = m_length + 10;
            m_string = new char[m_size];
            memcpy(m_string, string, m_length * sizeof(char));
            m_start = m_string;
            m_end = m_string + m_length;
        }
        
        // We had enough space in the SSO, so set up the pointers accordingly.
        else
        {
            m_start = m_SSO;
            m_end = m_SSO + m_sso_len;
        }
        
        *(m_start + len) = '\0';
    }

    CString(const std::string& string) : 
        m_SSO(), m_string(nullptr), m_start(nullptr), m_end(nullptr),
        m_sso_len(0), m_size(0), m_length(0)
    {
        // Largely identical to the char* version.
        
        size_t len = string.length();
        if(len < SSO_SIZE)
        {
            for(uint8_t i = 0; i < len; ++i)
                m_SSO[i] = string[i];
            m_sso_len = len;
            m_start = m_SSO;
            m_end = m_SSO + len;
        }
        else
        {
            m_length = len;
            m_size = m_length + 10;
            m_string = new char[m_size];
            memcpy(m_string, string.c_str(), m_length * sizeof(char));
            m_start = m_string;
            m_end = m_string + m_length;
        }
    }
    
    CString(const CString& Other) : 
        m_SSO(), m_string(nullptr), m_start(nullptr), m_end(nullptr),
        m_sso_len(0), m_size(0), m_length(0)
    {
        if(Other.m_sso_len)
        {
            for(size_t i = 0; i < Other.m_sso_len; ++i)
                m_SSO[i] = Other.m_SSO[i];
            m_sso_len = Other.m_sso_len;
        }
        else
        {
            m_length = Other.m_length;
            m_size = m_length + 10;
            m_string = new char[m_size];
            memcpy(m_string, Other.m_string, m_length * sizeof(char));
            m_start = m_string;
            m_end = m_string + m_length;
        }
    }
    
    ~CString()
    {
        this->Clear();
    }
    
    CString& operator=(const CString& Other) = delete;
    CString& operator=(const std::string& Other);
    CString& operator=(const char* string)
    {
        // Check to see if we are assigning to ourselves.
        size_t len = 0;
        if(m_start == nullptr) goto diff;
        for( ; string[len] != '\0'; ++len)
        {
            if(m_start[len] != string[len])
            {
                goto diff;
            }
        }
        
        m_end = m_start + len;
        return *this;

    diff:
        size_t real_len = len;
        for( ; string[real_len] != '\0'; ++real_len);

        // Fits in the SSO.
        if(real_len < SSO_SIZE)
        {
            // We rely on "len" here because the previous loop
            // left "len" at the index of the first incompatible
            // character pointed to by "m_start".
            // Thus, if we are using SSO, m_start points to the SSO
            // buffer anyway, and we can start copying to the
            // SSO directly after the "len" index, since all previous
            // ones are already the same.
            for( ; len < real_len; ++len)
            {
                m_SSO[len] = string[len];
            }
            
            m_start = m_SSO;
            m_end = m_SSO + len;
            m_sso_len = len;
            return *this;
        }
        
        // No SSO, but no re-alloc needed.
        else if(real_len < m_size)
        {
            for( ; len < real_len; ++len)
            {
                m_string[len] = string[len];
            }
            
            m_start = m_string;
            m_end = m_string + len;
            m_sso_len = 0;
            return *this;
        }
        
        // We need to re-allocate completely.
        delete[] m_string;
        m_sso_len = 0;
        m_length = real_len;
        m_size = m_length + 10;
        m_string = new char[m_size];
        memcpy(m_string, string, m_length * sizeof(char));
        m_start = m_string;
        m_end = m_string + m_length;
        return *this;
    }

    CString& operator+=(const CString& Other)
    {
        return this->Append(Other);
    }
    
    CString& Append(const CString& Other)
    {
        size_t ol = Other.GetLength();  // Other length
        size_t ul = this->GetLength();  // Us length
        
        ul *= sizeof(char);
        ol *= sizeof(char);
        
        if(ul + ol >= m_size || m_sso_len)
        {   
            // Copy old contents to new, larger buffer.
            char* tmp = new char[m_length + ol];
            memcpy(tmp, m_start, ul);
            delete[] m_string;
            
            // Copy new contents to end of new buffer, after
            // old contents.
            memcpy(tmp + ul, Other.m_start, ol);
            
            // Set up iterators.
            m_start = m_string = tmp;
            m_end = m_start + (ol + ul);
            m_sso_len = 0;
        }
        else
        {
            memcpy(m_string + ul, Other.m_start, ol);
            m_end = m_start + (ul + ol);
        }
        
        return *this;
    }
    
    void push_back(const char c)
    {
        if(m_sso_len)
        {
            if(m_sso_len < SSO_SIZE - 1)
                m_SSO[m_sso_len++] = c;
            else
            {
                m_length = m_sso_len + 1;
                m_size = m_length + 10;
                m_string = new char[m_size];
                memcpy(m_string, m_SSO, m_sso_len * sizeof(char));
                m_string[m_sso_len] = c;
                m_sso_len = 0;
                m_start = m_string;
                m_end = m_start + m_length;
            }
        }
        else if(m_length < m_size)
        {
            m_string[m_length++] = c;
        }
        else
        {
            m_size += 10;
            char* tmp = new char[m_size];
            memcpy(tmp, m_string, m_length * sizeof(char));
            delete[] m_string;
            tmp[m_length++] = c;
            m_start = m_string = tmp;
            m_end = m_string + m_length * sizeof(char);
        }
    }
    
    char operator[](const size_t index) const
    {
        return m_start[index];
    }
    
    operator std::string() const { return std::string(m_start, m_end); }
    operator char*() const       { return m_start; }
    
    char* begin() { return m_start; }
    char* end()   { return m_end;   }
    char  front() const { return *m_start; }
    char  back()  const { return *m_end;   }
    
    const char* c_str() const
    {
        if(m_start == nullptr) return "";
        *(m_end) = '\0';
        return m_start;
    }

    void Resize(const size_t size)
    {
        // If we are using the SSO buffer and the user
        // wants to expand past it, we need to copy SSO
        // contents to the heap buffer and use that instead.
        if(size >= SSO_SIZE && m_sso_len)
        {
            if(m_size < size)
            {
                char* tmp = new char[m_size = size];
                memcpy(tmp, m_SSO, m_sso_len * sizeof(char));
                delete[] m_string;
                m_string = tmp;
            }
            else
            {
                for(size_t i = 0; i < m_sso_len; ++i)
                    m_string[i] = m_SSO[i];
            }
            
            m_sso_len = 0;
            m_length = m_sso_len;
            m_start = m_string;
            m_end = m_string + m_length;
        }
        
        m_end = m_start + size;
    }
    
    void Reserve(const size_t size)
    {
        if(size < SSO_SIZE || m_size > size) return;
        
        m_size = size + 1;
        char* contents = new char[m_size];
        memcpy(contents, m_string, m_length * sizeof(char));
        delete[] m_string;
        m_string = contents;
        if(!m_sso_len)
        {
            m_start = m_string;
            m_end = m_start + m_length;
        }
    }
    
    void Clear()
    {
        m_length = m_sso_len = 0;
        m_start = m_end = nullptr;
    }
    
    void Shrink()
    {
        if(m_sso_len)
        {
            delete[] m_string;
            m_string = nullptr;
        }
        else if(m_size != m_length)
        {            
            char* tmp = new char[m_length + 1];
            memcpy(tmp, m_string, m_length * sizeof(char));
            delete[] m_string;
            m_start = m_string = tmp;
            m_end   = m_start + m_length;
            m_size  = m_length;
        }
    }
    
    inline size_t GetLength() const
    { return m_sso_len > 0 ? m_sso_len : m_length; }
    
    inline bool Empty() const
    { return this->GetLength() == 0; }
    
    friend std::ostream& operator<<(std::ostream& o, const CString& str)
    {
        return o << str.c_str();
    }
    
private:
    char    m_SSO[SSO_SIZE];
    char*   m_string;
    char*   m_start;
    char*   m_end;
    
    uint8_t m_sso_len;
    size_t  m_size, m_length;
};

typedef unsigned long long uint64_t;
typedef CString string_t;

uint64_t now()
{
    timeb t;
    ftime(&t);
    return t.millitm + t.time * 1000;
}

int main()
{
    static_assert(sizeof(char) == 1);
    srand(time(nullptr));
    
    const unsigned int BIG = 4000000;
    const char* STR1 = " this is a string ";
    const char* STR2 = "abcdefghijklmnopqrstuvwxyz";
    uint64_t start = 0;
    
    std::cout << "Starting std::string vs. zen::string_t comparison.\n";
    
    std::string tmp1(STR1), tmp4(STR2);
    string_t tmp2(STR1), tmp3(STR2);
    
    TIME_IT("Me  -- noop", string_t a());
    TIME_IT("STL -- noop", std::string a());
    
    TIME_IT("Me  -- ctor SSO", string_t a("short"));
    TIME_IT("STL -- ctor SSO", std::string a("short"));
    /*
    TIME_IT("Me  -- ctor", string_t a(STR1));
    TIME_IT("STL -- ctor", std::string a(STR1));
    
    TIME_IT("Me  -- STL ctor", string_t a(tmp1));
    TIME_IT("STL -- STL ctor", std::string a(tmp1));
    
    TIME_IT("Me  -- copy ctor", string_t a(tmp2));
    TIME_IT("STL -- copy ctor", std::string a(tmp1));
    
    TIME_IT("Me  -- op= same", tmp2 = STR1);
    TIME_IT("STL -- op= same", tmp1 = STR1);
    
    TIME_IT("Me  -- op[]", tmp2[rand() % tmp2.GetLength() - 1]);
    TIME_IT("STL -- op[]", tmp1[rand() % tmp1.length() - 1]);
    
    TIME_IT("Me  -- reserve", tmp2.Reserve(300); tmp2.Clear());
    TIME_IT("STL -- reserve", tmp1.reserve(300); tmp1.clear());
    
    TIME_IT("Me  -- iter", for(auto& i : tmp2) i = 'A';);
    TIME_IT("STL -- iter", for(auto& i : tmp1) i = 'A';);
    
    TIME_IT("Me  -- shrink", tmp2.Shrink());
    TIME_IT("STL -- shrink", tmp1.shrink_to_fit());
    
    TIME_IT("Me  -- op= diff", tmp2 = STR2; tmp2 = STR1);
    TIME_IT("STL -- op= diff", tmp1 = STR2; tmp1 = STR1);
    
    TIME_IT("Me  -- size", tmp2.GetLength());
    TIME_IT("STL -- size", tmp1.length());
    
    printf("Adding '%s' to '%s'.\n", tmp3.c_str(), tmp2.c_str());
    TIME_IT("Me  -- op+=", tmp2 += tmp3; tmp2.Clear(); tmp2.Shrink());
    TIME_IT("STL -- op+=", tmp1 += tmp4; tmp1.clear(); tmp1.shrink_to_fit());
    */
    
    tmp2 = STR1; tmp2 += tmp3;
    tmp1 = STR1; tmp1 += tmp4;
    
    TIME_IT("Me  -- push", tmp2.push_back('A'); if(i % 100 == 0) tmp2.Clear());
    TIME_IT("STL -- push", tmp1.push_back('A'); if(i % 100 == 0) tmp1.clear());
    
    printf("'%s' =?= '%s'\n", tmp1.c_str(), tmp2.c_str());
    std::cout << "sizeof(std::string) == " << sizeof(std::string) << std::endl
              << "sizeof(string_t)    == " << sizeof(string_t)    << std::endl;
    return 0;
}
