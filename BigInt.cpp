//Filename: BigInt.cpp
//Date: 2015/10/6

//Author: luotuo44   http://blog.csdn.net/luotuo44

//Copyright 2015, luotuo44. All rights reserved.
//Use of this source code is governed by a BSD-style license


#include"BigInt.h"
#include<algorithm>
#include<exception>
#include<stdexcept>
#include<cctype>


#include<assert.h>

int BigInt::s_span = 4;
int BigInt::s_mask = 10000;

BigInt::BigInt(int val)noexcept
    : m_sign(val < 0)
{
    if( val == 0)
        m_vec.push_back(0);
    else
    {
        unsigned int v = val < 0 ? -val : val;

        while(v > 0)
        {
            m_vec.push_back(v % s_mask);
            v /= s_mask;
        }
    }
}


BigInt::BigInt(const std::string &str)
    : m_sign(false)
{
    std::string ss(str);
    if(!ss.empty() && (ss[0] == '-' || ss[0] == '+') )
    {
        m_sign = (ss[0] == '-');
        ss.erase(ss.begin());
    }

    if(ss.empty())
        throw std::logic_error("invalid string");

    m_vec.reserve(ss.size()/s_span);
    int i = static_cast<int>(ss.size());
    for( ; i >= 0; i -= s_span)
    {
        int val = 0;
        int len = i >= s_span ? s_span : i;
        for(int j = i - len; j < i; ++j)
        {
            if( !isdigit(ss[j]) )
                throw std::logic_error("invalid string");
            val = 10*val + ss[j] - '0';
        }

        m_vec.push_back(val);
    }

    //str = "0000000001"
    while(!m_vec.empty() && m_vec.back() == 0)
        m_vec.pop_back();

    //str = "0000000000000" or "-000000"
    if( m_vec.empty())
    {
        m_sign = false;
        m_vec.push_back(0);
    }
}


BigInt::BigInt(const BigInt &bi)
    : m_sign(bi.m_sign),
      m_vec(bi.m_vec)
{
}

BigInt& BigInt::operator = (const BigInt& bi)
{
    if( this != &bi)
    {
        std::vector<int> tmp = bi.m_vec;
        m_sign = bi.m_sign;
        m_vec.swap(tmp);
    }

    return *this;
}

BigInt::BigInt(BigInt &&bi)
{
    m_sign = bi.m_sign;
    m_vec = std::move(bi.m_vec);
}


BigInt& BigInt::operator = (BigInt &&bi)
{
    if(this != &bi)
    {
        m_vec = std::move(bi.m_vec);
        m_sign = bi.m_sign;
    }

    return *this;
}


void BigInt::swap(BigInt &bi)noexcept
{
    std::swap(m_sign, bi.m_sign);
    m_vec.swap(bi.m_vec);
}


std::string BigInt::toString()const
{
    std::string str;
    str.reserve(m_vec.size() * 4);

    if(m_sign)
        str.push_back('-');

    for(auto it = m_vec.rbegin(); it != m_vec.rend(); ++it)
    {
        std::string tmp = std::to_string(*it);
        if( tmp.size() < static_cast<size_t>(s_span) && it != m_vec.rbegin())
            str.insert(str.end(), s_span-tmp.size(), '0');

        str += tmp;
    }

    return str;
}


std::string BigInt::toBinString()const
{
    std::string str = toBinStringWithoutSign();
    if(m_sign)
        str.insert(str.begin(), '-');

    return str;
}

std::string BigInt::toBinStringWithoutSign()const
{
    if( *this == 0 )
        return "0";

    BigInt tmp(*this);
    std::string str;

    while( tmp != 0 )
    {
        if(tmp.m_vec[0] & 1)
            str.push_back('1');
        else
            str.push_back('0');

        BigInt::div2(tmp);
    }

    std::reverse(str.begin(), str.end());
    return str;
}



const BigInt BigInt::operator + (const BigInt &bi)const
{
    BigInt tmp(*this);
    tmp += bi;

    return tmp;
}


BigInt& BigInt::operator += (const BigInt &bi)
{
    NumVec vec;
    if( !(m_sign ^ bi.m_sign) )//same sign
    {
        vec = BigInt::vecAdd(m_vec, bi.m_vec);
    }
    else
    {
        if( absCompare(bi) >= 0 )
        {
            vec = BigInt::vecSub(m_vec, bi.m_vec);
        }
        else
        {
            vec = BigInt::vecSub(bi.m_vec, m_vec);
            m_sign = bi.m_sign;
        }

        if( vec.size()==1 && vec[0] == 0)
            m_sign = false;
    }

    m_vec = std::move(vec);
    return *this;
}

