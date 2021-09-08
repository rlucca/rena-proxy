#include "adapter_base_service.h"
#include "adapter_header_visitor.h"

static const libecap::Name headerHost("Host");
static const libecap::Name headerContentType("Content-Type");
using libecap::headerReferer;

Adapter::Xaction::Xaction(libecap::shared_ptr<BaseService> aService,
	libecap::host::Xaction *x):
	service(aService),
	hostx(x), buffer(""), internal(NULL),
	receivingVb(opUndecided), sendingAb(opUndecided)
{ /* EMPTY */ }

Adapter::Xaction::~Xaction() {
	if (libecap::host::Xaction *x = hostx) {
		hostx = 0;
		x->adaptationAborted();
	}
}

const libecap::Area Adapter::Xaction::option(const libecap::Name &) const {
	return libecap::Area(); // this transaction has no meta-information
}

void Adapter::Xaction::visitEachOption(libecap::NamedValueVisitor &) const {
	// this transaction has no meta-information to pass to the visitor
}

void Adapter::Xaction::start() {
	Must(hostx);

    service->adaptBegin(&internal);

	if (hostx->virgin().body()) {
		receivingVb = opOn;
		hostx->vbMake(); // ask host to supply virgin body
	} else {
		// we are not interested in vb if there is not one
		receivingVb = opNever;
	}

	/* adapt message header */

	libecap::shared_ptr<libecap::Message> adapted
                                        = hostx->virgin().clone();
	Must(adapted != 0);
    libecap::Header &hdr = adapted->header();

	HeaderVisitor hv(service, internal);
	hdr.visitEach(hv);
    hv.applyChanges(hdr);

	// add a custom header
	static const libecap::Name name("X-Ecap");
	const libecap::Header::Value value =
		libecap::Area::FromTempString(libecap::MyHost().uri());
	hdr.add(name, value);

	if (!adapted->body()) {
		sendingAb = opNever; // there is nothing to send
		lastHostCall()->useAdapted(adapted);
	} else {
		hostx->useAdapted(adapted);
	}
}

void Adapter::Xaction::stop() {
	hostx = 0;
	// the caller will delete
}

void Adapter::Xaction::abDiscard()
{
	Must(sendingAb == opUndecided); // have not started yet
	sendingAb = opNever;
	// we do not need more vb if the host is not interested in ab
	stopVb();
}

void Adapter::Xaction::abMake()
{
	// have not yet started or decided not to send
	Must(sendingAb == opUndecided);
    // that is our only source of ab content
	Must(hostx->virgin().body());

	// we are or were receiving vb
	Must(receivingVb == opOn || receivingVb == opComplete);
	
	sendingAb = opOn;
	if (!buffer.empty())
		hostx->noteAbContentAvailable();
}

void Adapter::Xaction::abMakeMore()
{
	// a precondition for receiving more vb
	Must(receivingVb == opOn);
	hostx->vbMakeMore();
}

void Adapter::Xaction::abStopMaking()
{
	sendingAb = opComplete;
	// we do not need more vb if the host is not interested in more ab
	stopVb();
}


libecap::Area Adapter::Xaction::abContent(size_type offset,
                                          size_type size)
{
	Must(sendingAb == opOn || sendingAb == opComplete);
	return libecap::Area::FromTempString(buffer.substr(offset, size));
}

void Adapter::Xaction::abContentShift(size_type size) {
	Must(sendingAb == opOn || sendingAb == opComplete);
	buffer.erase(0, size);
}

void Adapter::Xaction::noteVbContentDone(bool atEnd)
{
	Must(receivingVb == opOn);

	if (sendingAb == opOn) {
        service->adaptEnd(&internal, buffer);

		notifyAdaptContent(atEnd, true);
		sendingAb = opComplete;
	}

	stopVb();
}

void Adapter::Xaction::notifyAdaptContent(bool at_end,
                                          bool is_final) const
{
	if (sendingAb == opOn) {
		if (is_final==false)
			hostx->noteAbContentAvailable();
		else
			hostx->noteAbContentDone(at_end);

	}
}

void Adapter::Xaction::noteVbContentAvailable()
{
	Must(receivingVb == opOn);

    // requisitamos toda requisicao...
	const libecap::Area vb = hostx->vbContent(0, libecap::nsize);
    service->adaptBody(internal, vb, buffer);
    // nao precisamos mais do vb, entao...
    hostx->vbContentShift(vb.size);

	notifyAdaptContent(false, false);
}

// tells the host that we are not interested in [more] vb
// if the host does not know that already
void Adapter::Xaction::stopVb() {
	if (receivingVb == opOn) {
		hostx->vbStopMaking(); // we will not call vbContent() any more
		receivingVb = opComplete;
	} else {
		// we already got the entire body or refused it earlier
		Must(receivingVb != opUndecided);
	}
}

// this method is used to make the last call to hostx transaction
// last call may delete adapter transaction if the host no longer needs it
// TODO: replace with hostx-independent "done" method
libecap::host::Xaction *Adapter::Xaction::lastHostCall() {
	libecap::host::Xaction *x = hostx;
	Must(x);
	hostx = 0;
	return x;
}
