"""
 File       phebloom_server.py
 Author     Jan Henrik Ziegeldorf (ziegeldorf (at) comsys.rwth-aachen.de)
 Brief      Server class of PHEBLOOM approach.
 
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

import argparse
import paillier
from network import TCPServer
from util import Timer, printNetworkStatistics

class Task3Server:
    def __init__(self, peer):
        self.peer = peer        
        self.setup_keys()
        self.benchmarks = {}
        
    def setup_keys(self):
        pubkey = self.peer.recv()
        self.P = paillier.Paillier(pubkey=pubkey)

    def setup(self, blowup=1):
        self.enc_bfs_DB = []
        if blowup > 1:
            print "WARNING: Blowing up rows by factor", blowup 
        print "Receiving DB in chunks"
        with Timer(logstring="Receive DB"):            
            nchunks = self.peer.recv()
            for nchunk in range(nchunks):
                with Timer(logstring="    Received chunk {}/{}".format(nchunk+1, nchunks)):
                    tmp = self.peer.recv()
                    tmp = [list(col) for col in tmp]
                    for _ in range(1,blowup):
                        for col in tmp: 
                            col.append(col[0])
                    self.enc_bfs_DB += tmp
            
    def match_and_aggregate(self, bfs_Q):
        # Select columns
        Cols = []
        cnt = 0
        for i,bit in enumerate(bfs_Q):
            if bit:
                cnt +=1
                Cols.append(self.enc_bfs_DB[i])                
        
        # Add columns
        Results = list(Cols[0])
        for Col in Cols[1:]:
            for i,C in enumerate(Col):
                Results[i] = self.P.Add(Results[i], C) 
        return Results       

    def run(self, qrycnt):
        for i in range(qrycnt):
            print "Query", i
            bfs_Q = self.peer.recv()            
            with Timer(logstring="    Matched and Aggregated") as t:
                Results = self.match_and_aggregate(bfs_Q)
            self.benchmarks["s_execute_{}".format(i)] = (t.secs,0,t.mem)
            with Timer(logstring="    Sent results") as t:
                snd_bytes = self.peer.send(Results)
            del Results[:]
            self.benchmarks['c_qry_download_{}'.format(i)] = (t.secs,snd_bytes,0)
            print
        self.peer.send(self.benchmarks, False)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--address", default="127.0.0.1", help="address to listen to [default: localhost]")
    parser.add_argument("-p", "--port", default=8123, type=int, help="port to listen on [default: 8213]")
    parser.add_argument("--qrycnt", type=int, default=3, help="Expected number of query repetitions [default: 3]")
    parser.add_argument("--blowup", type=int, default=1, help="Duplicate rows by this factor [default: 1] (This option can be used to produce synthetic large data sets on the server without transferring them from the client, e.g., to avoid large setup overheads during eval of online overheads)")         
    
    args = parser.parse_args()
    
    with Timer(logstring="Init server:"):
        peer = TCPServer(args.address, args.port)
        t3s = Task3Server(peer)
    
    t3s.setup(args.blowup)    
    t3s.run(args.qrycnt)
    
    printNetworkStatistics()
