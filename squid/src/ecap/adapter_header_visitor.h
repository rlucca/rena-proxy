#pragma once

#include "adapter_base_service.h"
#include <map>

class HeaderVisitor: public libecap::NamedValueVisitor {
	public:
        typedef
        libecap::shared_ptr<const Adapter::BaseService>
        shared_base_service;

		HeaderVisitor(shared_base_service &aSvc, void *internal);

		virtual void visit(const libecap::Name &name,
                           const libecap::Area &value);

        void applyChanges(libecap::Header &header);

    protected:
        shared_base_service &svc;
        typedef std::multimap<std::string, std::string> mmap;
        typedef mmap::iterator mmap_iter;
        mmap cheader;
        mmap aheader;
        void *state;
};
