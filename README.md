# BLOOM - BLoom filter based Oblivious Outsourced Matchings

## About

Whole genome sequencing has become fast, accurate, and cheap, paving the way towards the large-scale collection and processing of human genome data. Unfortunately, this dawning genome era does not only promise tremendous advances in biomedical research but also causes unprecedented privacy risks for the many. Handling storage and processing of large genome datasets through cloud services greatly aggravates these concerns.

BLOOM [4] is a client-server system that allows a researcher (client) with constrained processing or storage resources to *securely outsource* genomic matchings, e.g., for disease testing, to an untrusted computation cloud (server).
All sensitive genomic data is encrypted using (fully) homomorphic encryption before it is transferred to the cloud.
All computations in the cloud are performed *obliviously in an encrypted domain* such that the untrusted cloud learns nothing about the data or the outcome of the analysis.
The final result can only be decrypted by the data owner.

We provide two different implementations of BLOOM with differing performance and security guarantees.
The first, FHEBLOOM, has been developed in the context as the solution of the COMSYS [0] team at RWTH Aachen University to Track 3 of the 2016 iDASH Secure Genome Analysis competition [1-3].
It is based on fully homomorphic encryption and provides full security guarantees.
FHEBLOOM was ranked runner-up in the competition which judged submissions by their processing, memory and communication overheads.
Our second approach, PHEBLOOM, is based on partially homomorphic encryption (i.e., the Paillier cryptosystem) and improves performance by at least three orders of magnitude through a slight relaxation of security guarantees.
A detailed explanation and comparison of both approaches is given in [4].

## Contact

* mail: ziegeldorf (at) comsys (dot) rwth-aachen (dot) de
* web: https://www.comsys.rwth-aachen.de/home/

## Disclaimer

The provided code represents a proof-of-concept prototype and it is not suitable for production use nor is it in any way cleared or certified for use in medical contexts.

---

## Installation

First clone the repository.
```
  git clone git://github.com/comsys/bloom.git
```

FHEBLOOM and PHEBLOOM require different steps for configuration and installation as detailed in the following.

### FHEBLOOM

1. Install required dependencies
```
sudo pip install bitarray natsort pybloom_live
sudo apt-get install cmake gcc-6 g++-6 libboost-all-dev
```
1. Build the binaries. In `bloom/FHEBLOOM` execute
```
cmake ./
make
```
1. You're all set.


### PHEBLOOM

1. Install the required dependencies
```
sudo pip install bitarray gmpy gensafeprime msgpack natsort pybloom_live
```

1. You're all set.

---


## Usage

In FHEBLOOM, all major steps of the whole pipeline can be called separately and intermediate results are written to the disk (default directory: /tmp/).
This was done to simplify the benchmarking of those steps for the iDASH competition.
In PHEBLOOM, it suffices to start client and server. Afterwards, all steps, i.e., preprocessing, encryption, upload, matching, download, and decryption run automatically using standard configurations.

### Input Data

The repository includes sample data provided by the organizers of the iDASH competition at [1].

If you want to add your own patient database, put each patient's SNPs into a single VCF file and put all VCF files into a single folder then point the PHEBLOOM client / FHEBLOOM bloomfiltering script there (using the --db option).

To add custom queries, put the desired SNPs into a VCF file and point the PHEBLOOM client / FHEBLOOM bloomfiltering script there (using the --qry option).

### FHEBLOOM

1. Preprocess database and query (run with -h / --help for all options):
```
python2 FHEBLOOM/bloomfiltering/bloomfiltering_database.py
    # (optional: --db <path to VCF input files DIRECTORY>)
    # (optional: --out <output folder for generated Bloom filters>)
python2 FHEBLOOM/bloomfiltering/bloomfiltering_query.py
    # (optional: --qry <path to query input FILE>)
    # (optional: --out <output folder for generated Bloom filter>)
```

1. Generate and upload keys (run with --help for all options):
```
FHEBLOOM/fhebloom_client --generate_key
FHEBLOOM/fhebloom_client --upload_key
```

1. Encrypt and upload the database:
```
FHEBLOOM/fhebloom_client --db_bloom
    # (optional: <output directory from FHEBLOOM/bloomfiltering/bloomfiltering_database.py>)
FHEBLOOM/fhebloom_client --upload_db
```

1. Encrypt and upload the query:
```
FHEBLOOM/fhebloom_client --qry_bloom
    # (optional: <output directory from FHEBLOOM/bloomfiltering/bloomfiltering_query.py>)
FHEBLOOM/fhebloom_client --upload_qry
```

1. Compute the matching (run with --help for all options):
```
FHEBLOOM/fhebloom_server --run
```

1. Download and decrypt the result:
```
FHEBLOOM/fhebloom_client --download
FHEBLOOM/fhebloom_client --decrypt
```

### PHEBLOOM

1. Start the client (run with -h / --help for all options):
```
python2 PHEBLOOM/phebloom_client.py
    # (optional: --db <path to VCF files DIRECTORY> --qry <query FILE PATH>)
```
1. Start the server (run with -h / --help for all options):
```
python2 PHEBLOOM/phebloom_server.py
```

---

## References

    [0] Communication and Distributed Systems (COMSYS), RWTH Aachen University, Germany,
        https://www.comsys.rwth-aachen.de/home/, 2017.
    [1] iDASH Secure Genome Analysis Competition,
        http://www.humangenomeprivacy.org/2016/competition-tasks.html, 2016.
    [2] Jiang, Xiaoqian, et al. "A community assessment of privacy preserving techniques for
        human genomes." BMC Medical Informatics and Decision Making 14.Suppl 1 (2014): S1.
    [3] Tang, Haixu, et al. "Protecting genomic data analytics in the cloud: state of the art
        and opportunities." BMC Medical Genomics 9.1 (2016): 63.
    [4] Ziegeldorf, Jan Henrik, et al. "BLOOM: BLoom-filter-based Oblivious Outsourced Matchings." -
        to be published in BMC Medical Genomics (2017).
