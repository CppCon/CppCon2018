/* MIT License
 *
 * Copyright (c) 2018 Gilang Mentari Hamidy (gilang.hamidy@gmail.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>

template<typename TLambda, typename TCompleteLambda>
struct AsyncOperand
{
    TLambda lambda;
    TCompleteLambda completeLambda;
};

template<typename TCompleteLambda>
struct CompleteOperand
{
    TCompleteLambda lambda;
};

class AsyncBuilder
{
public:
    template<typename TLambda>
    void operator&(TLambda l)
    {
        std::cout << "Async with lambda\n";
    }

    template<typename TLambda, typename TCompleteLambda>
    void operator&(AsyncOperand<TLambda, TCompleteLambda> operand)
    {
        std::cout << "Async with AsyncOperand\n";
    }
};

class CompleteBuilder
{
public:
    template<typename TCompleteLambda>
    CompleteOperand<TCompleteLambda> operator*(TCompleteLambda complete)
    {
        std::cout << "Complete with lambda\n";
        return { complete };
    }
};

template<typename TLambda, typename TCompleteLambda>
AsyncOperand<TLambda, TCompleteLambda> operator>>(TLambda l, CompleteOperand<TCompleteLambda> c)
{
    std::cout << "Combining CompleteOperand and async lambda to AsyncOperand\n";
    return { l, c.lambda };
}


#define tfc_async AsyncBuilder() & [] ()
#define tfc_async_complete >> CompleteBuilder() * []

int main()
{
    std::cout << "Hello" << std::endl;

    tfc_async
    {
        
    }
    tfc_async_complete
    {

    };

    std::cout << "Done" << std::endl;

    return 0;
}


