"""
 File       bloomfiltering_query.py
 Author     Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
 Brief      Query preprocessing class of FHEBLOOM approach.
 
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

import argparse, os, sys, shutil
import bloomfiltering_config as BloomConfig

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", type=int, default=BloomConfig.m, help="Number of SNPs m per patient [default: {}]".format(BloomConfig.m))
    parser.add_argument("-q", type=int, default=BloomConfig.error_prob, help="False positive probability q [default: {}]".format(BloomConfig.error_prob))
    parser.add_argument("--qry", type=str, default=BloomConfig.vcf_qry, help="Path to the VCF file that contains query SNPs [default: {}]".format(BloomConfig.vcf_qry))
    parser.add_argument("--out", type=str, default=BloomConfig.outpath_qry, help="Path to the output directory where the databse Bloom filters are stored [default: {}]".format(BloomConfig.outpath_qry))
    args = parser.parse_args()

    # Clean
    shutil.rmtree(args.out, ignore_errors=True)
    os.makedirs(args.out)

    qryFile = os.path.join(args.out, 'query_{}'.format(args.m))
    BloomConfig.writeToFile(qryFile, args.qry, args.m, args.q)

