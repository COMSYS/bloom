"""
 File       bloomfiltering_config.py
 Author     Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
 Brief      Config class of FHEBLOOM approach.
 
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

from pybloom_live import BloomFilter
import sys, StringIO, cStringIO

# Error probability "q"
error_prob = 1/16384.

# Number of SNPs m per patient
m = 10000
# Number of patients n
n = 1

# Set vcf input database folder with a trailing '/'
vcf_db = 'data/vcf_database/'
# Set pattern of database vcf input files
vcf_db_pattern = '*_{}.vcf'.format(m)
# Set vcf input query
vcf_qry = 'data/vcf_query/multi_query.vcf'

# Set preprocessing database output path with a trailing '/'
outpath_db = 'data/bloom_database/'
# Set preprocessing query output path with a trailing '/'
outpath_qry = 'data/bloom_query/'

def writeToFile(outpath, path, size, error):
    bloom = BloomFilter(size, error)

    dest = open(outpath, 'w')
    
    f = open(path, 'r')
    for line in f:
        if not line.startswith('#'):
            snp = line.strip().split('\t')
            bloom.add(snp[0]+snp[1]+snp[3]+snp[4])
    f.close()
    
    bloom.bitarray.tofile(dest)
    dest.close()
