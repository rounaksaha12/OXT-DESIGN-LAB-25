// mainwindow_server.cpp

#include "mainwindow_server.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ssize_t recv_all(int sockfd, unsigned char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t bytes_received = recv(sockfd, buffer + total_received, length - total_received, 0);
        if (bytes_received <= 0) {
            // Connection closed or error
            return bytes_received;
        }
        total_received += bytes_received;
    }
    return total_received;
}

ssize_t send_all(int sockfd, unsigned char* buffer, size_t length) {
    size_t total_sent = 0;

    while (total_sent < length) {
        ssize_t bytes_sent = send(sockfd, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent <= 0) {
            // Error or connection closed
            return bytes_sent;
        }
        total_sent += bytes_sent;
    }

    return total_sent;
}

/**
 * Sends a file over a TCP socket
 * @param sockfd The socket file descriptor
 * @param filename The name of the file to send
 * @return 0 on success, -1 on failure
 */
int send_file(int sockfd, const char* filename) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << filename << ": " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Get file size
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Allocate memory for file contents
    std::vector<unsigned char> buffer(file_size);
    
    // Read file into buffer
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        std::cerr << "Error reading file " << filename << ": " << strerror(errno) << std::endl;
        file.close();
        return -1;
    }
    file.close();
    
    // First send the size of the file
    uint64_t size = static_cast<uint64_t>(file_size);
    if (send_all(sockfd, reinterpret_cast<unsigned char*>(&size), sizeof(size)) != sizeof(size)) {
        std::cerr << "Error sending file size: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Then send the file data
    if (send_all(sockfd, buffer.data(), file_size) != file_size) {
        std::cerr << "Error sending file data: " << strerror(errno) << std::endl;
        return -1;
    }
    
    std::cout << "File " << filename << " sent successfully (" << file_size << " bytes)" << std::endl;
    return 0;
}

/**
 * Receives a file over a TCP socket
 * @param sockfd The socket file descriptor
 * @param filename The name to save the received file as
 * @return 0 on success, -1 on failure
 */
int receive_file(int sockfd, const char* filename) {
    // First receive the file size
    uint64_t file_size;
    if (recv_all(sockfd, reinterpret_cast<unsigned char*>(&file_size), sizeof(file_size)) != sizeof(file_size)) {
        std::cerr << "Error receiving file size: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Allocate buffer for file data
    std::vector<unsigned char> buffer(file_size);
    
    // Receive file data
    if (recv_all(sockfd, buffer.data(), file_size) != static_cast<ssize_t>(file_size)) {
        std::cerr << "Error receiving file data: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Write data to file
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error creating file " << filename << ": " << strerror(errno) << std::endl;
        return -1;
    }
    
    outfile.write(reinterpret_cast<char*>(buffer.data()), file_size);
    if (outfile.fail()) {
        std::cerr << "Error writing to file " << filename << ": " << strerror(errno) << std::endl;
        outfile.close();
        return -1;
    }
    
    outfile.close();
    std::cout << "File " << filename << " received successfully (" << file_size << " bytes)" << std::endl;
    return 0;
}

void printMemoryNibbles(const void* address, size_t k) {
    const uint8_t* bytePtr = static_cast<const uint8_t*>(address);

    // Find the last non-zero byte
    size_t lastNonZero = 0;
    for (size_t i = 0; i < k; ++i) {
        if (bytePtr[i] != 0) {
            lastNonZero = i;
        }
    }

    // Print each nibble up to the last non-zero byte
    for (size_t i = 0; i <= lastNonZero; ++i) {
        uint8_t byte = bytePtr[i];
        std::cout << std::hex << ((byte >> 4) & 0xF);
        std::cout << std::hex << (byte & 0xF);
        std::cout << " ";
    }

    std::cout << std::dec << std::endl;
}

int Sys_Init()
{
    connection_options.host = "127.0.0.1";  // Required.
    pool_options.size = N_threads;  // Pool size, i.e. max number of connections.

    BloomFilter_Init(BF);
    SetUpThreads();

    return 0;
}

int Sys_Clear()
{
    BloomFilter_Clean(BF);
    ReleaseThreads();

    return 0;
}


int SetUpThreads()
{
    GL_AES_PT = new unsigned char[N_threads*16];
    GL_AES_KT = new unsigned char[16];
    GL_AES_CT = new unsigned char[N_threads*16];
    GL_ECC_INVA = new unsigned char[N_threads*32];
    GL_ECC_INVP = new unsigned char[N_threads*32];
    GL_ECC_SCA = new unsigned char[N_threads*32];
    GL_ECC_BP = new unsigned char[N_threads*32];
    GL_ECC_SMP = new unsigned char[N_threads*32];
    GL_ECC_INB = new unsigned char[N_threads*32];
    GL_ECC_INA = new unsigned char[N_threads*32];
    GL_ECC_PRD = new unsigned char[N_threads*32];
    GL_HASH_MSG = new unsigned char[N_threads*16];
    GL_HASH_DGST = new unsigned char[N_threads*64];
    GL_BLM_MSG = new unsigned char[N_threads*40];
    GL_BLM_DGST = new unsigned char[N_threads*64];

    GL_MGDB_RES = new unsigned char[N_threads*49];
    GL_MGDB_BIDX = new unsigned char[N_threads*2];
    GL_MGDB_JIDX = new unsigned char[N_threads*2];
    GL_MGDB_LBL = new unsigned char[N_threads*12];

    GL_OPCODE = new unsigned int;

    ready = false;
    processed = false;

    nWorkerCount = N_threads;
    nCurrentIteration = 0;

    thread_pool.clear();
    for(unsigned int i=0;i<N_threads;++i){
        thread_pool.push_back(thread(
          WorkerThread,
          GL_AES_PT+(i*16),
          GL_AES_KT,
          GL_AES_CT+(i*16),
          GL_ECC_INVA+(i*32),
          GL_ECC_INVP+(i*32),
          GL_ECC_SCA+(i*32),
          GL_ECC_BP+(i*32),
          GL_ECC_SMP+(i*32),
          GL_ECC_INB+(i*32),
          GL_ECC_INA+(i*32),
          GL_ECC_PRD+(i*32),
          GL_HASH_MSG+(i*16),
          GL_HASH_DGST+(i*64),
          GL_BLM_MSG+(i*40),
          GL_BLM_DGST+(i*64),
          GL_MGDB_RES+(i*49),
          GL_MGDB_BIDX+(i*2),
          GL_MGDB_JIDX+(i*2),
          GL_MGDB_LBL+(i*12),
          GL_OPCODE
        ));
    }

    int rc = 0;
    for(unsigned int i=0;i<N_threads;++i){
          cpu_set_t cpuset;
          CPU_ZERO(&cpuset);
          CPU_SET(i, &cpuset);
          rc = pthread_setaffinity_np(thread_pool[i].native_handle(), sizeof(cpu_set_t), &cpuset);
          if (rc != 0) {
              std::cout << "Error calling pthread_setaffinity_np: " << rc << std::endl;
          }
    }

    return 0;
}

int ReleaseThreads()
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        nWorkerCount = N_threads;
        processed = true;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
      std::unique_lock<std::mutex> lock(mrun);
      workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    for(std::thread &every_thread : thread_pool){
        every_thread.join();
    }

    thread_pool.clear();

    delete [] GL_AES_PT;
    delete [] GL_AES_KT;
    delete [] GL_AES_CT;
    delete [] GL_ECC_INVA;
    delete [] GL_ECC_INVP;
    delete [] GL_ECC_SCA;
    delete [] GL_ECC_BP;
    delete [] GL_ECC_SMP;
    delete [] GL_ECC_INB;
    delete [] GL_ECC_INA;
    delete [] GL_ECC_PRD;
    delete [] GL_HASH_MSG;
    delete [] GL_HASH_DGST;
    delete [] GL_BLM_MSG;
    delete [] GL_BLM_DGST;

    delete [] GL_MGDB_RES;
    delete [] GL_MGDB_BIDX;
    delete [] GL_MGDB_JIDX;
    delete [] GL_MGDB_LBL;

    delete GL_OPCODE;

    return 0;
}

