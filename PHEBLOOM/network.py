"""
 File       network.py
 Author     Jan Henrik Ziegeldorf (ziegeldorf (at) comsys.rwth-aachen.de)
 Brief      TCP Client and Server with custom serialization
 
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

import socket
import time
import util

class TCPClient():    
    def __init__(self, address, port):           
        print "Connecting to: " + address + ":" + str(port)
        while True:
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((address, port))
                break
            except socket.error:
                print "Server not serving yet, sleeping ..."
                time.sleep(1.5)
                            
        print "Connected to server."
    
    def send(self, msg, count=True):
        return util.send_encode(self.sock, msg, count)        
    def recv(self, count=True):
        return util.recv_decode(self.sock, count)
        
class TCPServer():    
    def __init__(self, address, port):            
        print "Listening on: " + address + ":" + str(port)
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)                
        self.server_socket.bind((address, port))
        self.server_socket.listen(5)        
        self.sock, _ = self.server_socket.accept()
        print "Client connected."
    def send(self, msg, count=True):
        return util.send_encode(self.sock, msg, count)        
    def recv(self, count=True):
        return util.recv_decode(self.sock, count)
