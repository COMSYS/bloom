"""
 File       util.py
 Author     Jan Henrik Ziegeldorf (ziegeldorf (at) comsys.rwth-aachen.de)
 Brief      Benchmark, network, and serialization utilities
 
 Copyright  BLOOM: Bloom filter based outsourced oblivious matchings
            Copyright (C) 2017 Communication and Distributed Systems (COMSYS), RWTH Aachen
            
            This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU Affero General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.
            
            This program is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU Affero General Public License for more details.
            You should have received a copy of the GNU Affero General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.        
"""

import msgpack
import struct
from gmpy import mpz, binary
import time
import sys
import resource
from bitarray import bitarray
           
def checkMemory(verbose=True):
    rusage_denom = 1.0
    if sys.platform == 'darwin':
        # ... it seems that in OSX the output is different units ...
        rusage_denom = 1000.0
    mem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / rusage_denom
    return mem
    #print "    Current memory usage: {} MB".format(mem)

class Timer(object):
    def __init__(self, logstring=None, verbose=False):
        self.verbose = verbose
        self.logstring = logstring

    def __enter__(self):
        self.start = time.time()
        return self

    def __exit__(self, *args):
        self.end = time.time()
        self.secs = self.end - self.start
        self.msecs = self.secs * 1000  # millisecs
        self.mem = checkMemory()        
        if self.logstring:
            print "%25s %10.4f ms    %10.4f MB" % ((self.logstring + ":").ljust(35), self.msecs, self.mem / 1000.)
        else:
            print 'elapsed time: %f ms, memory %f MB'.format(self.msecs, self.mem)

sent_bytes = 0
received_bytes = 0
sent_msgs = 0
recv_msgs = 0

def recv_decode(sock, count):
    global recv_msgs
    global received_bytes
    recv_buffer_size = 4096  
    size_data=sock.recv(4)
    while len(size_data) < 4:
        size_data += sock.recv(4)
    msg_size = int(struct.unpack('>i', size_data)[0])
    if count:
        received_bytes += 4 + msg_size
        recv_msgs += 1
    data = ''
    while msg_size > 0:
        sock_data = sock.recv(min(recv_buffer_size, msg_size))
        msg_size -= len(sock_data)
        data += sock_data            
    return msgpack.unpackb(data, object_hook=decode)

def send_encode(sock, msg, count):
    global sent_bytes
    global sent_msgs    
    data = msgpack.packb(msg, default=encode)
    if count:
        sent_bytes += 4 + len(data)
        sent_msgs += 1
    return sockSend(sock, struct.pack('>i', len(data))+data)

def sockSend(sock, data):
    totalsent = 0
    while totalsent < len(data):
        sent = sock.send(data[totalsent:])
        if sent == 0:
            raise RuntimeError("socket connection broken")
        totalsent = totalsent + sent
    return len(data)
    
def printNetworkStatistics():
    global received_bytes
    global sent_bytes
    global sent_msgs
    global recv_msgs
    print "Sent     %i messages and %.2f MB" % (sent_msgs, sent_bytes/10.**6.)
    print "Received %i messages and %.2f MB" % (recv_msgs, received_bytes/10.**6)
    print "Total    %i messages and %.2f MB" % (recv_msgs+sent_msgs, (sent_bytes+received_bytes)/10.**6)

__x = mpz(1)
__ba = bitarray()

def encode(obj):
    if type(obj) == type(mpz(__x)):
        return {'b':binary(obj)}
    elif type(obj) == type(__ba):
        return {'ba':obj.tobytes()}
#    elif type(obj) == type(__bi):
#        return {'__bi__': True, 'b':obj.bytes}
    return obj

def decode(obj):
    if b'b' in obj:
        obj = mpz(obj['b'], 256)
    elif b'ba' in obj:
        obj = bitarray(obj['ba'])
    #elif b'__bi__' in obj:
    #    obj = Bits(bytes=obj['b'])
    return obj

