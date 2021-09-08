#!/bin/python3

from redis import Redis


class CacheAccess(object):

    def __init__(self, unix_socket, expiration):
        self.expireSeconds = 7200
        self.unix_socket = unix_socket
        self.client = Redis(unix_socket_path=unix_socket)

    def _key(self, ip_as_txt, myaddr_as_txt):
        return '{}@{}'.format(ip_as_txt, myaddr_as_txt)

    def _set(self, key):
        self.client.set(key, 'a', ex=self.expireSeconds)

    def cache_set(self, ip, myaddr):
        self._set(self._key(ip, myaddr))

    def cache_check(self, ip, myaddr):
        key = self._key(ip, myaddr)
        if self.client.get(key) is None:
            return False
        self._set(key)  # do a touch
        return True


