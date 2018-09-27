#include <iostream>

class AsyncOperand
{

};

class CompleteOperand
{

};

class AsyncBuilder
{
public:
    template<typename TLambda>
    void operator&(TLambda l)
    {
        std::cout << "Async with lambda\n";
    }

    void operator&(AsyncOperand operand)
    {
        std::cout << "Async with AsyncOperand\n";
    }
};

class CompleteBuilder
{
public:
    template<typename TLambda>
    CompleteOperand operator*(TLambda complete)
    {
        std::cout << "Complete with lambda\n";
        return { };
    }
};

template<typename TLambda>
AsyncOperand operator>>(TLambda l, CompleteOperand c)
{
    std::cout << "Combining CompleteOperand and async lambda to AsyncOperand\n";
    return { };
}

#define tfc_async AsyncBuilder() & [] ()
#define tfc_async_complete >> CompleteBuilder() * []

int main()
{
    std::cout << "Hello" << std::endl;

    AsyncBuilder() & [] () { } >> CompleteBuilder() * [] () { };

    tfc_async
    {

    }
    tfc_async_complete
    {

    };

    std::cout << "Done" << std::endl;
    //std::getchar();
    return 0;
}