int WorkerThread(
  unsigned char *AES_PT,
  unsigned char *AES_KT,
  unsigned char *AES_CT,
  unsigned char *ECC_INVA,
  unsigned char *ECC_INVP,
  unsigned char *ECC_SCA,
  unsigned char *ECC_BP,
  unsigned char *ECC_SMP,
  unsigned char *ECC_INA,
  unsigned char *ECC_INB,
  unsigned char *ECC_PRD,
  unsigned char *HASH_MSG,
  unsigned char *HASH_DGST,
  unsigned char *BLM_MSG,
  unsigned char *BLM_DGST,
  unsigned char *MGDB_RES,
  unsigned char *MGDB_BIDX,
  unsigned char *MGDB_JIDX,
  unsigned char *MGDB_LBL,
  unsigned int *OPCODE
)
{

    int nNextIteration = 1;

    Redis redis_thread(connection_options, pool_options);
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);

    while(!processed){
        std::unique_lock<std::mutex> lock(mrun);
        dataReady.wait(lock, [&nNextIteration] { return nCurrentIteration==nNextIteration; });
        lock.unlock();

        ++nNextIteration;

        //Do thread stuff here
        if(*OPCODE == 1){
            AESENC(AES_CT,AES_PT,AES_KT);
        }
        else if(*OPCODE == 2){
            SHA3_HASH(&hasher,HASH_MSG,HASH_DGST);
        }
        else if(*OPCODE == 3){
            SHA3_HASH_K(&hasher,BLM_MSG,BLM_DGST);
        }
        else if(*OPCODE == 4){
            ECC_MUL(ECC_INA,ECC_INB,ECC_PRD);
        }
        else if(*OPCODE == 5){
            ECC_FPINV(ECC_INVA,ECC_INVP);
        }
        else if(*OPCODE == 6){
            ScalarMul(ECC_SMP,ECC_SCA,ecc_basep);
        }
        else if(*OPCODE == 7){
            ScalarMul(ECC_SMP,ECC_SCA,ECC_BP);
        }
        else if(*OPCODE == 8){
            string s = HexToStr(MGDB_BIDX,2) + HexToStr(MGDB_JIDX,2) + HexToStr(MGDB_LBL,12);
            cout<<"string s with which redis db is queried = "<<s<<endl;
            auto val = redis_thread.get(s);
            unsigned char *t_res = reinterpret_cast<unsigned char *>(val->data());
            DB_StrToHex49(MGDB_RES,t_res);
        }
        else if(*OPCODE == 9){
            PRF(AES_CT,AES_PT,AES_KT);
        }
        else{
            std::this_thread::sleep_for (std::chrono::milliseconds(1));//Check if it results in computation error
        }

        lock.lock();
        if (--nWorkerCount == 0)
        {
          lock.unlock();
          workComplete.notify_one();
        }
    }
    return 0;
}

