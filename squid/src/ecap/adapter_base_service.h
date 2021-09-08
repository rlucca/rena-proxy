#pragma once

#include "adapter_python_interface.h"

#include "adapter_base_xaction.h"

#include "queue.h"

namespace Adapter { // not required, but adds clarity

class BaseService: public libecap::adapter::Service {
	public:
		virtual ~BaseService()
        { retire(); }

		// About
		virtual std::string uri() const
		{ return "ecap://invalid/uri"; }

		virtual std::string tag() const; // changes with version and config
		virtual void describe(std::ostream &os) const; // free-format info
        virtual bool makesAsyncXactions() const { return true; } // needs suspend/resume

		// Configuration
		virtual void configure(const libecap::Options &cfg);
		virtual void reconfigure(const libecap::Options &cfg);
		virtual void setOne(const libecap::Name &name, const libecap::Area &valArea);

		// Lifecycle
		virtual void start(); // expect makeXaction() calls
        virtual void suspend(timeval &timeout); // influence host waiting time
        virtual void resume(); // kick async xactions via host::Xaction::resume
		virtual void stop(); // no more makeXaction() calls until start()
		virtual void retire(); // no more makeXaction() calls

		// Scope?
		virtual bool wantsUrl(const char *url) const;

		// Work
		virtual MadeXactionPointer
		makeXaction(libecap::host::Xaction *hostx) {
			return MadeXactionPointer(
				new Adapter::Xaction(std::tr1::static_pointer_cast<BaseService>(self), hostx));
		}
 
		virtual bool
		request_service() const {
			return false;
		}

		virtual bool
		reply_service() const {
			return false;
		}

        // Python Interface Adapter
        void adaptBegin(void **internal) const;
        bool adaptHeader(void *internal, const std::string &name,
                         const libecap::Area &value,
                         std::string &output) const;
        void adaptBody(void *internal, const libecap::Area &vb,
                       std::string &output) const;
        void adaptEnd(void **internal, std::string &output) const;

	public:
		// Configuration storage
		std::string suffix; // suffix to append...
        /* job queue buffer (to help on initialize) */
        static void* job_queue_buffer[256];
        /* job queue */
        queue_t* job_queue; // XXX
        /* answer queue buffer (to help on initialize) */
        static void* answer_queue_buffer[256];
        /* answer queue */
        queue_t* answer_queue;  // XXX
        //// alive Xaction objects in makeXaction() order
        ////std::list<XactionPointer> Xactions; // XXX

	protected:
		// Configuration storage
		std::string module;
		std::string pypath;
        PyObject *pyModule; // python module used
        PyObject *pyController; // proxy controller class

		void setModule(const std::string &value);
		void setPythonPath(const std::string &value);
		void setSuffix(const std::string &value);


};

} // namespace Adapter
