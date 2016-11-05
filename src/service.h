#ifndef _ZKS_SERVICE_H
#define _ZKS_SERVICE_H

#include <vector>
#include <memory>
#include <functional>

namespace zks {

namespace pattern {


    class Subscriber;

    class Service {
    public:
        using SubscriberCallBack = std::function<void(int)>;
        struct Subscription {
            std::weak_ptr<Subscriber> subscriber;
            SubscriberCallBack callback;
        };

    private:
        std::vector<Subscription> subscriptions_;

    public:
        Service();
        virtual ~Service();

        int reg()
    };

    class Subscriber {
    public:
        Subscriber();
        virtual ~Subscriber();

        int subscribe();
        int unsubscribe();
    };

    class ServiceCenter {
    public:
        int reg_service();
        int unreg_service();
        int subscribe();
        int unsubscribe();
    }
}


}


#endif // _ZKS_SERVICE_H