int EDB_SetUp(int socket_fd)
{
    
    cout << "Executing TSet Setup..." << endl;

    TSet_SetUp(socket_fd);

    cout << "TSet SetUp Done!" << endl;
    cout << "[SERVER] Returning from EDB_SetUp..." << endl;

    return 0;
}

int EDB_Search(unsigned char *query_str, int NWords, int socket_fd)
{
    // cout<<"NWords = "<<NWords<<endl;
    unsigned char Q1[16];

    unsigned char *stag;
    unsigned char *tset_row;
    unsigned char *WC;
    unsigned char *FW1;
    unsigned char *KXWL;
    unsigned char *FPKXWL;
    unsigned char *G_WC;
    unsigned char *G_FW1;
    unsigned char *GFW_KX;
    unsigned char *XTOKEN;
    unsigned char *XTAG;
    unsigned char *bhash;
    unsigned char *ESET;
    
    int N_words = 0;
    unsigned int N_max_id_words = 0;
    
    N_words = (N_max_ids/N_threads) + ((N_max_ids%N_threads==0)?0:1);
    N_max_id_words = N_words * N_threads;
    
    cout<<"N_words = "<<N_words<<endl;
    cout<<"N_max_id_words = "<<N_max_id_words<<endl;

    stag = new unsigned char[16];
    tset_row = new unsigned char[48*N_max_id_words];
    WC = new unsigned char[16*N_max_id_words];
    FW1 = new unsigned char[16*N_max_id_words];
    KXWL = new unsigned char[16*N_max_id_words];
    FPKXWL = new unsigned char[16*N_max_id_words];
    G_WC = new unsigned char[32*N_max_id_words];
    G_FW1 = new unsigned char[32*N_max_id_words];
    GFW_KX = new unsigned char[32*N_max_id_words];
    XTOKEN = new unsigned char[32*N_max_id_words];
    XTAG = new unsigned char[32*N_max_id_words];
    bhash = new unsigned char[64*N_threads];
    ESET = new unsigned char[16*N_max_id_words];

    unsigned char *YID_ALL;
    YID_ALL = new unsigned char[32*N_max_id_words];

    unsigned char YID[32];
    unsigned char ECE[16];

    bool idx_in_set = false;
    int nmatch = 0;
    unsigned int bfidx = 0;

    unsigned int** bf_n_indices;

    int n_ids_tset = 0;

    double time_elapsed;

    bf_n_indices = new unsigned int * [N_HASH];
    for(unsigned int i=0;i<N_HASH;++i){
        bf_n_indices[i] = new unsigned int [NWords];//check what happens if NWords is 0
    }

    ::memset(stag,0x00,16);
    ::memset(tset_row,0x00,48*N_max_id_words);
    ::memset(WC,0x00,16*N_max_id_words);
    ::memset(FW1,0x00,16*N_max_id_words);
    ::memset(KXWL,0x00,16*N_max_id_words);
    ::memset(FPKXWL,0x00,16*N_max_id_words);
    ::memset(G_WC,0x00,32*N_max_id_words);
    ::memset(G_FW1,0x00,32*N_max_id_words);
    ::memset(GFW_KX,0x00,32*N_max_id_words);
    ::memset(XTOKEN,0x00,32*N_max_id_words);
    ::memset(XTAG,0x00,32*N_max_id_words);
    ::memset(ESET,0x00,16*N_max_id_words);
    ::memset(YID_ALL,0x00,32*N_max_id_words);

    unsigned char *wc_local = WC;
    unsigned char *fw1_local = FW1;

    unsigned char *kxwl_local = KXWL;
    unsigned char *fpkxwl_local = FPKXWL;

    unsigned char *g_wc_local = G_WC;
    unsigned char *g_fw1_local = G_FW1;

    unsigned char *tset_row_local = tset_row;
    unsigned char *xtg_local = nullptr;
    unsigned char *eset_local = ESET;
    unsigned char *yid_local = YID_ALL;

    ::memcpy(Q1,query_str,16); // Q1 is the first keyword in the conjunctive query

    // TSet_GetTag(Q1,stag);

    /*
    
    receive stag from client
    
    */
   size_t stag_size = 16;
   recv_all(socket_fd, stag, stag_size);
   cout<<"Stag recvd = ";
   printMemoryNibbles(stag, stag_size);
    

    TSet_Retrieve(stag,tset_row,&n_ids_tset); // server can do this, not client

    /*
    
    client cannot perform TSet_Retrieve, so server would send n_ids_test to client


    therefore,
    send n_ids_test to client

    */
    size_t n_ids_tset_size = sizeof(n_ids_tset);
    send_all(socket_fd, (unsigned char*)&n_ids_tset, n_ids_tset_size);
    cout<<"Sent n_ids_tset = ";
    printMemoryNibbles(&n_ids_tset, n_ids_tset_size);

    // cout << "N IDs TSet: " << n_ids_tset << endl;

    N_words = (n_ids_tset/N_threads) + ((n_ids_tset%N_threads==0)?0:1);

    // for(int i=0;i< N_max_id_words;++i)
    // {
    //     ::memcpy(WC+(i*16),Q1,16); // why are we filling all WC cells with Q1?? what is this WC anyway, edit: looks like WC is going to contain w1||c for different values of counter c, it is being initialised with the first query keyword
    // }
    /* above part of code is only executed at the client side */

	//Copy all query keywords except first
    // ::memcpy(KXWL,query_str+16,(NWords*16)); // {w_2, w_3, ..., w_n}, makes sense
    /* above part of code is only executed at the client side */

    // unsigned int count_wc = 0;
    // unsigned int count_wc_local = 0;
    // for(unsigned int nword = 0;nword < n_ids_tset;++nword){
    //     // this looks like the loop where the consturction of w1||c is being done
    //     count_wc_local = count_wc;
    //     WC[(nword*16)+15] = count_wc_local & 0xFF; 
    //     count_wc_local >>= 8;
    //     WC[(nword*16)+14] = count_wc_local & 0xFF;
    //     // are we overwriting the last two bytes of w1?? but it is supposed to be w1||c right??
    //     count_wc++;
    // }
    /* above part of code is only executed at the client side */

    // wc_local = WC;
    // fw1_local = FW1;
    // for(int nword = 0;nword < N_words;++nword){
    //     // FPGA_AES_ENC(wc_local,KZ,fw1_local);
    //     FPGA_PRF(wc_local,KZ,fw1_local); // computation of z
    //     wc_local += sym_block_size;
    //     fw1_local += sym_block_size;
    // }
    // wc_local = WC;
    // fw1_local = FW1;
    // // fw1_local contains array of z s i believe
    /* above part of code is executed at the client only */

    // kxwl_local = KXWL;
    // fpkxwl_local = FPKXWL;
    // for(int nword = 0;nword < N_words;++nword){
    //     // FPGA_AES_ENC(kxwl_local,KX,fpkxwl_local);
    //     FPGA_PRF(kxwl_local,KX,fpkxwl_local); // The other w_i (i!=1) specific thingy multiplied with z to get z 
    //     kxwl_local += sym_block_size;
    //     fpkxwl_local += sym_block_size;
    // }
    // kxwl_local = KXWL;
    // fpkxwl_local = FPKXWL;
    /* above part of code is executed in the client side only */

    // xtoken[c,i] = wc_local[c] * kxwl_local[i]

    tset_row_local = tset_row;

    for(int n=0;n<n_ids_tset;++n){
        // here we are iterating over all (e,y) pairs obtained from TSetRetrieve(Tset,stag) :
        // look at how things are updated at the end: 
        // tset_row_local +=48; each (e,y) pair is 48bytes, actually the first 32 bytes are y part and last 16 bytes are e part
        // fw1_local += 16;

        idx_in_set = false;

        // ::memset(G_WC,0x00,32*N_max_id_words);
        // ::memset(G_FW1,0x00,32*N_max_id_words);
        // ::memset(GFW_KX,0x00,32*N_max_id_words);
        // ::memset(GFW_KX,0x00,32*N_max_id_words);
        // ::memset(ECE,0x00,16);
        /* executed only at the client side */

        // g_wc_local = G_WC;
        // g_fw1_local = G_FW1;
        // fpkxwl_local = FPKXWL;
        // yid_local = YID_ALL;

        // for(int i=0;i<NWords;++i){
        //     ::memcpy(g_wc_local+16,fpkxwl_local,16);
        //     ::memcpy(g_fw1_local+16,fw1_local,16);
        //     g_wc_local += 32;
        //     g_fw1_local += 32;
        //     fpkxwl_local += 16;
        //     // fw1_local does not change in this loop => we are doing this for the same counter value
        // }

        // g_wc_local = G_WC; // Fp(Kx, w_i) for i \in {2,3,...,NWords}
        // g_fw1_local = G_FW1; // Fp(Kz, w1||c), same entry in all cels of g_fe1_local
        // fpkxwl_local = FPKXWL; // essentially same thing: as g_wc_local, // Fp(Kx, w_i) for i \in {2,3,...,NWords}, exect that each entry in g_wc_local is 32 bytes but each entry in gpkxwl is 16 bytes. last 16 bytes of g_wc_local are same as the corresponding entry in fpkxwl

        // FPGA_ECC_MUL(G_WC,G_FW1,GFW_KX);//Assumed that NWords is less than n threads
        // // i am assuming this GFW_KX stores [Fp(Kz,w1||c).Fp(Kx,w_i)], i!=1

        // FPGA_ECC_SCAMUL(GFW_KX,XTOKEN);//Assumed that NWords is less than n threads

        ::memcpy(YID,tset_row_local,32); // get the y part from this (e,y) pair
        ::memcpy(ECE,tset_row_local+32,16); // get the e part from this (e,y) pair

        // now we are going towards veryifying whether g^(Fp(Kz,w1||c).Fp(Kx,w_i),y) for all i \in {2,3,..,NWords}

        /*
        
        till now, the server has not done much, but now it is time to shine.

        so, now the server will receive the XTOKEN from the client and on to verifying whether g^(Fp(Kz,w1||c).Fp(Kx,w_i),y) for all i \in {2,3,..,NWords}

        therefore, receive XTOKEN
        
        */
       size_t xtoken_size = 32*N_max_id_words;
       recv_all(socket_fd, XTOKEN, xtoken_size);
       cout<<"Recvd XTOKEN = ";
       printMemoryNibbles(XTOKEN, xtoken_size);

        yid_local = YID_ALL;
        for(int i=0;i<NWords;++i){//This should run till row_len
            ::memcpy(yid_local,YID,32);
            yid_local += 32;
        }
        yid_local = YID_ALL; // we copied y of this (e,y) in all Nwords cells of YID_ALL

        if(NWords == 0){
            ::memcpy(eset_local,ECE,16);
            eset_local +=16;
            ++nmatch;
        }
        else{
            //xtag computation

            FPGA_ECC_SCAMUL_BASE(YID_ALL,XTOKEN,XTAG);//Assumed that NWords is less than 128
            // i am assuming we are computing (g^(Fp(Kz,w1||c).Fp(Kx,w_i)))^    y    here
            //                                <-------xtoken[c][i]--------> <---y--->

            xtg_local = XTAG;

            // just check if xtg is in the damn bloom filter
            // ideally all what has happened upto here should happen in the client side, only xtg_local should be sent to the server

            for(int i=0;i<NWords;++i){//Check this length

                ::memset(bhash,0x00,bhash_block_size);
                FPGA_BLOOM_HASH(xtg_local,bhash);

                for(int j=0;j<N_HASH;++j){
                    bf_n_indices[j][i] = BFIdxConv(bhash+(64*j),N_BF_BITS);
                    // bf_n_indices[j][i] = (bhash[64*j] & 0xFF) + ((bhash[64*j+1] & 0xFF) << 8) + ((bhash[64*j+2] & 0x01) << 16);
                }

                xtg_local +=32;
            }

            BloomFilter_Match_N(BF, bf_n_indices, NWords, &idx_in_set); // modifies idx_in_set flag to true if xtag was found in the bloom filter

            if(idx_in_set){
                ::memcpy(eset_local,ECE,16);
                eset_local +=16; // i guess this is where we are storing the e values of the documents which passed the test, server should return this to client
                ++nmatch;
            }

            // tset_row_local +=48; // we go to next (e,y) pair
            // fw1_local += 16; // next counter

            /*
            
            I dont think updating tset_row_local and fw1_local should ONLY happen in the else part, for the if part (i.e. if NWords==0, then too you need to go to the next (e,y) pair)

            with this understandng, I AM GOING TO TAKE A HUGE RISK AND DO WHAT MY HEAR SAYS, WHICH IS TO MOVE THE TSET_ROW_LOCAL AND FW1_LOCAL UPDATE LOGIC OUT OUT THE ELSE PART, OM SHANTI
            
            */
        }

        tset_row_local +=48; // we go to next (e,y) pair
        fw1_local += 16; // next counter
    }

    // server's job should end here

    /*
    
    now the server will communicate the results to client in the form of: nmatch and eset_local


    therefore,
    send nmatch

    send ESET

    */
    size_t nmatch_size = sizeof(nmatch);
    send_all(socket_fd, (unsigned char*)&nmatch, nmatch_size);
    cout<<"Sent nmatch_size = ";
    printMemoryNibbles(&nmatch, nmatch_size);

    size_t ESET_size = 16*N_max_id_words;
    send_all(socket_fd, ESET, ESET_size);
    cout<<"Sent ESET = ";
    printMemoryNibbles(ESET, ESET_size);

    cout << "Nmatch: " << nmatch << endl;

    // unsigned char KE[16];

    // ::memset(KE,0x00,16);

    // AESENC(KE,Q1,KS); // we got the encryted e's, but the key with which these were encrypted was KS. KE = F(KS,w1), here w1 is same as Q1. This step should happen in client

    // for(int i=0;i<nmatch;++i){
    //     AESDEC(UIDX+(16*i),ESET+(16*i),KE); // write the decrypted doc ids in uidx array, this also happens in the client
    // }
    
    // //auto stop_time = chrono::high_resolution_clock::now();
    // //auto time_elapsed = chrono::duration_cast<chrono::microseconds>(stop_time - start_time).count();
    // //cout << time_elapsed << endl;
    /* above part is only executed at the client, as server does not have the power to */

    for(unsigned int i=0;i<N_HASH;++i){
        delete [] bf_n_indices[i];
    }
    delete [] bf_n_indices;

    delete [] stag;
    delete [] tset_row;
    delete [] WC;
    delete [] FW1;
    delete [] KXWL;
    delete [] FPKXWL;
    delete [] G_WC;
    delete [] G_FW1;
    delete [] GFW_KX;
    delete [] XTOKEN;
    delete [] XTAG;
    delete [] bhash;
    delete [] ESET;

    delete [] YID_ALL;

    return nmatch;
}


