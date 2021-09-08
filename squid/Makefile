
# DESTDIR usa atribuicao fraca para poder sobrepor de fora...
DESTDIR:=
ADAPTER_PATH=src/ecap
LIBRARY=${ADAPTER_PATH}/ecap_adapter_service_modifier.so

SKEL_SPOOL=${DESTDIR}/var/spool/squid-squid1 \
	${DESTDIR}/var/spool/squid-squid2 \
	${DESTDIR}/var/spool/squid-squid3

SKEL_DIRECTORIES=${DESTDIR}/etc/squid/dbs \
	${DESTDIR}/etc/squid/templates \
	${DESTDIR}/usr/local/lib \
	${SKEL_SPOOL}


.PHONY: library clean skel install fast_tests slow_tests all_tests
library: skel
	@make -C ${ADAPTER_PATH} clean library
	@cp -vu ${LIBRARY} lib

clean:
	@-make -C ${ADAPTER_PATH} clean
	
skel:
	@-mkdir -p ${SKEL_DIRECTORIES}

install: skel
	@chmod 776 ${SKEL_SPOOL}
	@chown root:squid ${SKEL_SPOOL}
	# git pull --ff-only
	-for i in `ls -1 lib/*`; do cp -u $${PWD}/$${i} ${DESTDIR}/usr/local/lib ; done
	-for i in `find etc -maxdepth 1 -type f`; do cp -u $${PWD}/$${i} ${DESTDIR}/etc/squid ; done
	-for i in `find src/helpers/dbs -maxdepth 1 -type f`; do cp -u $${PWD}/$${i} ${DESTDIR}/etc/squid/dbs ; done
	-cp -u src/helpers/proxy_*.py ${DESTDIR}/etc/squid
	-cp -u src/helpers/url_rewriter.py ${DESTDIR}/etc/squid
	-cp -u src/helpers/ecap_logic_*.py ${DESTDIR}/etc/squid
	-cp -u src/helpers/libera*.py ${DESTDIR}/etc/squid
	-cp -u src/helpers/memcache*.py ${DESTDIR}/etc/squid
	-cp -u templates/* ${DESTDIR}/etc/squid/templates
	-cp -i etc/proxy_pass.txt ${DESTDIR}/etc/squid
	-chmod 640 ${DESTDIR}/etc/squid/proxy_pass.txt

fast_tests:
	-for i in `ls -1 tests/*_test.py`; do python -m unittest $${i} ; done

slow_tests:
	-python -m unittest tests.proxy_model_test.TestProxyModel.test_check_database
	-python -m unittest tests.url_rewriter_test.TestUrlRewriter.test_slow
	-python -m unittest tests.ecap_logic_test.TestEcapLogicReplyFromFile

all_tests: fast_tests slow_tests
