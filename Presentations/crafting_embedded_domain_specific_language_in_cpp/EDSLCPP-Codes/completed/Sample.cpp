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

#include "Async.hpp"
#include <iostream>

#include <windows.h>
int main()
{
    std::cout << "Hello, ThreadID: " << GetCurrentThreadId() << std::endl;
    
    tfc_async
    {
        std::cout << "On Async sleep 2000, ThreadID: " << GetCurrentThreadId() << std::endl;
        Sleep(2000);
    }
    tfc_async_complete
    {
        std::cout << "On AsyncComplete sleep 2000" << std::endl;
    };

    tfc_async
    {
        std::cout << "On Async sleep 1000, ThreadID: " << GetCurrentThreadId() << std::endl;
        Sleep(1000);
    }
    tfc_async_complete
    {
        std::cout << "On AsyncComplete sleep 1000" << std::endl;
    };

    tfc_async
    {
        std::cout << "On Async counter, ThreadID: " << GetCurrentThreadId() << std::endl;
        int count = 0;

        for(int i = 0; i < 100; i++)
        {
            count += rand();
            Sleep(10);
        }
        std::cout << "On Async counter: " << count << std::endl;
        return count;
    }
    tfc_async_complete(double count)
    {
        std::cout << "On AsyncComplete counter: " << count << std::endl;
    };

    std::cout << "Before Blocking" << std::endl;

    // Simulate a main loop which will get notified when a thread is completed
    BlockAndWait();
    
    std::cout << "Done" << std::endl;
    return 0;
}


