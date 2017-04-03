"""
 File       phebloom_client.py
 Author     Jan Henrik Ziegeldorf (ziegeldorf (at) comsys.rwth-aachen.de)
 Brief      Client class of PHEBLOOM approach.
 
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
import os
from network import TCPClient
import paillier
from util import Timer, printNetworkStatistics
from math import ceil, log
import multiprocessing
from functools import partial
from gmpy import mpz
from itertools import izip, islice
import sys
from pybloom_live import BloomFilter
import natsort

def pack_and_enc(vals, HE, l):
    keylen = log(long(HE.pubkey['n']), 2)
    k = int(keylen/l)
    shift_factor = mpz(2**l)
    Res = []
    i = 0
    while (i < len(vals)):
        x = mpz(vals[i])
        for v in vals[i+1:i+k]:
            x = x*shift_factor
            x = x + v
        Res.append(HE.Enc(x))
        i += k
    return Res

class Task3Client:
    def __init__(self, peer, n, m, q, Cc, chunksize):
        self.peer = peer
        self.chunksize=chunksize
        self.n = n
        self.m = m
        self.Cc = Cc
        self.q = q
        self.l = int(ceil(-1*self.m*log(q) / (log(2)**2))) # lenght of the Bloomfilter
        print "DEBUG: Bloom filter length should be ~=", self.l
        
        # Max query length
        self.Q_size = 4
        
        self.benchmarks = {}

    def setup_keys(self, key_length=1024):
        with Timer(logstring="    Key generation") as t:
            self.p = paillier.Paillier(key_length=1024)
        self.benchmarks['c_key_generate'] = (t.secs,0,t.mem)
        
        with Timer(logstring="    Key upload") as t:
            snd_bytes = self.peer.send(self.p.pubkey)
        self.benchmarks['c_key_upload'] = (t.secs,snd_bytes,t.mem)

    def create_DB(self, path_DB):
        db = []
        for j,filepath in enumerate(natsort.natsorted(os.listdir(path_DB))[:self.n]):
            dbBloom = BloomFilter(self.m, self.q)
            self.k = dbBloom.num_slices 
            f = open('{}/{}'.format(path_DB, filepath), 'r')
            print "    DEBUG: Reading patient file {:3d} from: {}/{}".format(j, path_DB, filepath)
            for i,line in enumerate(f):
                # '#'-lines might result in too few added snps
                if not line.startswith('#'):
                    snp = line.strip().split('\t')
                    try:
                        dbBloom.add(snp[0] + snp[1] + snp[3] + snp[4])
                    except:
                        pass
                if i+1 >= self.m:
                    break
            
            db.append(dbBloom.bitarray)
            f.close()
            
        # Update n (needs to be done since len(db) could be smaller than specified n)
        self.n = len(db)
        
        # Reset Bloom filter length (the used library is slightly above the theoretical optimum)
        self.l = len(db[0])
        
        return db

    def create_query(self, path_QRY):
        queryBloom = BloomFilter(self.m, self.q)
        f = open(path_QRY, 'r')
        for qrySNP in f:
            if not qrySNP.startswith('#'):
                snp = qrySNP.strip().split('\t')
                try:
                    queryBloom.add(snp[0] + snp[1] + snp[3] + snp[4])
                except:
                    pass
        return 0, queryBloom.bitarray 

    def enryptandsendDB(self, bfs_DB):
        # this is the number of bits we need to reserve in each slot to avoid overflows
        self.bl = int(ceil(log(self.Q_size*self.k,2)))
        keylen = log(long(self.p.pubkey['n']), 2)
        s = int(keylen/self.bl)
        p = int(ceil(float(self.n) / s))
        
        print "    Packing {}x{} patient DB into {} packs of {} CTs (each CT packs max {} entries)".format(self.n, self.l, self.l, p, s)
         
        pool = multiprocessing.Pool(self.Cc)
        packit = partial(pack_and_enc, HE=self.p, l=self.bl)
        
        if self.chunksize == 0:
            self.chunksize = bfs_DB[0].length()
        nchunks = int(ceil(float(bfs_DB[0].length()) / self.chunksize))                    
        
        # Tell server how many chunks to expect
        self.peer.send(nchunks)
        self.benchmarks['c_db_encrypt'] = [0,0,0]
        self.benchmarks['c_db_upload'] = [0,0,0]
        for nchunk in range(nchunks):
            # Encrypt DB chunk
            with Timer(logstring="    Encrypted DB chunk {}/{}".format(nchunk+1, nchunks)) as t:
                enc_bfs_DB = pool.map(packit, islice(izip(*bfs_DB), nchunk*self.chunksize, (nchunk+1)*self.chunksize))
            self.benchmarks['c_db_encrypt'][0] += t.secs
            self.benchmarks['c_db_encrypt'][2] = t.mem
            
            # Measure total size in memory
            db_chunk_size = 0
            for col in enc_bfs_DB:
                for e in col:
                    db_chunk_size += sys.getsizeof(e)
                    db_chunk_size += 1024 / 8.
                db_chunk_size += sys.getsizeof(col)
            db_chunk_size += sys.getsizeof(enc_bfs_DB)
            self.benchmarks['c_db_encrypt'][1] += db_chunk_size
            
            # Send to server
            with Timer(logstring="    Sent DB chunk {}".format(nchunk)) as t:
                snd_bytes = self.peer.send(enc_bfs_DB)
            self.benchmarks['c_db_upload'][0] += t.secs
            self.benchmarks['c_db_upload'][1] += snd_bytes
            self.benchmarks['c_db_upload'][2] = t.mem
            
            # Delete DB
            for l in enc_bfs_DB:
                del l[:]
            del enc_bfs_DB[:]
    
    def setup(self):
        self.setup_keys()
        
        with Timer(logstring="    DB Blooming") as t:
            self.bfs_DB = self.create_DB(args.db)
        self.benchmarks['c_db_blooming'] = (t.secs,0,t.mem)
        
        self.enryptandsendDB(self.bfs_DB)
 
    def query(self, i, blowup):
        
        with Timer(logstring="    Query blooming") as t:
            total_qry_size, bfs_Q = self.create_query(args.qry)
        self.benchmarks["c_qry_blooming_{}".format(i)] = (t.secs,total_qry_size,t.mem)
        
        self.benchmarks["c_qry_encrypt_{}".format(i)] = (0,0,0)            
        with Timer(logstring="    Sent Query") as t:
            snd_bytes = self.peer.send(bfs_Q)
        self.benchmarks["c_qry_upload_{}".format(i)] = (t.secs, snd_bytes, t.mem)
                    
        with Timer(logstring="    Received Result"):
            Results = self.peer.recv()
        
        with Timer(logstring = "    Decrypting results") as t:
            results = map(int, self.p.Dec_unpack(Results, l=self.bl, max_cnt=self.n*blowup))
        self.benchmarks["c_qry_decrypt_{}".format(i)] = (t.secs, 0, t.mem)            
        
        # CONTROL (does not work with blowup)
        if args.CONTROL:
            if args.blowup > 1:
                print "WARNING: --CONTROL only works for --blowup 1"
            else:
                results_control = [0] * self.n
                for i,(col,bit) in enumerate(izip(izip(*self.bfs_DB), bfs_Q)):
                    for j,resbit in enumerate(col):
                        results_control[j] += resbit*int(bit)
                #print "CONTROL RESULT:  ", results_control        
                if not results == results_control:
                    print "    ERROR:"
                    print "      SECURE:", results
                    print "      CLEAR: ", results_control
                else:
                    print "    Sucessufully validated results. "
        
    def getBenchmarks(self):
        server_benchmarks = t3c.peer.recv()
        self.benchmarks.update(server_benchmarks)
        return self.benchmarks

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--address", default="127.0.0.1", help="address to connect to [default: localhost]")
    parser.add_argument("-p", "--port", default=8123, type=int, help="port to connect to [default: 8213]")
    parser.add_argument("-n", type=int, default=1, help="Number of patients n [default: 1]")
    parser.add_argument("-m", type=int, default=10000, help="Number of SNPs m per patient [default: 10000]")
    parser.add_argument("-q", type=int, default=-14, help="False positive probability 2**q [default: -14]")
    parser.add_argument("-Cc", type=int, default=None, help="Number of CPUs to use [default: all the CPUs!]")
    parser.add_argument("-r", type=int, default=0, help="Number of the current run [default: 0] (used to enumerate eval runs)")
    parser.add_argument("--blowup", type=int, default=1, help="Duplicate rows by this factor [default: 1] (This option can be used to produce synthetic large data sets on the server without transferring them from the client, e.g., to avoid large setup overheads during eval of online overheads)")
    parser.add_argument("--qrycnt", type=int, default=3, help="Number of queries repetitions to send [default: 3]")
    parser.add_argument("--db", type=str, default="data/vcf_database", help="Path to the directory that contains the VCF patient files [default: data/vcf_database]")
    parser.add_argument("--qry", type=str, default="data/vcf_query/multi_query.vcf", help="Path to the VCF file that contains query SNPs [default: data/vcf_query/multi_query.vcf]")
    parser.add_argument("-chunksize", type=int, default=0, help="Process DB in chunks of this size [default: 0 (infinity)]")
    parser.add_argument("--CONTROL", action="store_true", help="Validate results. DISABLE DURING EVAL! [default: False]")
    args = parser.parse_args()

    if not args.CONTROL:
        print "WARNING: Eval mode - not checking correctness of results"

    if not args.Cc:
        args.Cc = multiprocessing.cpu_count()
    print "Using {} CPUs".format(args.Cc)

    with Timer(logstring="Init client"):
        peer = TCPClient(args.address, args.port)
        t3c = Task3Client(peer, args.n, args.m, 2**args.q, args.Cc, args.chunksize)
    print
    
    print "Preprocessing"
    with Timer(logstring="Setup total time") as t:
        t3c.setup()
    print
    
    for i in range(args.qrycnt):
        print "Query", i
        with Timer(logstring="    Total".format(i)) as t:
            t3c.query(i, args.blowup)
        print
        t3c.benchmarks['c_qry_total'] = (t.secs, 0, 0)
    
    if not args.CONTROL:
        benchmarks = t3c.getBenchmarks()
        if args.blowup == 1:
            fp = "III_m{m}_n{n}_q{q}_Cc{Cc}_Cs4.r{r}".format(m=args.m, n=args.n, q=args.q, Cc=args.Cc, r=args.r)
        else:
            fp = "III_m{m}_n{n}_b{b}_q{q}_Cc{Cc}_Cs4.r{r}".format(m=args.m, n=args.n, b=args.blowup, q=args.q, Cc=args.Cc, r=args.r)
        
        with open(fp, "w") as f:
            f.write("c_db_blooming,{},{},{}\n".format(*(benchmarks['c_db_blooming'])))
            f.write("c_db_encrypt,{},{},{}\n".format(*(benchmarks['c_db_encrypt'])))
            f.write("c_db_upload,{},{},{}\n".format(*(benchmarks['c_db_upload'])))
            for i in range(args.qrycnt):
                for key in ["c_qry_blooming", "c_qry_encrypt", "c_qry_upload", "s_execute", "c_qry_download", "c_qry_decrypt"]:
                    f.write("{}_{},{},{},{}\n".format(key, i,*(benchmarks["{}_{}".format(key,i)])))
        
        printNetworkStatistics()
