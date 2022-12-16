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
        std::weak_ptr<OverlappedEventInterface> callback;
    };

    struct OverlappedEventInterface
    {
        virtual void OnCompleteOverlappedIO(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) = 0;
    };

}

#endif