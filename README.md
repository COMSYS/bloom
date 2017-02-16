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
Our second approach, PHEBLOOM, is based on partially homomorphic encryption (i.e., the Paillier crypto system) and improves performance by at least three orders of magnitude through a slight relaxation of security guarantees.
A detailed explanation and comparison of both approaches is given in [4].


## Code

We are currently revising and documenting our source code.
It will be published in this repository as soon as possible.


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
