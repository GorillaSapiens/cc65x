
/*============================================================================

This C source fragment is part of the Berkeley SoftFloat IEEE Floating-Point
Arithmetic Package, Release 2c, by John R. Hauser.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TOLERATE ALL LOSSES, COSTS, OR OTHER
PROBLEMS THEY INCUR DUE TO THE SOFTWARE WITHOUT RECOMPENSE FROM JOHN HAUSER OR
THE INTERNATIONAL COMPUTER SCIENCE INSTITUTE, AND WHO FURTHERMORE EFFECTIVELY
INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE INSTITUTE
(possibly via similar legal notice) AGAINST ALL LOSSES, COSTS, OR OTHER
PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE, OR
INCURRED BY ANYONE DUE TO A DERIVATIVE WORK THEY CREATE USING ANY PART OF THE
SOFTWARE.

Derivative works require also that (1) the source code for the derivative work
includes prominent notice that the work is derivative, and (2) the source code
includes prominent notice of these three paragraphs for those parts of this
code that are retained.

=============================================================================*/

/*----------------------------------------------------------------------------
| Shifts `a' right by the number of bits given in `count'.  If any nonzero
| bits are shifted off, they are "jammed" into the least significant bit of
| the result by setting the least significant bit to 1.  The value of `count'
| can be arbitrarily large; in particular, if `count' is greater than 32, the
| result will be either 0 or 1, depending on whether `a' is zero or nonzero.
| The result is stored in the location pointed to by `zPtr'.
*----------------------------------------------------------------------------*/

