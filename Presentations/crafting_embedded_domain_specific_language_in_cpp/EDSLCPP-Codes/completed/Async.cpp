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

#include <list>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

struct ThreadInfo
{
    AsyncContextErased context;
    HANDLE threadHandle;
    DWORD threadId;
    bool done { false };
    ThreadInfo(AsyncContextErased&& ctx) : context(ctx) { }
};

std::list<ThreadInfo> info;

DWORD WINAPI AsyncWorks(LPVOID lpParam)
{
    auto threadInfo = static_cast<ThreadInfo*>(lpParam);
    threadInfo->context.asyncFuncPtr(threadInfo->context.context);
    threadInfo->done = true;
    return 0;
}

void DispatchThread(AsyncContextErased ctx)
{
    decltype(auto) ctxItem = info.emplace_back(std::move(ctx));
    ctxItem.threadHandle = CreateThread(NULL, 0, AsyncWorks, &ctxItem, 0, &ctxItem.threadId);
}

void BlockAndWait()
{
    while(!info.empty())
    {
        for(auto iter = info.begin(); iter != info.end();)
        {
            auto& infoItem = *iter;
            if(infoItem.done)
            {
                infoItem.context.completeFuncPtr(infoItem.context.context);
                auto oldIter = iter++;
                info.erase(oldIter);
            }
            else
            {
                iter++;
            }
        }
        Sleep(100); // Throttle
    }
}