const BigInt BigInt::operator - (const BigInt &bi)const
{
    BigInt tmp(*this);
    tmp -= bi;

    return tmp;
}


BigInt& BigInt::operator -= (const BigInt &bi)
{
    NumVec vec;

    if( m_sign ^ bi.m_sign )
    {
        vec = BigInt::vecAdd(m_vec, bi.m_vec);
    }
    else
    {
        if( absCompare(bi) >= 0 )
        {
            vec = BigInt::vecSub(m_vec, bi.m_vec);
        }
        else
        {
            vec = BigInt::vecSub(bi.m_vec, m_vec);
            m_sign = !bi.m_sign;
        }

        if( vec.size()==1 && vec[0] == 0)
            m_sign = false;
    }

    m_vec = std::move(vec);

    return *this;
}


const BigInt BigInt::operator * (const BigInt &bi)const
{
    BigInt tmp(*this);
    tmp *= bi;

    return tmp;
}


BigInt& BigInt::operator *= (const BigInt &bi)
{
    NumVec vec(m_vec.size() + bi.m_vec.size()+2, 0);

    for(size_t i = 0; i < m_vec.size(); ++i)
    {
        if( m_vec[i] == 0) continue;

        for(size_t j = 0; j < bi.m_vec.size(); ++j)
        {
            if( bi.m_vec[j] == 0 ) continue;

            int val = vec[i+j] + m_vec[i] * bi.m_vec[j];
            vec[i+j] = val % s_mask;
            vec[i+j+1] += val / s_mask;//ret[i+j+1]超过s_mask也无所谓，因为接着会在下次循环中处理
        }
    }

    while(!vec.empty() && vec.back() == 0) vec.pop_back();

    if( vec.empty() )
        vec.push_back(0);

    m_vec = std::move(vec);
    if( m_vec.size() == 1 && m_vec[0] == 0 )
        m_sign = false;
    else
        m_sign ^= bi.m_sign;

    return *this;
}


const BigInt BigInt::operator / (const BigInt &bi)const
{
    BigInt tmp(*this);
    tmp /= bi;

    return tmp;
}


BigInt& BigInt::operator /= (const BigInt &bi)
{
    if( bi == 0 )
        throw std::logic_error("can't div 0");

    bool sign = m_sign;
    m_sign = false;

    BigInt res(0);

    BigInt divsor(bi), count(1);
    while( absCompare(divsor) >= 0 )
    {
        divsor.selfLeftShift(10);//fast
        count.selfLeftShift(10);
    }

    while(absCompare(bi) >= 0)
    {
        while(absCompare(divsor) < 0)
        {
            divsor.selfRightShift(1);//slow
            count.selfRightShift(1);
        }

        res += count;
        (*this) -= divsor;
    }

    m_vec = std::move(res.m_vec);
    if( m_vec.size() == 1 && m_vec[0] == 0 )
        m_sign = false;
    else
        m_sign = sign ^ bi.m_sign;

    return *this;
}


const BigInt BigInt::operator % (const BigInt &bi)const
{
    BigInt div = (*this) / bi;
    return (*this) - (div * bi);
}


BigInt& BigInt::operator %= (const BigInt &bi)
{
    BigInt div = (*this) / bi;
    BigInt res = (*this) - (div * bi);

    this->swap(res);
    return *this;
}



//-1 : less; 0 : equals; 1 greater
int BigInt::absCompare(const BigInt &bi) const
{
    if( m_vec.size() != bi.m_vec.size() )
    {
        return m_vec.size() < bi.m_vec.size() ? -1 : 1;
    }

    return BigInt::onlyVecCompare(m_vec, bi.m_vec);
}

const BigInt BigInt::abs()const
{
    BigInt tmp(*this);
    tmp.m_sign = false;
    return tmp;
}



bool BigInt::operator == (const BigInt &bi)const
{
    if( m_sign != bi.m_sign || m_vec.size() != bi.m_vec.size())
        return false;

    size_t i = 0;
    for(i = 0; i < m_vec.size(); ++i)
    {
        if( m_vec[i] != bi.m_vec[i] )
            break;
    }

    return i == m_vec.size();
}



