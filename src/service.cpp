#include "service.h"

using namespace std;

namespace zks {

namespace pattern {

    Service::Service()
    {
    }


    Service::~Service()
    {
    }

    int Service::reg(SubscriberPtr pSub, SubscriberCallBack cb)
    {
        if (auto sp = pSub.lock()) {
            subscriptions_[pSub] = cb;
        }
        else {
            return -1;
        }
        return 0;
    }

    int Service::unreg(SubscriberPtr pSub)
    {
        if (auto sp = pSub.lock()) {
            subscriptions_.erase(pSub);
        }
        else {
            return -1;
        }
        return 0;
    }

    int Service::purge()
    {
        vector<SubscriberPtr> tbd;
        for (auto& sub : subscriptions_) {
            if (!sub.first.lock()) {
                tbd.push_back(sub.first);
            }
        }
        for (auto const& s : tbd) {
            subscriptions_.erase(s);
        }
        return (int)tbd.size();
    }

    int Service::notify(void * msg) const
    {
        int ret = 0;
        for (auto const& sub : subscriptions_) {
            if (auto sp = sub.first.lock()) {
                if (sub.second) {
                    sub.second(msg);
                    ++ret;
                }
            }
        }
        return ret;
    }

    Subscriber::~Subscriber()
    {
    }

    int Subscriber::subscribe(ServicePtr svc)
    {
        if (auto sp = svc.lock()) {
            sp->reg(shared_from_this(), 
                bind(&Subscriber::on_notification, this, placeholders::_1));
        }
        return 0;
    }

    int Subscriber::unsubscribe(ServicePtr svc)
    {
        if (auto sp = svc.lock()) {
            sp->unreg(shared_from_this());
        }
        return 0;
    }

    void Subscriber::on_notification(void * msg)
    {
        msg;
    }
     
}
}
