// File       fhebloom_client.cpp
// Authors    Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
//            David Hellmans (david.hellmanns (at) rwth-aachen.de)
//            Felix Schwinger (felix.schwinger (at) rwth-aachen.de)
// Brief      Client class file of FHEBLOOM approach.
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

#include <EncryptedArray.h>
#include <bitset>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <string>
#include <unistd.h>
#include "fhebloom_config.h"
#include "commandline.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace bl = bloomLib;

void generateKeys(long m, long p, long r, long L, long c, long w, long d, long security, string public_path, string private_path)
{
    m = FindM(security,L,c,p, d, 0, 0);

    FHEcontext context(m, p, r);
    // initialize context
    buildModChain(context, L, c);
    // modify the context, adding primes to the modulus chain
    FHESecKey secretKey(context);
    // construct a secret key structure
    const FHEPubKey& publicKey = secretKey;
    // an "upcast": FHESecKey is a subclass of FHEPubKey

    secretKey.GenSecKey(w);
    // actually generate a secret key with Hamming weight w

    addSome1DMatrices(secretKey);

    //write key to file
    {
        fstream contextFile(public_path + "helib_context.key", fstream::out|fstream::trunc);
        assert(contextFile.is_open());

        // Output the FHEcontext to file
        writeContextBase(contextFile, context);
        contextFile << context << endl;

        contextFile.close();
    }

    //write key to file
    {
        fstream secretFile(private_path + "helib_secret.key", fstream::out|fstream::trunc);
        assert(secretFile.is_open());

        secretFile << secretKey << endl;

        secretFile.close();
    }

    //write key to file
    {
        fstream publicFile(public_path + "helib_public.key", fstream::out|fstream::trunc);
        assert(publicFile.is_open());

        publicFile << publicKey << endl;

        publicFile.close();
    }
}

bool getBit(unsigned char byte, int position) // position in range 0-7
{
    return (byte >> position) & 0x1;
}


int loadBloomFile(string path, vector<bool> &bloom)
{

    ifstream bloomFile;
    bloomFile.open(path, ifstream::binary);
    int setBits = 0;

    //cout << "Open File " << path << endl;
    if(bloomFile.is_open())
    {
        //cout << "File is opened" << endl;

        bloomFile.seekg (0, bloomFile.end);
        int length = bloomFile.tellg();
        bloomFile.seekg (0, bloomFile.beg);

        int currentChar = 0;
        //Do NOT use eof for binary files
        while (currentChar < length)
        {
            // read returns a byte
            char b;

            bloomFile.read(&b, 1);
            // we use bits! Count down here!
            for(int i = 7; i >= 0; i--)
            {
                bool bit = getBit(b, i);
                bloom.push_back(bit);
                setBits += bit;
            }
            currentChar++;
        }
    }

//    cout << "Bloomfilter has size " << bloom.size() << endl;
//    cout << bloom << endl;
    return setBits;
}


vector<vector<long>> splitVector(vector<bool> bloom, int chunkSize)
{
    vector<vector<long>> bloomChunks;
    double chunkCount = static_cast<double>(bloom.size())/static_cast<double>(chunkSize);
    for (int i=0; i < ceil(chunkCount); i++)
    {
        vector<bool>::iterator startIterator = bloom.begin()+i*chunkSize;
        vector<bool>::iterator endIterator = bloom.begin()+(i+1)*chunkSize;
        if ((i+1)*chunkSize >= (int) bloom.size())
        {
            endIterator = bloom.end();

            vector<long> temp = vector<long>(startIterator, endIterator);
            while ((int) temp.size() < chunkSize)
            {
                temp.push_back(false);
            }
            bloomChunks.push_back(temp);

        }
        else
        {
            bloomChunks.push_back(vector<long>(startIterator, endIterator));
        }

    }

    //cout << bloomChunks.size() << " chunks with size " << chunkSize << endl;

    return bloomChunks;
}

void encryptBloomfilter(EncryptedArray& ea, const FHEPubKey& publicKey, vector<vector<long>>& cur_vector, string path, string prefix, int setBits)
{

    boost::replace_all(path, "//", "/");

    if (path.compare("") != 0)
        #pragma omp critical
        cout << "[CLT] >> Encrypting: " << path << endl;

    for(vector<vector<long>>::iterator it = cur_vector.begin(); it != cur_vector.end(); it++)
    {
        stringstream filename;
        filename << prefix << "_ct_chunk_" << distance(cur_vector.begin(), it) << ".enc";
        fstream ctChunkFile(filename.str(), fstream::out|fstream::trunc);
        Ctxt ctChunk(publicKey);
        ea.encrypt(ctChunk, publicKey, *it);
        ctChunkFile << ctChunk;
        ctChunkFile.close();
    }

    int p = prefix.find_last_of("/");

    if (path.compare("") != 0)
        #pragma omp critical
        cout << "[CLT] <<   Finished: " << prefix.substr(p+1) << ".enc - " << setBits << endl;
}

