#pragma once
#ifndef WSTRING_H
#define WSTRING_H

#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cctype>

class String {
public:
    String() {}
    String(const char *s)         : s_(s ? s : "") {}
    String(const std::string &s)  : s_(s) {}
    String(char c)                : s_(1, c) {}

    explicit String(int n, int base = 10)          { char b[32]; if (base==16) snprintf(b,32,"%x",n); else snprintf(b,32,"%d",n); s_=b; }
    explicit String(unsigned int n, int base = 10) { char b[32]; snprintf(b,32,"%u",n); s_=b; }
    explicit String(long n, int base = 10)         { char b[32]; if (base==16) snprintf(b,32,"%lx",n); else snprintf(b,32,"%ld",n); s_=b; }
    explicit String(unsigned long n, int base = 10){ char b[32]; snprintf(b,32,"%lu",n); s_=b; }
    explicit String(float n, int dec = 2)          { char fmt[8],b[32]; snprintf(fmt,8,"%%.%df",dec); snprintf(b,32,fmt,(double)n); s_=b; }
    explicit String(double n, int dec = 2)         { char fmt[8],b[32]; snprintf(fmt,8,"%%.%df",dec); snprintf(b,32,fmt,n); s_=b; }

    // Assignment
    String &operator=(const String &o)  { s_=o.s_; return *this; }
    String &operator=(const char *s)    { s_=s?s:""; return *this; }
    String &operator=(char c)           { s_=std::string(1,c); return *this; }

    // Concatenation
    String &operator+=(const String &o)     { s_+=o.s_; return *this; }
    String &operator+=(const char *s)       { if(s) s_+=s; return *this; }
    String &operator+=(char c)              { s_+=c; return *this; }
    String &operator+=(int n)               { return *this += String(n); }
    String &operator+=(unsigned int n)      { return *this += String(n); }
    String &operator+=(long n)              { return *this += String(n); }
    String &operator+=(unsigned long n)     { return *this += String(n); }
    String &operator+=(float n)             { return *this += String(n); }
    String &operator+=(double n)            { return *this += String(n); }

    String operator+(const String &o) const { return String(s_+o.s_); }
    String operator+(const char *s)   const { return String(s_+(s?s:"")); }
    String operator+(char c)          const { return String(s_+c); }

    // Comparison
    bool operator==(const String &o) const { return s_==o.s_; }
    bool operator==(const char *s)   const { return s_==(s?s:""); }
    bool operator!=(const String &o) const { return s_!=o.s_; }
    bool operator!=(const char *s)   const { return s_!=(s?s:""); }
    bool operator< (const String &o) const { return s_< o.s_; }
    bool operator> (const String &o) const { return s_> o.s_; }
    bool operator<=(const String &o) const { return s_<=o.s_; }
    bool operator>=(const String &o) const { return s_>=o.s_; }

    // Access
    char   operator[](unsigned int i) const { return i<s_.size() ? s_[i] : 0; }
    char  &operator[](unsigned int i)       { return s_[i]; }
    char   charAt(unsigned int i)     const { return (*this)[i]; }
    void   setCharAt(unsigned int i, char c){ if(i<s_.size()) s_[i]=c; }

    // Properties
    unsigned int length()   const { return (unsigned int)s_.size(); }
    bool         isEmpty()  const { return s_.empty(); }
    const char  *c_str()    const { return s_.c_str(); }
    operator const char *() const { return s_.c_str(); }
    explicit operator bool() const { return !s_.empty(); }

    // Search
    int indexOf(char c, unsigned int from=0) const {
        auto p=s_.find(c,from); return p==std::string::npos ? -1 : (int)p;
    }
    int indexOf(const String &str, unsigned int from=0) const {
        auto p=s_.find(str.s_,from); return p==std::string::npos ? -1 : (int)p;
    }
    int lastIndexOf(char c) const {
        auto p=s_.rfind(c); return p==std::string::npos ? -1 : (int)p;
    }
    int lastIndexOf(const String &str) const {
        auto p=s_.rfind(str.s_); return p==std::string::npos ? -1 : (int)p;
    }
    bool startsWith(const String &p) const {
        return s_.size()>=p.s_.size() && s_.compare(0,p.s_.size(),p.s_)==0;
    }
    bool endsWith(const String &s) const {
        return s_.size()>=s.s_.size() && s_.compare(s_.size()-s.s_.size(),s.s_.size(),s.s_)==0;
    }

    // Extraction
    String substring(unsigned int from, unsigned int to) const {
        if(from>=s_.size()) return String("");
        if(to>s_.size()) to=(unsigned int)s_.size();
        return String(s_.substr(from,to-from));
    }
    String substring(unsigned int from) const {
        return substring(from,(unsigned int)s_.size());
    }

    // Modification
    void toLowerCase()  { for(char &c:s_) c=(char)tolower((unsigned char)c); }
    void toUpperCase()  { for(char &c:s_) c=(char)toupper((unsigned char)c); }
    void trim() {
        size_t a=s_.find_first_not_of(" \t\r\n");
        size_t b=s_.find_last_not_of(" \t\r\n");
        s_=(a==std::string::npos)?"":s_.substr(a,b-a+1);
    }
    void remove(unsigned int idx)              { if(idx<s_.size()) s_.erase(idx); }
    void remove(unsigned int idx, unsigned int n){ if(idx<s_.size()) s_.erase(idx,n); }
    void replace(const String &from, const String &to) {
        size_t p=0;
        while((p=s_.find(from.s_,p))!=std::string::npos){ s_.replace(p,from.s_.size(),to.s_); p+=to.s_.size(); }
    }
    bool concat(const String &s)  { s_+=s.s_; return true; }
    bool concat(const char *s)    { if(s) s_+=s; return true; }
    bool reserve(unsigned int n)  { s_.reserve(n); return true; }

    // Conversion
    long          toInt()    const { return std::strtol(s_.c_str(),nullptr,10); }
    float         toFloat()  const { return (float)std::strtod(s_.c_str(),nullptr); }
    double        toDouble() const { return std::strtod(s_.c_str(),nullptr); }

    // Misc
    int  compareTo(const String &s) const { return s_.compare(s.s_); }
    bool equalsIgnoreCase(const String &s) const {
        if(s_.size()!=s.s_.size()) return false;
        for(size_t i=0;i<s_.size();i++)
            if(tolower((unsigned char)s_[i])!=tolower((unsigned char)s.s_[i])) return false;
        return true;
    }
    bool equals(const String &s) const { return s_==s.s_; }
    void toCharArray(char *buf, unsigned int sz, unsigned int idx=0) const {
        if(!sz) return;
        unsigned int n=0;
        for(unsigned int i=idx; i<s_.size()&&n<sz-1; i++) buf[n++]=(char)s_[i];
        buf[n]=0;
    }
    void getBytes(unsigned char *buf, unsigned int sz, unsigned int idx=0) const {
        toCharArray((char*)buf, sz, idx);
    }

private:
    std::string s_;
};

inline String operator+(const char *lhs, const String &rhs) { return String(lhs)+rhs; }
inline String operator+(char lhs,        const String &rhs) { return String(lhs)+rhs; }

#endif // WSTRING_H
