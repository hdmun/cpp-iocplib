#ifndef __OVERLAPPED_H__
#define __OVERLAPPED_H__

namespace iocplib {
    enum eOverlappedType
    {
        Recv = 1,
        Send,
    };

    struct OverlappedEventInterface;

    struct OverlappedContext
        : public OVERLAPPED
    {
        OverlappedContext()
        {
            std::memset(this, 0, sizeof(*this));
        }

        typedef union {
            eOverlappedType type;
            void* obj;
        } Data;

        Data data;
        OverlappedEventInterface* callback{ nullptr };
    };

    struct OverlappedEventInterface
    {
        virtual void OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) = 0;
    };

}

#endif