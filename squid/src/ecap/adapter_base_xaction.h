#pragma once

#include <libecap/common/autoconf.h>
#include <libecap/common/registry.h>
#include <libecap/common/errors.h>
#include <libecap/common/message.h>
#include <libecap/common/header.h>
#include <libecap/common/names.h>
#include <libecap/common/named_values.h>
#include <libecap/host/host.h>
#include <libecap/adapter/service.h>
#include <libecap/adapter/xaction.h>
#include <libecap/host/xaction.h>

namespace Adapter { // not required, but adds clarity

using libecap::size_type;
class BaseService; // forward

class Xaction: public libecap::adapter::Xaction {
	public:
		Xaction(libecap::shared_ptr<BaseService> s, libecap::host::Xaction *x);
		virtual ~Xaction();

		// meta-information for the host transaction
		virtual const libecap::Area option(const libecap::Name &name) const;
		virtual void visitEachOption(libecap::NamedValueVisitor &visitor) const;

		// lifecycle
		virtual void start();
		virtual void stop();

		// adapted body transmission control
		virtual void abDiscard();
		virtual void abMake();
		virtual void abMakeMore();
		virtual void abStopMaking();

		// adapted body content extraction and consumption
		virtual libecap::Area abContent(size_type offset, size_type size);
		virtual void abContentShift(size_type size);

		// virgin body state notification
		virtual void noteVbContentDone(bool atEnd);
		virtual void noteVbContentAvailable();

		void partialContent(const unsigned char *key, const unsigned char *value);

	protected:
		void notifyAdaptContent(bool at_end, bool is_final=false) const ; 
		void incrementalContent(std::string &chunk); // converts vb to ab
		bool adaptPartialContent(std::string &chunk, bool &ignore) const; // converts vb to ab

		void stopVb(); // stops receiving vb (if we are receiving it)
		libecap::host::Xaction *lastHostCall(); // clears hostx

	private:
		libecap::shared_ptr<const BaseService> service; // configuration access
		libecap::host::Xaction *hostx; // Host transaction rep

		std::string buffer; // for content adaptation
        void *internal; // python dict for internal state of parser

		typedef enum { opUndecided, opOn, opComplete, opNever } OperationState;
		OperationState receivingVb;
		OperationState sendingAb;
};

}
