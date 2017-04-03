// File       fhebloom_config.h
// Authors    Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
//            David Hellmans (david.hellmanns (at) rwth-aachen.de)
//            Felix Schwinger (felix.schwinger (at) rwth-aachen.de)
// Brief      Config header file of FHEBLOOM approach.
// 
// Copyright  BLOOM: Bloom filter based outsourced oblivious matchings
//            Copyright (C) 2017 Communication and Distributed Systems (COMSYS), RWTH Aachen
//            
//            This program is free software: you can redistribute it and/or modify
//            it under the terms of the GNU Affero General Public License as published
//            by the Free Software Foundation, either version 3 of the License, or
//            (at your option) any later version.
//            
//            This program is distributed in the hope that it will be useful,
//            but WITHOUT ANY WARRANTY; without even the implied warranty of
//            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//            GNU Affero General Public License for more details.
//            You should have received a copy of the GNU Affero General Public License
//            along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <FHE.h>

namespace bloomLib 
{
    const static char *dir_client_db = "fhebloom_client_database/";
    const static char *dir_client_privKey = "fhebloom_client_private_key/";
    const static char *dir_client_pubKey = "fhebloom_client_public_key/";
    const static char *dir_client_qry = "fhebloom_client_query/";
    const static char *dir_client_res = "fhebloom_client_result/";

    const static char *dir_server_db = "fhebloom_server_database/";
    const static char *dir_server_pubKey = "fhebloom_server_public_key/";
    const static char *dir_server_qry = "fhebloom_server_query/";
    const static char *dir_server_res = "fhebloom_server_result/";

    //Key Settings
    long p = 59;          // Modulo
    long L = 3;           // Levels
    long security = 80;   // security bits
    
    Ctxt loadCtChunk(uint32_t chunkNo, const FHEPubKey &publicKey, string prefix);
    void storeCtResult(Ctxt ctChunk, const FHEPubKey& publicKey, string prefix);
    void execute(string name, string database_path, string query_path, string path_result, const FHEPubKey& publicKey, EncryptedArray ea);

    void removeFiles(string path, string filter);
    vector<string> enumerateFiles(string path, string filter);
    int getChunkCount(string path, string prefix);
}
