#ifndef _ZKS_SERVICE_H
#define _ZKS_SERVICE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
//#include <any>

namespace zks {

namespace pattern {


    class Subscriber;
    using SubscriberPtr = std::weak_ptr<Subscriber>;
    using SubscriberCallBack = std::function<void(void*)>;

    struct SubscriberPtrHash {
        size_t operator()(SubscriberPtr const& wp) const {
            if (auto sp = wp.lock()) {
                return std::hash<decltype(sp)>()(sp);
            }
            return std::hash<size_t>()(0);
        }
    };

    struct SubscriberPtrComp {
        bool operator()(SubscriberPtr const& lhp, SubscriberPtr const& rhp) const {
            return !lhp.owner_before(rhp) && !rhp.owner_before(lhp);
        }
    };

    class Service {
        std::unordered_map<SubscriberPtr, SubscriberCallBack, SubscriberPtrHash, SubscriberPtrComp> subscriptions_;

    public:
        Service();
        virtual ~Service() = 0;

        int reg(SubscriberPtr pSub, SubscriberCallBack cb);
        int unreg(SubscriberPtr pSub);
        int purge();
        int notify(void* msg = nullptr) const;
    };

    using ServicePtr = std::weak_ptr<Service>;

    class Subscriber : private std::enable_shared_from_this<Subscriber> {
    public:
        virtual ~Subscriber();

        int subscribe(ServicePtr svc);
        int unsubscribe(ServicePtr svc);

        virtual void on_notification(void* msg);
    };
}


}


#endif // _ZKS_SERVICE_H