int TSet_SetUp(int socket_fd)
{
    
    auto redis = Redis("tcp://127.0.0.1:6379");

    int n_rows = 0;
    int n_row_ids = 0;

    std::string db_in_key = "";
    std::string db_in_val = "";

    /*
    
    recv value of n_rows fron client
    
    */
    recv_all(socket_fd, (unsigned char*)&n_rows, sizeof(n_rows));

    for(int n=0;n<n_rows;++n){

        /*
        
        recvd n_row_ids from client
        
        */
        recv_all(socket_fd, (unsigned char*)&n_row_ids, sizeof(n_row_ids));

        
        for(int i=0;i<n_row_ids;++i){

            db_in_key.clear();
            db_in_val.clear();

            /*
            
            recv key, value pair from client and write that entry to redis
            
            */
            unsigned char db_in_key_buf[32];
            unsigned char db_in_val_buf[98];
            recv_all(socket_fd, db_in_key_buf, 32);
            recv_all(socket_fd, db_in_val_buf, 98);

            db_in_key = std::string(reinterpret_cast<const char*>(db_in_key_buf), 32);
            db_in_val = std::string(reinterpret_cast<const char*>(db_in_val_buf), 98);

            redis.set(db_in_key.data(), db_in_val.data());

        }
    }

    cout<<"[SERVER] Returning from TSet_SetUp"<<endl;
    return 0;
}