bool BigInt::operator < (const BigInt &bi)const
{
    if( m_sign != bi.m_sign )
        return m_sign;

    //same sign
    int ret = absCompare(bi);
    if( ret == 0 )
        return false;

    /*
    if( m_sign )//negative
        return ret > 0;
    else //positive
        return ret < 0;
    */
    return m_sign ^ (ret < 0);
}


bool BigInt::operator > (const BigInt &bi)const
{
    if( m_sign != bi.m_sign )
        return !m_sign;

    int ret = absCompare(bi);
    if( ret == 0 )
        return false;

    //当同为正数时(m_sign为false)，对于正数来说，直接判断ret是否大于0即可。
    //此时异或的结果也是根据ret>0而确定的.
    //当同为负数时(m_sign为true)，对于负数来说，结果要与ret>0取反，
    //而异或在m_sign为true时会自动与ret>0取反.
    return m_sign ^ (ret > 0);
}


//left shift base Decimal
const BigInt BigInt::leftShift(size_t n)const
{
    if( n == 0 ) return *this;

    std::string str = toString();
    if( str.size() == 1 && str[0] == '0')
        return *this;

    str.insert(str.end(), n, '0');

    return BigInt(str);
}

//left shift base Decimal
BigInt& BigInt::selfLeftShift(size_t n)
{
    BigInt bi = leftShift(n);

    *this = bi;
    return *this;
}


//right shift base Decimal
const BigInt BigInt::rightShift(size_t n)const
{
    if( n == 0 ) return *this;

    std::string str = toString();

    while( !str.empty() && n-- )
        str.pop_back();

    if( str.empty() || (str.size() == 1 && str[0] == '-') )
    {
        str.clear();
        str.push_back('0');
    }

    return BigInt(str);
}

//right shift base Decimal
BigInt& BigInt::selfRightShift(size_t n)
{
    BigInt bi = rightShift(n);
    *this = bi;
    return *this;
}



const BigInt BigInt::operator >> (size_t n)const
{
    if( n == 0 )
        return *this;

    std::string str = toBinStringWithoutSign();

    if( str.size() <= n)
    {
        return BigInt(0);
    }
    else
    {
        str.erase(str.size()-n, n);
    }

    return BigInt::binString2BigInt(str, m_sign);
}


BigInt& BigInt::operator >>= (size_t n)
{
    if( n != 0)
    {
        BigInt tmp = (*this) >> n;
        *this = std::move(tmp);
    }

    return *this;
}

const BigInt BigInt::operator << (size_t n)const
{
    if( n == 0 )
        return *this;

    std::string str = toBinStringWithoutSign();
    str.insert(str.end(), n, '0');

    return BigInt::binString2BigInt(str, m_sign);
}


BigInt& BigInt::operator <<= (size_t n)
{
    if( n != 0 )
    {
        BigInt tmp = (*this) << n;
        *this = std::move(tmp);
    }

    return *this;
}


static std::string logicOperator(std::string &&s1, std::string &&s2, char op)
{
    if( s1.size() > s2.size() )
        s2.insert(s2.begin(), s1.size()-s2.size(), '0');
    else
        s1.insert(s1.begin(), s2.size()-s1.size(), '0');

    std::string ret;
    ret.reserve(s1.size());

    for(size_t i = 0; i < s1.size(); ++i)
    {
        char c = 0;
        switch (op) {
        case '&' :
            c = ((s1[i] - '0') & (s2[i] - '0')) + '0'; break;

        case '|' :
            c = ((s1[i] - '0') | (s2[i] - '0')) + '0'; break;

        case '^' :
            c = ((s1[i] - '0') ^ (s2[i] - '0')) + '0'; break;
        }

        ret.push_back(c);
    }

    return ret;
}

const BigInt BigInt::operator & (const BigInt &bi)const
{
    std::string ret = logicOperator(toBinStringWithoutSign(), bi.toBinStringWithoutSign(), '&');

    return BigInt::binString2BigInt(ret, bi.m_sign & m_sign);
}


BigInt& BigInt::operator &= (const BigInt &bi)
{
    BigInt tmp = (*this) & bi;
    *this = std::move(tmp);

    return *this;
}


const BigInt BigInt::operator | (const BigInt &bi)const
{
    std::string ret = logicOperator(toBinStringWithoutSign(), bi.toBinStringWithoutSign(), '|');
    return BigInt::binString2BigInt(ret, bi.m_sign | m_sign);
}