INLINE void shift32RightJamming( bits32 a, int16 count, bits32 *zPtr )
{
    bits32 z;

    if ( count == 0 ) {
        z = a;
    }
    else if ( count < 32 ) {
        z = ( a>>count ) | ( ( a<<( ( - count ) & 31 ) ) != 0 );
    }
    else {
        z = ( a != 0 );
    }
    *zPtr = z;

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Shifts the 64-bit value formed by concatenating `a0' and `a1' right by the
| number of bits given in `count'.  Any bits shifted off are lost.  The value
| of `count' can be arbitrarily large; in particular, if `count' is greater
| than 64, the result will be 0.  The result is broken into two 32-bit pieces
| which are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 shift64Right(
     bits32 a0, bits32 a1, int16 count, bits32 *z0Ptr, bits32 *z1Ptr )
{
    bits32 z0, z1;
    int8 negCount = ( - count ) & 31;

    if ( count == 0 ) {
        z1 = a1;
        z0 = a0;
    }
    else if ( count < 32 ) {
        z1 = ( a0<<negCount ) | ( a1>>count );
        z0 = a0>>count;
    }
    else {
        z1 = ( count < 64 ) ? ( a0>>( count & 31 ) ) : 0;
        z0 = 0;
    }
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Shifts the 64-bit value formed by concatenating `a0' and `a1' right by the
| number of bits given in `count'.  If any nonzero bits are shifted off, they
| are "jammed" into the least significant bit of the result by setting the
| least significant bit to 1.  The value of `count' can be arbitrarily large;
| in particular, if `count' is greater than 64, the result will be either 0
| or 1, depending on whether the concatenation of `a0' and `a1' is zero or
| nonzero.  The result is broken into two 32-bit pieces which are stored at
| the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 shift64RightJamming(
     bits32 a0, bits32 a1, int16 count, bits32 *z0Ptr, bits32 *z1Ptr )
{
    bits32 z0, z1;
    int8 negCount = ( - count ) & 31;

    if ( count == 0 ) {
        z1 = a1;
        z0 = a0;
    }
    else if ( count < 32 ) {
        z1 = ( a0<<negCount ) | ( a1>>count ) | ( ( a1<<negCount ) != 0 );
        z0 = a0>>count;
    }
    else {
        if ( count == 32 ) {
            z1 = a0 | ( a1 != 0 );
        }
        else if ( count < 64 ) {
            z1 = ( a0>>( count & 31 ) ) | ( ( ( a0<<negCount ) | a1 ) != 0 );
        }
        else {
            z1 = ( ( a0 | a1 ) != 0 );
        }
        z0 = 0;
    }
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Shifts the 96-bit value formed by concatenating `a0', `a1', and `a2' right
| by 32 _plus_ the number of bits given in `count'.  The shifted result is
| at most 64 nonzero bits; these are broken into two 32-bit pieces which are
| stored at the locations pointed to by `z0Ptr' and `z1Ptr'.  The bits shifted
| off form a third 32-bit result as follows:  The _last_ bit shifted off is
| the most-significant bit of the extra result, and the other 31 bits of the
| extra result are all zero if and only if _all_but_the_last_ bits shifted off
| were all zero.  This extra result is stored in the location pointed to by
| `z2Ptr'.  The value of `count' can be arbitrarily large.
|     (This routine makes more sense if `a0', `a1', and `a2' are considered
| to form a fixed-point value with binary point between `a1' and `a2'.  This
| fixed-point value is shifted right by the number of bits given in `count',
| and the integer part of the result is returned at the locations pointed to
| by `z0Ptr' and `z1Ptr'.  The fractional part of the result may be slightly
| corrupted as described above, and is returned at the location pointed to by
| `z2Ptr'.)
*----------------------------------------------------------------------------*/

INLINE void
 shift64ExtraRightJamming(
     bits32 a0,
     bits32 a1,
     bits32 a2,
     int16 count,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr
 )
{
    bits32 z0, z1, z2;
    int8 negCount = ( - count ) & 31;

    if ( count == 0 ) {
        z2 = a2;
        z1 = a1;
        z0 = a0;
    }
    else {
        if ( count < 32 ) {
            z2 = a1<<negCount;
            z1 = ( a0<<negCount ) | ( a1>>count );
            z0 = a0>>count;
        }
        else {
            if ( count == 32 ) {
                z2 = a1;
                z1 = a0;
            }
            else {
                a2 |= a1;
                if ( count < 64 ) {
                    z2 = a0<<negCount;
                    z1 = a0>>( count & 31 );
                }
                else {
                    z2 = ( count == 64 ) ? a0 : ( a0 != 0 );
                    z1 = 0;
                }
            }
            z0 = 0;
        }
        z2 |= ( a2 != 0 );
    }
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

/*----------------------------------------------------------------------------
| Shifts the 64-bit value formed by concatenating `a0' and `a1' left by the
| number of bits given in `count'.  Any bits shifted off are lost.  The value
| of `count' must be less than 32.  The result is broken into two 32-bit
| pieces which are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 shortShift64Left(
     bits32 a0, bits32 a1, int16 count, bits32 *z0Ptr, bits32 *z1Ptr )
{

    *z1Ptr = a1<<count;
    *z0Ptr =
        ( count == 0 ) ? a0 : ( a0<<count ) | ( a1>>( ( - count ) & 31 ) );

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Shifts the 96-bit value formed by concatenating `a0', `a1', and `a2' left
| by the number of bits given in `count'.  Any bits shifted off are lost.
| The value of `count' must be less than 32.  The result is broken into three
| 32-bit pieces which are stored at the locations pointed to by `z0Ptr',
| `z1Ptr', and `z2Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 shortShift96Left(
     bits32 a0,
     bits32 a1,
     bits32 a2,
     int16 count,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr
 )
{
    bits32 z0, z1, z2;
    int8 negCount;

    z2 = a2<<count;
    z1 = a1<<count;
    z0 = a0<<count;
    if ( 0 < count ) {
        negCount = ( ( - count ) & 31 );
        z1 |= a2>>negCount;
        z0 |= a1>>negCount;
    }
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

/*----------------------------------------------------------------------------
| Adds the 64-bit value formed by concatenating `a0' and `a1' to the 64-bit
| value formed by concatenating `b0' and `b1'.  Addition is modulo 2^64, so
| any carry out is lost.  The result is broken into two 32-bit pieces which
| are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 add64(
     bits32 a0, bits32 a1, bits32 b0, bits32 b1, bits32 *z0Ptr, bits32 *z1Ptr )
{
    bits32 z1;

    z1 = a1 + b1;
    *z1Ptr = z1;
    *z0Ptr = a0 + b0 + ( z1 < a1 );

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Adds the 96-bit value formed by concatenating `a0', `a1', and `a2' to the
| 96-bit value formed by concatenating `b0', `b1', and `b2'.  Addition is
| modulo 2^96, so any carry out is lost.  The result is broken into three
| 32-bit pieces which are stored at the locations pointed to by `z0Ptr',
| `z1Ptr', and `z2Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 add96(
     bits32 a0,
     bits32 a1,
     bits32 a2,
     bits32 b0,
     bits32 b1,
     bits32 b2,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr
 )
{
    bits32 z0, z1, z2;
    int8 carry0, carry1;

    z2 = a2 + b2;
    carry1 = ( z2 < a2 );
    z1 = a1 + b1;
    carry0 = ( z1 < a1 );
    z0 = a0 + b0;
    z1 += carry1;
    z0 += ( z1 < carry1 );
    z0 += carry0;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

/*----------------------------------------------------------------------------
| Subtracts the 64-bit value formed by concatenating `b0' and `b1' from the
| 64-bit value formed by concatenating `a0' and `a1'.  Subtraction is modulo
| 2^64, so any borrow out (carry out) is lost.  The result is broken into two
| 32-bit pieces which are stored at the locations pointed to by `z0Ptr' and
| `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 sub64(
     bits32 a0, bits32 a1, bits32 b0, bits32 b1, bits32 *z0Ptr, bits32 *z1Ptr )
{

    *z1Ptr = a1 - b1;
    *z0Ptr = a0 - b0 - ( a1 < b1 );

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Subtracts the 96-bit value formed by concatenating `b0', `b1', and `b2' from
| the 96-bit value formed by concatenating `a0', `a1', and `a2'.  Subtraction
| is modulo 2^96, so any borrow out (carry out) is lost.  The result is broken
| into three 32-bit pieces which are stored at the locations pointed to by
| `z0Ptr', `z1Ptr', and `z2Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 sub96(
     bits32 a0,
     bits32 a1,
     bits32 a2,
     bits32 b0,
     bits32 b1,
     bits32 b2,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr
 )
{
    bits32 z0, z1, z2;
    int8 borrow0, borrow1;

    z2 = a2 - b2;
    borrow1 = ( a2 < b2 );
    z1 = a1 - b1;
    borrow0 = ( a1 < b1 );
    z0 = a0 - b0;
    z0 -= ( z1 < borrow1 );
    z1 -= borrow1;
    z0 -= borrow0;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

/*----------------------------------------------------------------------------
| Multiplies `a' by `b' to obtain a 64-bit product.  The product is broken
| into two 32-bit pieces which are stored at the locations pointed to by
| `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

INLINE void mul32To64( bits32 a, bits32 b, bits32 *z0Ptr, bits32 *z1Ptr )
{
    bits16 aHigh, aLow, bHigh, bLow;
    bits32 z0, zMiddleA, zMiddleB, z1;

    aLow = a;
    aHigh = a>>16;
    bLow = b;
    bHigh = b>>16;
    z1 = ( (bits32) aLow ) * bLow;
    zMiddleA = ( (bits32) aLow ) * bHigh;
    zMiddleB = ( (bits32) aHigh ) * bLow;
    z0 = ( (bits32) aHigh ) * bHigh;
    zMiddleA += zMiddleB;
    z0 += ( ( (bits32) ( zMiddleA < zMiddleB ) )<<16 ) + ( zMiddleA>>16 );
    zMiddleA <<= 16;
    z1 += zMiddleA;
    z0 += ( z1 < zMiddleA );
    *z1Ptr = z1;
    *z0Ptr = z0;

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Multiplies the 64-bit value formed by concatenating `a0' and `a1' by `b'
| to obtain a 96-bit product.  The product is broken into three 32-bit pieces
| which are stored at the locations pointed to by `z0Ptr', `z1Ptr', and
| `z2Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 mul64By32To96(
     bits32 a0,
     bits32 a1,
     bits32 b,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr
 )
{
    bits32 z0, z1, z2, more1;

    mul32To64( a1, b, &z1, &z2 );
    mul32To64( a0, b, &z0, &more1 );
    add64( z0, more1, 0, z1, &z0, &z1 );
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Multiplies the 64-bit value formed by concatenating `a0' and `a1' to the
| 64-bit value formed by concatenating `b0' and `b1' to obtain a 128-bit
| product.  The product is broken into four 32-bit pieces which are stored at
| the locations pointed to by `z0Ptr', `z1Ptr', `z2Ptr', and `z3Ptr'.
*----------------------------------------------------------------------------*/

INLINE void
 mul64To128(
     bits32 a0,
     bits32 a1,
     bits32 b0,
     bits32 b1,
     bits32 *z0Ptr,
     bits32 *z1Ptr,
     bits32 *z2Ptr,
     bits32 *z3Ptr
 )
{
    bits32 z0, z1, z2, z3;
    bits32 more1, more2;

    mul32To64( a1, b1, &z2, &z3 );
    mul32To64( a1, b0, &z1, &more2 );
    add64( z1, more2, 0, z2, &z1, &z2 );
    mul32To64( a0, b0, &z0, &more1 );
    add64( z0, more1, 0, z1, &z0, &z1 );
    mul32To64( a0, b1, &more1, &more2 );
    add64( more1, more2, 0, z2, &more1, &z2 );
    add64( z0, z1, 0, more1, &z0, &z1 );
    *z3Ptr = z3;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;

}
#endif

/*----------------------------------------------------------------------------
| Returns an approximation to the 32-bit integer quotient obtained by dividing
| `b' into the 64-bit value formed by concatenating `a0' and `a1'.  The
| divisor `b' must be at least 2^31.  If q is the exact quotient truncated
| toward zero, the approximation returned lies between q and q + 2 inclusive.
| If the exact quotient q is larger than 32 bits, the maximum positive 32-bit
| unsigned integer is returned.
*----------------------------------------------------------------------------*/

static bits32 estimateDiv64To32( bits32 a0, bits32 a1, bits32 b )
{
    bits32 b0, b1;
    bits32 rem0, rem1, term0, term1;
    bits32 z;

    if ( b <= a0 ) return 0xFFFFFFFF;
    b0 = b>>16;
    z = ( b0<<16 <= a0 ) ? 0xFFFF0000 : ( a0 / b0 )<<16;
    mul32To64( b, z, &term0, &term1 );
    sub64( a0, a1, term0, term1, &rem0, &rem1 );
    while ( ( (sbits32) rem0 ) < 0 ) {
        z -= 0x10000;
        b1 = b<<16;
        add64( rem0, rem1, b0, b1, &rem0, &rem1 );
    }
    rem0 = ( rem0<<16 ) | ( rem1>>16 );
    z |= ( b0<<16 <= rem0 ) ? 0xFFFF : rem0 / b0;
    return z;

}

/*----------------------------------------------------------------------------
| Returns an approximation to the square root of the 32-bit significand given
| by `a'.  Considered as an integer, `a' must be at least 2^31.  If bit 0 of
| `aExp' (the least significant bit) is 1, the integer returned approximates
| 2^31*sqrt(`a'/2^31), where `a' is considered an integer.  If bit 0 of `aExp'
| is 0, the integer returned approximates 2^31*sqrt(`a'/2^30).  In either
| case, the approximation returned lies strictly within +/-2 of the exact
| value.
*----------------------------------------------------------------------------*/

static bits32 estimateSqrt32( int16 aExp, bits32 a )
{
    static const bits16 sqrtOddAdjustments[] = {
        0x0004, 0x0022, 0x005D, 0x00B1, 0x011D, 0x019F, 0x0236, 0x02E0,
        0x039C, 0x0468, 0x0545, 0x0631, 0x072B, 0x0832, 0x0946, 0x0A67
    };
    static const bits16 sqrtEvenAdjustments[] = {
        0x0A2D, 0x08AF, 0x075A, 0x0629, 0x051A, 0x0429, 0x0356, 0x029E,
        0x0200, 0x0179, 0x0109, 0x00AF, 0x0068, 0x0034, 0x0012, 0x0002
    };
    int8 index;
    bits32 z;

    index = ( a>>27 ) & 15;
    if ( aExp & 1 ) {
        z = 0x4000 + ( a>>17 ) - sqrtOddAdjustments[ index ];
        z = ( ( a / z )<<14 ) + ( z<<15 );
        a >>= 1;
    }
    else {
        z = 0x8000 + ( a>>17 ) - sqrtEvenAdjustments[ index ];
        z = a / z + z;
        z = ( 0x20000 <= z ) ? 0xFFFF8000 : ( z<<15 );
        if ( z <= a ) return (bits32) ( ( (sbits32) a )>>1 );
    }
    return ( ( estimateDiv64To32( a, 0, z ) )>>1 ) + ( z>>1 );

}

/*----------------------------------------------------------------------------
| Returns the number of leading 0 bits before the most-significant 1 bit of
| `a'.  If `a' is zero, 32 is returned.
*----------------------------------------------------------------------------*/

static int8 countLeadingZeros32( bits32 a )
{
    static const int8 countLeadingZerosHigh[] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int8 shiftCount;

    shiftCount = 0;
    if ( a < 0x10000 ) {
        shiftCount += 16;
        a <<= 16;
    }
    if ( a < 0x1000000 ) {
        shiftCount += 8;
        a <<= 8;
    }
    shiftCount += countLeadingZerosHigh[ a>>24 ];
    return shiftCount;

}

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Returns 1 if the 64-bit value formed by concatenating `a0' and `a1' is
| equal to the 64-bit value formed by concatenating `b0' and `b1'.  Otherwise,
| returns 0.
*----------------------------------------------------------------------------*/

INLINE flag eq64( bits32 a0, bits32 a1, bits32 b0, bits32 b1 )
{

    return ( a0 == b0 ) && ( a1 == b1 );

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Returns 1 if the 64-bit value formed by concatenating `a0' and `a1' is less
| than or equal to the 64-bit value formed by concatenating `b0' and `b1'.
| Otherwise, returns 0.
*----------------------------------------------------------------------------*/

INLINE flag le64( bits32 a0, bits32 a1, bits32 b0, bits32 b1 )
{

    return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 <= b1 ) );

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Returns 1 if the 64-bit value formed by concatenating `a0' and `a1' is less
| than the 64-bit value formed by concatenating `b0' and `b1'.  Otherwise,
| returns 0.
*----------------------------------------------------------------------------*/

INLINE flag lt64( bits32 a0, bits32 a1, bits32 b0, bits32 b1 )
{

    return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 < b1 ) );

}
#endif

#ifdef DOUBLES
/*----------------------------------------------------------------------------
| Returns 1 if the 64-bit value formed by concatenating `a0' and `a1' is not
| equal to the 64-bit value formed by concatenating `b0' and `b1'.  Otherwise,
| returns 0.
*----------------------------------------------------------------------------*/

INLINE flag ne64( bits32 a0, bits32 a1, bits32 b0, bits32 b1 )
{

    return ( a0 != b0 ) || ( a1 != b1 );

}
#endif