int TSet_GetTag(unsigned char *word, unsigned char *stag)
{
    ::memset(stag,0x00,16);
    AESENC(stag,word,KT);
    return 0;
}

int TSet_Retrieve(unsigned char *stag,unsigned char *tset_row, int *n_ids_tset)
{
    unsigned char *stagi;
    unsigned char *stago;
    unsigned char *hashin;
    unsigned char *hashout;
    unsigned char *TV;
    
    int N_words = 0;
    unsigned int N_max_id_words = 0;
    
    N_words = (N_max_ids/N_threads) + ((N_max_ids%N_threads==0)?0:1);
    N_max_id_words = N_words * N_threads;

    stagi = new unsigned char[16*N_max_id_words];
    stago = new unsigned char[16*N_max_id_words];
    hashin = new unsigned char[16*N_max_id_words];
    hashout = new unsigned char[64*N_max_id_words];
    TV = new unsigned char[48*N_max_id_words];

    unsigned char TENTRY[61];
    unsigned char TVAL[49];
    unsigned char TBIDX[2];
    unsigned char TJIDX[2];
    unsigned char TLBL[12];
    unsigned char HLBL[12];


    unsigned int *FreeB;
    int bidx=0;
    int len_freeb = 65536;
    int freeb_idx = 0;
    bool BETA = 0;

    unsigned char *stagi_local;
    unsigned char *stago_local;
    unsigned char *hashin_local;
    unsigned char *hashout_local;

    unsigned char * TV_curr;

    unsigned char *T_RES;
    unsigned char *T_BIDX;
    unsigned char *T_JIDX;
    unsigned char *T_LBL;

    unsigned char *local_t_res;
    unsigned char *local_t_bidx;
    unsigned char *local_t_jidx;
    unsigned char *local_t_lbl;
 
    unsigned char *local_t_res_word;
    unsigned char *local_t_bidx_word;
    unsigned char *local_t_jidx_word;
    unsigned char *local_t_lbl_word;
    unsigned char *local_hashout_word;

    T_RES = new unsigned char[49*N_max_id_words];
    T_BIDX = new unsigned char[2*N_max_id_words];
    T_JIDX = new unsigned char[2*N_max_id_words];
    T_LBL = new unsigned char[12*N_max_id_words];

    int rcnt = 0;

    ::memset(stagi,0x00,16*N_max_id_words);
    ::memset(stago,0x00,16*N_max_id_words);
    ::memset(hashin,0x00,16*N_max_id_words);
    ::memset(hashout,0x00,64*N_max_id_words);
    ::memset(TV,0x00,48*N_max_id_words);

    ::memset(TVAL,0x00,49);
    ::memset(TJIDX,0x00,2);

    FreeB = new unsigned int[len_freeb];

    for(int bc=0;bc<len_freeb;++bc){
        FreeB[bc] = 0;
    }

    //Fill stagi array
    stagi_local = stagi;
    for(int nword = 0;nword < N_words;++nword){
        for(int nid=0;nid<N_threads;++nid){
            stagi_local[0] = ((nword*N_threads)+nid) & 0xFF;
            stagi_local += 16;
        }
    }
    stagi_local = stagi;

    //PRF of stag and if
    stagi_local = stagi;
    hashin_local = hashin;
    for(int nword = 0;nword < N_words;++nword){
        FPGA_AES_ENC(stagi_local,stag,hashin_local);
        stagi_local += sym_block_size;
        hashin_local += sym_block_size;
    }
    stagi_local = stagi;
    hashin_local = hashin;

    //Compute Hash
    hashin_local = hashin;
    hashout_local = hashout;
    for(int nword = 0;nword < N_words;++nword){
        FPGA_HASH(hashin_local,hashout_local);
        hashin_local += sym_block_size;
        hashout_local += hash_block_size;
    }
    hashin_local = hashin;
    hashout_local = hashout;

    TV_curr = TV;

    ::memset(T_RES,0x00,49*N_max_id_words);
    ::memset(T_BIDX,0x00,2*N_max_id_words);
    ::memset(T_JIDX,0x00,2*N_max_id_words);
    ::memset(T_LBL,0x00,12*N_max_id_words);

    local_t_res = T_RES;
    local_t_bidx = T_BIDX;
    local_t_jidx = T_JIDX;
    local_t_lbl = T_LBL;
    
    while(!BETA){

      local_hashout_word = hashout_local;
      
      local_t_res_word = local_t_res;
      local_t_bidx_word = local_t_bidx;
      local_t_jidx_word = local_t_jidx;
      local_t_lbl_word = local_t_lbl;

      for(unsigned int ni=0;ni<N_threads;++ni){
          ::memcpy(local_t_bidx,hashout_local,2);

          freeb_idx = ((local_t_bidx[1] << 8) + local_t_bidx[0]);

          bidx = (FreeB[freeb_idx]++);
          local_t_jidx[0] =  bidx & 0xFF;
          local_t_jidx[1] =  (bidx >> 8) & 0xFF;

          ::memcpy(local_t_lbl,hashout_local+2,12);
          
          local_t_bidx += 2;
          local_t_jidx += 2;
          local_t_lbl += 12;
          hashout_local +=64;
      }
      
      cout<<"local_t_bidx_word before mgdb_query = ";
      printMemoryNibbles(local_t_bidx_word, 2*N_max_id_words);
      
      cout<<"local_t_jidx_word before mgdb_query = ";
      printMemoryNibbles(local_t_jidx_word, 2*N_max_id_words);
      
      cout<<"local_t_lbl_word before mgdb_query = ";
      printMemoryNibbles(local_t_lbl_word, 12*N_max_id_words);
      
      // verified that inputs to MGDB_QUERY are same in both cases (just one query vs. that one query but after some other queries
      MGDB_QUERY(local_t_res,local_t_bidx_word,local_t_jidx_word,local_t_lbl_word);
      
      cout<<"local_t_res after mgdb_query = ";
      printMemoryNibbles(local_t_res, 49*N_max_id_words);
      
      for(unsigned int ni=0;ni<N_threads;++ni){
          ::memcpy(TVAL,local_t_res,49);
          BETA = TVAL[0] ^ local_hashout_word[15];

          for(int i=0;i<48;++i){
              TV_curr[i] = local_hashout_word[16+i] ^ TVAL[i+1];
          }

          rcnt++;
          if(BETA == 0x01) break;

          TV_curr += 48;
          local_t_res += 49;

          local_hashout_word += 64;
      }
    }
    
    *n_ids_tset = rcnt;

    ::memcpy(tset_row,TV,48*rcnt);

    delete [] stagi;
    delete [] stago;
    delete [] hashin;
    delete [] hashout;
    delete [] TV;

    delete [] FreeB;

    delete [] T_RES;
    delete [] T_BIDX;
    delete [] T_JIDX;
    delete [] T_LBL;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

int FPGA_AES_ENC(unsigned char *ptext,unsigned char *key, unsigned char *ctext)
{
    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_AES_CT,0x00,sym_block_size);
        ::memcpy(GL_AES_PT,ptext,sym_block_size);
        ::memcpy(GL_AES_KT,key,16);

        *GL_OPCODE = 1;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(ctext,GL_AES_CT,sym_block_size);

    return 0;
}