BigInt& BigInt::operator |= (const BigInt &bi)
{
    BigInt tmp = (*this) | bi;
    *this = std::move(tmp);

    return *this;
}

const BigInt BigInt::operator ^ (const BigInt &bi)const
{
    std::string ret = logicOperator(toBinStringWithoutSign(), bi.toBinStringWithoutSign(), '^');
    return BigInt::binString2BigInt(ret, bi.m_sign ^ m_sign);
}


BigInt& BigInt::operator ^= (const BigInt &bi)
{
    BigInt tmp = (*this) ^ bi;
    *this = std::move(tmp);

    return *this;
}


//----------------------private member function---------------

BigInt::BigInt(bool sign, const NumVec &vec)
    : m_sign(sign),
      m_vec(vec)
{

}

int BigInt::onlyVecCompare(const NumVec &lh, const NumVec &rh)
{
    if(lh.size() != rh.size() )
        return lh.size() < rh.size() ? -1 : 1;

    int i = static_cast<int>(lh.size()) - 1;
    for(; i >= 0; --i)
    {
        if( lh[i] != rh[i] )
            return lh[i] < rh[i] ? -1 : 1;
    }

    return 0;
}

static int addJudge(int val, int mask, BigInt::NumVec &vec)
{
    int inc;
    if( val < mask )
    {
        vec.push_back(val);
        inc = 0;
    }
    else
    {
        vec.push_back(val % mask);
        inc = 1;
    }

    return inc;
}


BigInt::NumVec BigInt::vecAdd(const NumVec &lh, const NumVec &rh)
{
    size_t i = 0, j = 0;

    BigInt::NumVec vec;
    vec.reserve(lh.size() + rh.size());
    int inc = 0;
    for( ; i < lh.size() && j < rh.size(); ++i, ++j)
    {
        int val = lh[i] + rh[j] + inc;
        inc = addJudge(val, s_mask, vec);
    }

    while( i < lh.size() )
    {
        int val = lh[i] + inc;
        inc = addJudge(val, s_mask, vec);
        ++i;
    }

    while( j < rh.size() )
    {
        int val = rh[j] + inc;
        inc = addJudge(val, s_mask, vec);
        ++j;
    }

    if( inc )
        vec.push_back(1);

    return vec;
}


BigInt::NumVec BigInt::vecSub(const NumVec &lh, const NumVec &rh)
{
    assert(BigInt::onlyVecCompare(lh, rh) >= 0);

    NumVec vec(lh);
    size_t i = 0, j = 0;

    for(; i < lh.size() && j < rh.size(); ++i, ++j)
    {
        if(vec[i] < rh[j]) //need to borrow number
        {
            int k = static_cast<int>(i + 1);
            while( vec[k] == 0 ) ++k;
            --vec[k];

            for(--k; k >= static_cast<int>(i); --k)
                vec[k] += s_mask - 1;
            vec[i] += 1;
        }

        vec[i] -= rh[j];
    }

    while( !vec.empty() && vec.back() == 0)
        vec.pop_back();

    if( vec.empty() )
        vec.push_back(0);

    return vec;
}



void BigInt::div2(BigInt &bi)
{
    bool add = false;

    for(auto it = bi.m_vec.rbegin(); it != bi.m_vec.rend(); ++it)
    {
        if( add )
            *it += s_mask;

        if( *it & 1)//odd num
            add = true;
        else
            add = false;

        *it /= 2;
    }


    while(!bi.m_vec.empty() && bi.m_vec.back() == 0)
        bi.m_vec.pop_back();

    if( bi.m_vec.empty())
    {
        bi.m_sign = false;
        bi.m_vec.push_back(0);
    }
}





BigInt BigInt::binString2BigInt(const std::string &str, bool sign)
{
    assert( !str.empty() );

    std::vector<BigInt> vecInt;
    vecInt.push_back(BigInt(1));

    for(auto it = std::next(str.rbegin()); it != str.rend(); ++it)
        vecInt.push_back( vecInt.back() * 2);

    BigInt res(0);
    auto it = str.rbegin();
    auto iter = vecInt.begin();
    for(; it != str.rend(); ++it, ++iter)
    {
        if( *it == '1' )
        {
            res += *iter;
        }
    }

    res.m_sign = sign;
    return res;
}
