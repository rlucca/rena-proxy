#include "adapter_base_service.h"
#include <libecap/common/registry.h>

namespace Adapter {

class HttpReplyService : public Adapter::BaseService
{
	public:
		virtual std::string uri() const {
			return "ecap://e-cap.org/ecap/services/reply/http_modifier";
		}

		virtual bool reply_service() const {
			return true;
		}
};

class HttpRequestService : public Adapter::BaseService
{
	public:
		virtual std::string uri() const {
			return "ecap://e-cap.org/ecap/services/request/http_modifier";
		}

		virtual bool request_service() const {
			return true;
		}
};

}

// create the adapter and register with libecap to reach the host application
Adapter::BaseService *registered[] = {
        new Adapter::HttpReplyService,
        new Adapter::HttpRequestService
    };

static const bool ReplyRegistered0 =
	libecap::RegisterVersionedService(registered[0]);
static const bool ReplyRegistered1 =
	libecap::RegisterVersionedService(registered[1]);