int FPGA_PRF(unsigned char *ptext,unsigned char *key, unsigned char *ctext)
{
    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_AES_CT,0x00,sym_block_size);
        ::memcpy(GL_AES_PT,ptext,sym_block_size);
        ::memcpy(GL_AES_KT,key,16);

        *GL_OPCODE = 9;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(ctext,GL_AES_CT,sym_block_size);

    return 0;
}

int FPGA_HASH(unsigned char *msg, unsigned char *digest)
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_HASH_DGST,0x00,hash_block_size);
        ::memcpy(GL_HASH_MSG,msg,sym_block_size);

        *GL_OPCODE = 2;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(digest,GL_HASH_DGST,hash_block_size);

    return 0;
}

int FPGA_BLOOM_HASH(unsigned char *msg, unsigned char *digest)
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_BLM_MSG,0x00,bhash_in_block_size);
        ::memset(GL_BLM_DGST,0x00,hash_block_size);
        for(int i=0;i<N_HASH;i++){ //keep this operation here
            ::memcpy(GL_BLM_MSG+(40*i),msg,32);
            GL_BLM_MSG[40*i+39] = (i & 0xFF);
        }

        *GL_OPCODE = 3;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(digest,GL_BLM_DGST,hash_block_size);

    return 0;
}

int FPGA_ECC_FPINV(unsigned char *fp_x, unsigned char *fp_invx)
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_ECC_INVP,0x00,ecc_block_size);
        ::memcpy(GL_ECC_INVA,fp_x,ecc_block_size);

        *GL_OPCODE = 5;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(fp_invx,GL_ECC_INVP,ecc_block_size);

    return 0;
}

