#include "adapter_header_visitor.h"


HeaderVisitor::HeaderVisitor(shared_base_service &aSvc,
                      void *internal): svc(aSvc), state(internal)
{
    cheader.clear();
    aheader.clear();
}


void HeaderVisitor::visit(const libecap::Name &name,
                          const libecap::Area &value)
{
    const std::string &name_header = name.image();
    std::string ret;
    if (svc->adaptHeader(state, name_header, value, ret))
    {
        mmap_iter search = cheader.find(name_header);
        mmap_iter end = cheader.end();
        if (search == end) {
            search = aheader.find(name_header);
            end = aheader.end();

            if (search != end) {
                end = search;
                while (end != aheader.end()
                        && end->first == name_header) {
                    end++;
                }

                cheader.insert(search, end);
            }
        }

        cheader.insert(
            std::make_pair(name_header, ret)
        );

        aheader.insert(
            std::make_pair(name_header, ret)
        );

        return ;
    }

    // quando nao modificamos o header, devemos ir na
    // lista de modificados e validar que o mesmo ja
    // nao esta lah cadastrado. No caso de estar, a
    // insercao tb eh feita nessa lista. Isso garante
    // que caso tenhamos headers sem modificacao depois
    // de um header com modificado foi feito. O apply
    // que roda depois ira apagar todos headers com nome
    // igual ao modificado para posteriormente reinseri-los
    mmap_iter search = cheader.find(name_header);
    mmap_iter end = cheader.end();
    if (search != end) {
        cheader.insert(
            std::make_pair(name_header, value.start)
        );
    }

    aheader.insert(
        std::make_pair(name_header, value.start)
    );
}

void HeaderVisitor::applyChanges(libecap::Header &header)
{
    mmap_iter it;
    
    // Removeremos todos os headers primeiro
    for (it = cheader.begin(); it != cheader.end(); it++) {
        libecap::Name hdr(it->first);
        header.removeAny(hdr);
    }

    // Vamos reinserir os headers agora...
    for (it = cheader.begin(); it != cheader.end(); it++) {
        libecap::Name hdr(it->first);
        if (!it->second.empty()) {
            header.add(hdr,
                   libecap::Area::FromTempString(it->second));
        }
    }
}
