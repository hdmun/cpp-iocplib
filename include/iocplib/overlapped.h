#ifndef __OVERLAPPED_H__
#define __OVERLAPPED_H__

namespace iocplib {
    struct OverlappedEventInterface
    {
        virtual void OnComplete( void* data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey ) = 0;
    };

    struct OverlappedContext
        : public OVERLAPPED
    {
        void* data{ nullptr };
        OverlappedEventInterface* callback{ nullptr };
    };
}

#endif