int FPGA_ECC_MUL(unsigned char *in_A,unsigned char *in_B,unsigned char *prod)
{

      {
          std::lock_guard<std::mutex> lock(mrun);
          ::memset(GL_ECC_PRD,0x00,ecc_block_size);
          ::memcpy(GL_ECC_INA,in_A,ecc_block_size);
          ::memcpy(GL_ECC_INB,in_B,ecc_block_size);

          *GL_OPCODE = 4;

          nWorkerCount = N_threads;
          ++nCurrentIteration;
      }
      dataReady.notify_all();

      {
          std::unique_lock<std::mutex> lock(mrun);
          workComplete.wait(lock, [] { return nWorkerCount == 0; });
      }

      *GL_OPCODE = 0;

      ::memcpy(prod,GL_ECC_PRD,ecc_block_size);

      return 0;
}

int FPGA_ECC_SCAMUL(unsigned char *sca, unsigned char *prod)
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_ECC_SMP,0x00,ecc_block_size);
        ::memcpy(GL_ECC_SCA,sca,ecc_block_size);

        *GL_OPCODE = 6;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(prod,GL_ECC_SMP,ecc_block_size);

    return 0;
}

int FPGA_ECC_SCAMUL_BASE(unsigned char *sca, unsigned char *basep, unsigned char *prod)
{

    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_ECC_SMP,0x00,ecc_block_size);
        ::memcpy(GL_ECC_SCA,sca,ecc_block_size);
        ::memcpy(GL_ECC_BP,basep,ecc_block_size);

        *GL_OPCODE = 7;

        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(prod,GL_ECC_SMP,ecc_block_size);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MGDB_QUERY(unsigned char *RES, unsigned char *BIDX, unsigned char *JIDX, unsigned char *LBL)
{
    {
        std::lock_guard<std::mutex> lock(mrun);
        ::memset(GL_MGDB_BIDX, 0x00, N_threads*2);
        ::memset(GL_MGDB_JIDX, 0x00, N_threads*2);
        ::memset(GL_MGDB_LBL,  0x00, N_threads*12);
        ::memset(GL_MGDB_RES,  0x00, N_threads*49);
        
        ::memcpy(GL_MGDB_BIDX,BIDX,(N_threads * 2));
        ::memcpy(GL_MGDB_JIDX,JIDX,(N_threads * 2));
        ::memcpy(GL_MGDB_LBL,LBL,(N_threads * 12));
        ::memset(GL_MGDB_RES,0x00,(N_threads * 49));
        
//        for(unsigned int i=0;i<N_threads;++i){
//            std::cout << MDB_HexToStr(LBL+(12*i),12) << std::endl;
//        }

        *GL_OPCODE = 8;
        cout << "N_threads inside MGDB_QUERY = "<<N_threads<<endl;
        nWorkerCount = N_threads;
        ++nCurrentIteration;
    }
    dataReady.notify_all();

    {
        std::unique_lock<std::mutex> lock(mrun);
        workComplete.wait(lock, [] { return nWorkerCount == 0; });
    }

    *GL_OPCODE = 0;

    ::memcpy(RES,GL_MGDB_RES,(N_threads*49));

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

int SHA3_HASH(blake3_hasher *hasher,unsigned char *msg, unsigned char *digest)
{
    Blake3(hasher,digest,msg);
    return 0;
}

int SHA3_HASH_K(blake3_hasher *hasher,unsigned char *msg, unsigned char *digest)
{
    Blake3_K(hasher,digest,msg);
    return 0;
}

int ECC_FPINV(unsigned char *fp_x, unsigned char *fp_invx)
{
    mpz_class a, b;
    size_t ss = 32;
    mpz_import(a.get_mpz_t(),32,1,1,0,0,fp_x);
    mpz_powm(b.get_mpz_t(),a.get_mpz_t(),InvExp.get_mpz_t(),Prime.get_mpz_t());
    StrToHex(fp_invx,b.get_str(16));
    return 0;
}

int ECC_MUL(unsigned char *in_A,unsigned char *in_B,unsigned char *prod)
{
    mpz_class a, b, c, r;
    size_t ss = 32;

    mpz_import(a.get_mpz_t(),32,1,1,0,0,in_A);
    mpz_import(b.get_mpz_t(),32,1,1,0,0,in_B);
    r = a * b;
    mpz_mod(c.get_mpz_t(),r.get_mpz_t(),Prime.get_mpz_t());
    StrToHex(prod,c.get_str(16));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

std::string HexToStr(unsigned char *hexarr, int len)
{
    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < len; ++i)
        ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(hexarr[i]);
    return ss.str();
}

std::string NumToHexStr(int num){
    std::stringstream ss;
    ss << std::hex;

    ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(num & 0xFF);
    return ss.str();
}

int StrToHex(unsigned char *hexarr,string numin)
{
    std::string dest = std::string( 64-numin.length(), '0').append( numin);
    const char *text = dest.data();
    char temp[2];
    for (int j=0; j<32; j++)
    {
        temp[0] = text[2*j];
        temp[1] = text[2*j+1];
        hexarr[j] = ::strtoul(temp,nullptr,16) & 0xFF;
    }
    return 0;
}

int StrToHexBVec(unsigned char *hexarr,string bvec)
{
    const char *text = bvec.data();
    char temp[2];
    for (int j=0; j<4; j++)
    {
        temp[0] = text[2*j];
        temp[1] = text[2*j+1];
        hexarr[j] = ::strtoul(temp,nullptr,16) & 0xFF;
    }
    return 0;
}

unsigned int BFIdxConv(unsigned char *hex_arr,unsigned int n_bits)
{
    unsigned int idx_val = 0;
    unsigned int n_bytes = n_bits/8;
    unsigned int n_bits_rem = n_bits%8;
    unsigned char tmp_char;
    
    for(unsigned int i=0;i<n_bytes;++i){
        idx_val = (idx_val << 8) | hex_arr[i];
    }

    if(n_bits_rem != 0){
        tmp_char = hex_arr[n_bytes];
        tmp_char = tmp_char >> (8 - n_bits_rem);
        idx_val = (idx_val << n_bits_rem) | tmp_char;
    }

    return idx_val;
}

/*

THIS IS A CHECKPOINT, THINGS ARE WORKING UNTIL NOW. BUT NOW I AM GOING TO REMOVE EVERYTHING THAT I THINK IS NOT NECESSARY. BUT NOT EVERYTHING THAT I THINK IS CORRECT. SO THIS MESSAGE WILL TELL ME WHERE TO STOP WHILE I AM CTRL+Z-ING MY WAY TO A WORKING VERSION

*/
