//Filename: BigInt.h
//Date: 2015/10/6

//Author: luotuo44   http://blog.csdn.net/luotuo44

//Copyright 2015, luotuo44. All rights reserved.
//Use of this source code is governed by a BSD-style license

#ifndef BIGINT_H
#define BIGINT_H

#include<vector>
#include<string>

class BigInt
{
public:
    using NumVec = std::vector<int>;

public:
    BigInt(int val)noexcept;
    explicit BigInt(const std::string &str);

    BigInt(const BigInt &bi);
    BigInt& operator = (const BigInt &bi);

    BigInt(BigInt &&bi);
    BigInt& operator = (BigInt &&bi);

    void swap(BigInt &bi)noexcept;

    std::string toString()const;
    std::string toBinString()const;
    std::string toBinStringWithoutSign()const;

    BigInt& operator - ();//reverse the sign

    BigInt& operator ++();
    const BigInt operator ++(int);
    BigInt& operator --();
    const BigInt operator --(int);

    const BigInt operator + (const BigInt &bi)const;
    BigInt& operator += (const BigInt &bi);
    const BigInt operator - (const BigInt &bi)const;
    BigInt& operator -= (const BigInt &bi);

    const BigInt operator * (const BigInt &bi)const;
    BigInt& operator *= (const BigInt &bi);

    const BigInt operator / (const BigInt &bi)const;
    BigInt& operator /= (const BigInt &bi);

    const BigInt operator % (const BigInt &bi)const;
    BigInt& operator %= (const BigInt &bi);

    //-1 : less; 0 : equals; 1 greater
    int absCompare(const BigInt &bi)const;

    const BigInt abs()const;


    bool operator == (const BigInt &bi)const;
    bool operator != (const BigInt &bi)const
    {
        return !((*this) == bi);
    }

    bool operator < (const BigInt &bi)const;
    bool operator > (const BigInt &bi)const;

    bool operator <= (const BigInt &bi)const
    {
        return !( *this > bi);
    }
    bool operator >= (const BigInt &bi)const
    {
        return !( *this < bi);
    }


    //left shift base Decimal
    const BigInt leftShift(size_t n)const;
    BigInt& selfLeftShift(size_t n);

    //right shift base Decimal
    const BigInt rightShift(size_t n)const;
    BigInt& selfRightShift(size_t n);


    const BigInt operator >> (size_t n)const;
    BigInt& operator >>= (size_t n);

    const BigInt operator << (size_t n)const;
    BigInt& operator <<= (size_t n);

    const BigInt operator & (const BigInt &bi)const;
    BigInt& operator &= (const BigInt &bi);

    const BigInt operator | (const BigInt &bi)const;
    BigInt& operator |= (const BigInt &bi);

    const BigInt operator ^ (const BigInt &bi)const;
    BigInt& operator ^= (const BigInt &bi);


private:
    BigInt(bool sign, const NumVec &vec);

    static int onlyVecCompare(const NumVec &lh, const NumVec &rh);
    static NumVec vecAdd(const NumVec &lh, const NumVec &rh);
    static NumVec vecSub(const NumVec &lh, const NumVec &rh);

    static void div2(BigInt &bi);
    static BigInt binString2BigInt(const std::string &str, bool sign);

private:
    bool m_sign;//true for negative
    //num 123456789 will be save as
    //vec[0] = 6789, vec[1] = 2345, vec[2] = 1,
    //when s_span equals 4
    NumVec m_vec;

    static int s_span;
    static int s_mask;
};


inline BigInt& BigInt::operator - ()//reverse the sign
{
    m_sign = !m_sign;
    return *this;
}

inline BigInt& BigInt::operator ++()
{
    *this += 1;
    return *this;
}

inline const BigInt BigInt::operator ++(int)
{
    BigInt tmp(*this);
    *this += 1;
    return tmp;
}


inline BigInt& BigInt::operator --()
{
    *this -= 1;
    return *this;
}


inline const BigInt BigInt::operator --(int)
{
    BigInt tmp(*this);
    *this -= 1;
    return tmp;
}

#endif // BIGINT_H
