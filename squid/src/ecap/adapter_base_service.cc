
#include "adapter_base_service.h"
#include "squid_internal_action.h"
#include <iostream>
#include <cstring>
#include <libecap/common/name.h>
#include <libecap/common/errors.h>
#include <libecap/common/named_values.h>
#include <libecap/common/names.h>
#include "queue.h"
#include <iostream>
#include <cstdlib>

void *Adapter::BaseService::job_queue_buffer[256] = { NULL, };
void *Adapter::BaseService::answer_queue_buffer[256] = { NULL, };

// Calls Service::setOne() for each host-provided configuration option.
// See Service::configure().
class Cfgtor: public libecap::NamedValueVisitor {
	public:
		Cfgtor(Adapter::BaseService &aSvc): svc(aSvc)
		{ /* EMPTY */ }

		virtual void visit(const libecap::Name &name, const libecap::Area &value)
		{
			svc.setOne(name, value);
		}

		Adapter::BaseService &svc;
};

static const std::string CfgErrorPrefix =
	"Sqlite Adapter: configuration error: ";


std::string Adapter::BaseService::tag() const {
	return std::string("1.0.0");
}

void Adapter::BaseService::describe(std::ostream &os) const {
	os << "Modifica o conteudo com base nas urls armazenadas "
		<< "no arquivo sqlite informado";
}

void Adapter::BaseService::configure(const libecap::Options &cfg) {
	Cfgtor cfgtor(*this);
	//std::cout << "configure called!";
	cfg.visitEachOption(cfgtor);

	// check for post-configuration errors and inconsistencies

	if (module.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"module_name value is not set");
	}

	if (pypath.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"module_directory value is not set");
	}

	if (suffix.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"suffix value is not set");
	}

	if ((!reply_service() && !request_service())
		|| (reply_service() && request_service()))
	{
		throw libecap::TextException(CfgErrorPrefix +
			"setup error on URI:" + uri());
		return ;
	}

    pyin_start(pypath.c_str());
    pyModule = pyin_get_module(module.c_str());
    
    std::string db_path = pypath + "/dbs";
    pyController = pyin_create_controller(pyModule, suffix.c_str(),
                                          db_path.c_str());
    SquidInternalAction::getInstance()->add(db_path);
}

void Adapter::BaseService::reconfigure(const libecap::Options &cfg) {
    pyin_finish(&pyModule, &pyController);

	module.clear();
	pypath.clear();
	suffix.clear();
	configure(cfg);
}

void Adapter::BaseService::setOne(const libecap::Name &name, const libecap::Area &valArea) {
	const std::string value = valArea.toString();
	if (name == "module_name")
		setModule(value);
	else if (name == "module_directory")
		setPythonPath(value);
	else if (name == "suffix")
		setSuffix(value);
	else if (name.assignedHostId())
		; // skip host-standard options we do not know or care about
	else
		throw libecap::TextException(CfgErrorPrefix +
			"unsupported configuration parameter: " + name.image());
}

void Adapter::BaseService::setModule(const std::string &value) {
	if (value.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"empty module_name value is not allowed");
	}
	module = value;
}

void Adapter::BaseService::setPythonPath(const std::string &value) {
	if (value.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"empty module_directory value is not allowed");
	}
	pypath = value;
}

void Adapter::BaseService::setSuffix(const std::string &value) {
	if (value.empty()) {
		throw libecap::TextException(CfgErrorPrefix +
			"empty suffix value is not allowed");
	}
	suffix = value;
}

void Adapter::BaseService::start() {
	libecap::adapter::Service::start();

    // LATER achar um lugar melhor...
    static queue_t inQueue = QUEUE_INITIALIZER(this->job_queue_buffer);
    static queue_t outQueue = QUEUE_INITIALIZER(this->answer_queue_buffer);
    this->job_queue = &inQueue;
    this->answer_queue = &outQueue;

}

void Adapter::BaseService::suspend(timeval &timeout) {

    if (queue_size(this->answer_queue) == 0) {
		const int maxUsec = 300*1000; // 300 milisegundos
		if (timeout.tv_sec > 0 || timeout.tv_usec > maxUsec) {
			timeout.tv_sec = 0;
			timeout.tv_usec = maxUsec;
		}
		return;
    }

    // temos uma resposta, sem timeout!
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
}

void Adapter::BaseService::resume() {
#if 0
	assert(WorkingXactions_);
    // o texto abaixo nao procedera...
	// We are running inside the transaction thread so we cannot call the host
	// application now. We must wait for the host to call our Service::resume.
	// XXX: push_back creates a copy of x, which is not thread-safe
	WaitingXactions_.push_back(x);
#endif
}


void Adapter::BaseService::stop() {
	libecap::adapter::Service::stop();
}

void Adapter::BaseService::retire() {
    stop();
}

bool Adapter::BaseService::wantsUrl(const char *url) const {
	return true; // no-op is applied to all messages
}

void Adapter::BaseService::adaptBegin(void **internal) const
{
    struct pyin_adapt_text at = {
        pyModule, NULL,
        NULL, 0,
        NULL, 0
    };
    pyin_adapt_begin(&at, pyController,
                     uri().c_str(), request_service());
    if (at.state && internal)
        *internal = at.state;
}

void Adapter::BaseService::adaptEnd(void **internal,
                                    std::string &output) const
{
    struct pyin_adapt_text at = {
        pyModule, (PyObject *) *internal,
        NULL, 0,
        NULL, 0
    };
    if (pyin_adapt_end(&at)) {
        *internal = at.state;
        return ;
    }

    if (at.ab != NULL) {
        output.append(at.ab, at.ab_len);
    }

    free(at.ab);
    *internal = at.state;
}

bool Adapter::BaseService::adaptHeader(void *internal,
                                       const std::string &name,
                                       const libecap::Area &value,
                                       std::string &output) const
{
    struct pyin_adapt_text at = {
        pyModule, (PyObject *) internal,
        value.start, value.size,
        NULL, 0
    };
    int ret_code = pyin_adapt_header(&at, name.c_str());
    if (ret_code > 0) {
        if (at.ab) {
            output.append(at.ab, at.ab_len);
            free(at.ab);
        }
        return true;
    }

    return false;
}

void Adapter::BaseService::adaptBody(void *internal,
                                     const libecap::Area &vb,
                                     std::string &output) const
{
    struct pyin_adapt_text at = {
        pyModule, (PyObject *) internal,
        vb.start, vb.size,
        NULL, 0
    };

    if (pyin_adapt_body(&at)) {
        output.append(vb.start, vb.size);
        return ;
    }

    if (at.ab != NULL) {
        output.append(at.ab, at.ab_len);
    } else {
        output.append(vb.start, vb.size);
    }

    free(at.ab);
}