void create_dir(string path, string pattern)
{
    if(fs::exists(path))
        bl::removeFiles(path, pattern);
    else
        fs::create_directory(path);
}

void upload_rsync(string output, string client, string server, string host, string user, int port)
{
    cout << "[SRV] Uploading " << output << " to: "<< user << "@" << host << "[" << port << "]"  << endl;

    // from local path to user @ host:remote_folder
    send_files_commandline(host.c_str(), user.c_str(), port, client.c_str(), server.c_str());
}

void download_rsync(string client, string server, string host, string user, int port)
{
    cout << "[SRV] Downloading results from: "<< user << "@" << host << "[" <<  port << "]"  << endl;
    //cout << "[SRV] Remote Directory: "<< server << "\n[SRV] Local Directory: " << client << endl;

    // from user @ host:remote_folder into local path
    recv_files_commandline(host.c_str(), user.c_str(), port, client.c_str(), server.c_str());
}

int main(int argc, char* argv[])
{
    //HElib Settings
    long m=0;
    long r=1;
    long c=2;
    long w=64;
    long d=0;

    po::variables_map vm;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("generate_key", "generates a new key")

            ("db_bloom", po::value<string>()->implicit_value(fs::system_complete("data/bloom_database").string()), "Path to the directory that contains the preprocessed patient files [default: data/bloom_database/]")
            ("qry_bloom", po::value<string>()->implicit_value(fs::system_complete("data/bloom_query").string()), "Path to the directory that contains the preprocessed query file [default: data/bloom_query/]")
            ("dir_client", po::value<string>()->default_value("/tmp/"), "Path to the client processing directory [default: /tmp/]")
            ("dir_server", po::value<string>()->default_value("/tmp/"), "Path to the server processing directory [default: /tmp/]")

            ("upload_db", "Upload encrypted database to server")
            ("upload_key", "Upload public key to server")
            ("upload_qry", "Upload encrypted query to server")

            ("execute", "Perform calculation locally [DO NOT USE DURING EVAL!]")
            ("decrypt", "Decrypt server result")
            ("download", "Download encrypted results from server")
            
            ("a", po::value<string>()->default_value("127.0.0.1"), "address to connect to [default: localhost]")
            ("p", po::value<int>()->default_value(22), "port to connect to [default: 22]")
            ("u", po::value<string>()->default_value(string(getlogin())), "user to connect with [default: current]")

            ;

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << desc << "\n";
        return 1;
    }

    if (vm.count("generate_key"))
    {
        cout << "[CLT] Removing all old data" << endl;
        create_dir(vm["dir_client"].as<string>() + bl::dir_client_pubKey, ".*\\.key");
        create_dir(vm["dir_client"].as<string>() + bl::dir_client_privKey, ".*\\.key");
        generateKeys(m, bl::p, r, bl::L, c, w, d, bl::security, vm["dir_client"].as<string>() + bl::dir_client_pubKey, vm["dir_client"].as<string>() + bl::dir_client_privKey);
        cout << "[CLT] Generated new key files, please use --upload_key" << endl;
    }

    if(!(fs::exists(vm["dir_client"].as<string>() + string(bl::dir_client_pubKey) + "helib_context.key") || fs::exists(vm["dir_client"].as<string>() + string(bl::dir_client_privKey) + "helib_secret.key")))
    {
        cout << "[CLT] No key files found, please use --generate_key" << endl;
        return -1;
    }

    int nslots = 0;
    map<int, int> comparison;

        fstream contextFile(vm["dir_client"].as<string>() + string(bl::dir_client_pubKey) + "helib_context.key", fstream::in);
        // Read context from file
        unsigned long m1, p1, r1;
        vector<long> gens, ords;
        readContextBase(contextFile, m1, p1, r1, gens, ords);
        FHEcontext context(m1, p1, r1, gens, ords);
        contextFile >> context;

        fstream secretFile(vm["dir_client"].as<string>() + string(bl::dir_client_privKey) + "helib_secret.key", fstream::in);
        FHESecKey secretKey(context);
        const FHEPubKey &publicKey = secretKey;
        secretFile >> secretKey;

        ZZX G = context.alMod.getFactorsOverZZ()[0];

        EncryptedArray ea(context, G);

        nslots = ea.size();
        //cout << "[CLT] Slots: " << nslots << endl;


    /*
     * Step 1: Encrypt Database if necessary
     *          only if we generated a new key
     *
     */

    if(vm.count("db_bloom"))
    {
        create_dir(vm["dir_client"].as<string>() + bl::dir_client_db, ".*\\.enc");
        vector<string> dbFilenames = bl::enumerateFiles(vm["db_bloom"].as<string>() + '/', "database_.*");

        #pragma omp parallel for schedule(dynamic,1)
        for (int j = 0; j < (int) dbFilenames.size(); j++)
        {
            vector<bool> db;
            int setBits = loadBloomFile(vm["db_bloom"].as<string>() + '/' + dbFilenames.at(j), db);
            vector<vector<long>> db_vector;
            db_vector = splitVector(db, nslots);
            encryptBloomfilter(ea, publicKey, db_vector, vm["db_bloom"].as<string>() + '/' + dbFilenames.at(j), vm["dir_client"].as<string>() + string(bl::dir_client_db) + dbFilenames.at(j), setBits);
        }

        // Encrypt empty vector for chunk aggregation
        vector<vector<long>> emptyVector(1 , vector<long>(nslots, 0));
        encryptBloomfilter(ea, publicKey, emptyVector, "", vm["dir_client"].as<string>() + string(bl::dir_client_db) + "emptyvector", 0);

    }

    /*
     * Step 2a: Encrypt Query
     * Step 2b: Read Query into memory
     *
     */

    if(vm.count("qry_bloom"))
    {
        create_dir(vm["dir_client"].as<string>() + bl::dir_client_qry, "query_.*\\.enc");
        vector<string> qryFilenames = bl::enumerateFiles(vm["qry_bloom"].as<string>() + '/', "query_.*");

        #pragma omp parallel for schedule(dynamic,1)
        for (int j = 0; j < (int) qryFilenames.size(); j++)
        {
            vector<bool> query;
            int setBits = loadBloomFile(vm["qry_bloom"].as<string>() + '/' + qryFilenames.at(j), query);
            int tmpPos = qryFilenames.at(j).find("_");
            string queryname = qryFilenames.at(j).substr(qryFilenames.at(j).find("_", tmpPos+1)+1, qryFilenames.at(j).length());
            int currentBloom = stoi(queryname.substr(string("query_").length(), queryname.length()));
            comparison.emplace(currentBloom, setBits);

            vector<vector<long>> query_vector;
            query_vector = splitVector(query, nslots);
            encryptBloomfilter(ea, publicKey, query_vector, vm["qry_bloom"].as<string>() + '/' + qryFilenames.at(j), vm["dir_client"].as<string>() + string(bl::dir_client_qry) + qryFilenames.at(j), setBits);
        }
    }

    if(vm.count("execute"))
    {
        /*
         * Step 3: Process all database files with their respective chunks
         *
         */
        bl::execute("[CLT]", vm["dir_client"].as<string>() + bl::dir_client_db, vm["dir_client"].as<string>() + bl::dir_client_qry, vm["dir_client"].as<string>() + bl::dir_client_res, publicKey, ea);
    }
    else
    {
        if(vm.count("upload_key"))
            upload_rsync("public key", vm["dir_client"].as<string>() + bl::dir_client_pubKey, vm["dir_server"].as<string>() + bl::dir_server_pubKey, vm["a"].as<string>(), vm["u"].as<string>(), vm["p"].as<int>());
        if(vm.count("upload_db"))
            upload_rsync("encrypted database", vm["dir_client"].as<string>() + bl::dir_client_db, vm["dir_server"].as<string>() + bl::dir_server_db, vm["a"].as<string>(), vm["u"].as<string>(), vm["p"].as<int>());
        if(vm.count("upload_qry"))
            upload_rsync("encrypted query", vm["dir_client"].as<string>() + bl::dir_client_qry, vm["dir_server"].as<string>() + bl::dir_server_qry, vm["a"].as<string>(), vm["u"].as<string>(), vm["p"].as<int>());
    }

    /*
     * Step 4a: Return Result
     * Step 4b: Decrypt and process into end result
     *
     */

    if(vm.count("download"))
    {
        //delete local result
        create_dir(vm["dir_client"].as<string>() + bl::dir_client_res, "database_.*");
        download_rsync(vm["dir_client"].as<string>() + bl::dir_client_res, vm["dir_server"].as<string>() + bl::dir_server_res, vm["a"].as<string>(), vm["u"].as<string>(), vm["p"].as<int>());
    }

    if(vm.count("decrypt") || vm.count("execute"))
    {
        map<string, int> results;
        vector<string> resFilenames = bl::enumerateFiles(vm["dir_client"].as<string>() + bl::dir_client_res, "database_.*");

        #pragma omp parallel for schedule(dynamic,1)
        for (int j = 0; j < (int) resFilenames.size(); j++)
        {
            string prefix = resFilenames.at(j).substr(0, resFilenames.at(j).size()-string("_ct_chunk_0.enc").length());

            #pragma omp critical
            cout << "[CLT] >> Decrypting: " << prefix << "_result.enc" << endl;

            Ctxt ctResult = bl::loadCtChunk(0, publicKey, vm["dir_client"].as<string>() + bl::dir_client_res + prefix);

            // Decrypt
            vector<long> temp;
            ea.decrypt(ctResult, secretKey, temp);

            #pragma omp critical
            cout << "[CLT] <<     Result: " << prefix << " - " << temp.at(0) << endl;
        }
    }
}

