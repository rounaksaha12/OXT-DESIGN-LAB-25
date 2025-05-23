#define _GNU_SOURCE

#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include <set>
#include <algorithm>
#include <functional>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "mainwindow_server.h"
#include "aes.h"

using namespace std;

int N_keywords = 0; //Number of keywords
int N_max_ids = 0; // Maximum number of ids for a kw -- or maximum frequency of a kw

int N_row_ids = N_max_ids;

string widxdb_file = "../databases/db6k.csv";//Raw enron database
string bloomfilter_file = "bloom_filter.dat";//Bloom filter file

sw::redis::ConnectionOptions connection_options;
sw::redis::ConnectionPoolOptions pool_options;

mpz_class Prime{"7237005577332262213973186563042994240857116359379907606001950938285454250989",10};//Curve25519 curve order
mpz_class InvExp{"7237005577332262213973186563042994240857116359379907606001950938285454250987",10};

unsigned char ecc_basep[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09};

unsigned char **BF;

unsigned char *UIDX;

unsigned char *GL_AES_PT;
unsigned char *GL_AES_KT;
unsigned char *GL_AES_CT;

unsigned char *GL_ECC_INVA;
unsigned char *GL_ECC_INVP;

unsigned char *GL_ECC_SCA;
unsigned char *GL_ECC_BP;
unsigned char *GL_ECC_SMP;

unsigned char *GL_ECC_INA;
unsigned char *GL_ECC_INB;
unsigned char *GL_ECC_PRD;

unsigned char *GL_HASH_MSG;
unsigned char *GL_HASH_DGST;

unsigned char *GL_BLM_MSG;
unsigned char *GL_BLM_DGST;

unsigned char *GL_MGDB_RES;
unsigned char *GL_MGDB_BIDX;
unsigned char *GL_MGDB_JIDX;
unsigned char *GL_MGDB_LBL;

unsigned int *GL_OPCODE;

unsigned int N_threads = 1;

std::mutex mrun;
std::condition_variable dataReady;
std::condition_variable workComplete;

bool ready = false;
bool processed = false;

size_t nWorkerCount = N_threads; //Number of threads
int nCurrentIteration = 0;

std::vector<thread> thread_pool;

int sym_block_size = 0;
int ecc_block_size = 0;
int hash_block_size = 0;
int bhash_block_size = 0;
int bhash_in_block_size = 0;
int N_HASH = 0;
int MAX_BF_BIN_SIZE = 0;
int N_BF_BITS = 0;

//Keeping all same for the debugging and experimental purpose
unsigned char KS[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KI[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KZ[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KX[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KT[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};

int ReadConfAll(std::string db_conf_filename)
{
    std::ifstream input_file(db_conf_filename);
    std::string line;

    if (input_file.good()) {
        //Line 1 -- plain database file name
        line.clear();
        getline(input_file,line);
        widxdb_file = line;
        
        //Line 2 -- number of cores
        line.clear();
        getline(input_file,line);
        N_threads = std::stoi(line);
        
        //Line 3 -- number of keywords in the plain database
        line.clear();
        getline(input_file,line); 
        N_keywords = std::stoi(line);

        //Line 4 -- maximum number of docuement identifiers for a keyword in the plain database
        line.clear();
        getline(input_file,line);
        N_max_ids = std::stoi(line);

        //Line 5 -- Plain DB Bloom filter size -- typically a power of 2, next to the total number keyword-id pairs
        line.clear();
        getline(input_file,line);
        MAX_BF_BIN_SIZE = std::stoi(line);

        //Line 6 -- Number of bits to address plain DB Bloom filter -- typically the n in 2^n of BF size
        line.clear();
        getline(input_file,line);
        N_BF_BITS = std::stoi(line);

        ///////////////////////////////////////////////////////////////////////
        //Existing values will get overwritten! I know not a good way to do this, but fine for now.

        /*
        //Line 7 -- meta database file name
        line.clear();
        getline(input_file,line);
        widxdb_file = line;
        
        //Line 8 -- number of cores to use for metadb
        line.clear();
        getline(input_file,line);
        N_threads = std::stoi(line);
        
        //Line 9 -- number of metakeywords in the meta database
        line.clear();
        getline(input_file,line); 
        N_keywords = std::stoi(line);

        //Line 10 -- maximum number of docuement identifiers for a metakeyword in the plain database
        line.clear();
        getline(input_file,line);
        N_max_ids = std::stoi(line);

        //Line 11 -- meta DB Bloom filter size -- typically a power of 2, next to the total number metakeyword-id pairs
        line.clear();
        getline(input_file,line);
        MAX_BF_BIN_SIZE = std::stoi(line);

        //Line 12 -- Number of bits to address meta DB Bloom filter -- typically the n in 2^n of meta BF size
        line.clear();
        getline(input_file,line);
        N_BF_BITS = std::stoi(line);
        */

        ///////////////////////////////////////////////////////////////////////

        nWorkerCount = N_threads;
        N_row_ids = N_max_ids;

        sym_block_size = N_threads * 16;
        ecc_block_size = N_threads * 32;
        hash_block_size = N_threads * 64;
        bhash_block_size = N_threads * 64;
        bhash_in_block_size = N_threads * 40;

        N_HASH = N_threads;
    }
    else{
        std::cout << "Error reading database configuration file" << std::endl;
        return -1;
    }

    input_file.close();


    // std::cout << "File path: " << widxdb_file << std::endl;
    // std::cout << "N_threads: " << N_threads << std::endl;
    // std::cout << "Number of keywords: " << N_keywords << std::endl;
    // std::cout << "Maximum number of ids: " << N_max_ids << std::endl;
    // std::cout << "Bloom filter size: " << MAX_BF_BIN_SIZE << std::endl;
    // std::cout << "Bloom filter address bits: " << N_BF_BITS << std::endl;

    return 0;
}

int main()
{
    cout << "Starting program..." << endl;

    ReadConfAll("../configuration/db6k.conf");

    UIDX = new unsigned char[16*N_max_ids];
    ::memset(UIDX,0x00,16*N_max_ids);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::vector<std::string> query;

    unsigned int n_keywords = 0;
    unsigned int n_iterations = 100;//Number of text vectors to search

    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------


	// CONNECTION VARIABLES ----------------------------------------------------------------------------------------------
    int portno = 8080;
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr, serv_addr;

    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------

    srand(time(NULL));

    //----------------------------------------------------------------------------------------------
    //Initialise
    Sys_Init();
    
    std::cout << "Reading Bloom Filter from disk..." << std::endl;
    BloomFilter_ReadBFfromFile(bloomfilter_file, BF); //Load bloom filter from file
    //----------------------------------------------------------------------------------------------
    // Search
    
    auto search_start_time = std::chrono::high_resolution_clock::now();
    auto search_stop_time = std::chrono::high_resolution_clock::now();
    auto search_time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(search_stop_time - search_start_time).count();

    unsigned int kw_idx = 0;
    unsigned int kw_freq = 0;
    unsigned int n_q_kw = 2;//Number of keywords in a query to search for

    std::vector<unsigned int> idx_vec;
    std::map<unsigned int,unsigned int> freq_map;
    std::vector<unsigned int> idx_sorted;
    std::vector<unsigned int> freq_sorted;
    std::vector<std::string> kw_sorted;

    unsigned int nm = 0;
    unsigned char row_vec[2048]; //16 bytes * Number of keywords in the query
    int n_vec = 0;
    std::set<std::string> result_temp;

	/*
	
	create the server socket, bind address and start listening
	
	*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		perror("Socket can't be opened\n");
		exit(1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if ((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) == -1)
	{
		perror("Could not bind\n");
		exit(1);
	}
	listen(sockfd, 1);

    unsigned int q_idx = 0;
    while(true){
        std::cout << "--------------------------------------------------" << std::endl;
        cout<<q_idx%100<<endl;
        query.clear();
        freq_map.clear();

        ::memset(row_vec,0x00,2048);
        n_vec = 0;
        
        ::memset(UIDX,0x00,16*N_max_ids);
        result_temp.clear();

		/*
		
		accept a client connection
		
		*/
		clilen = sizeof(cli_addr);
		cout<<"waiting for accept"<<endl;
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		cout<<"after accept"<<endl;
		if (newsockfd == -1)
		{
			perror("Connection error\n");
			exit(1);
		}
		else{
		        cout<<"accepted connection with client"<<endl;
		}

        //-------------------------------------------------------------------------------
        search_start_time = std::chrono::high_resolution_clock::now();

        // assuming conjunctive queries contain 2 keywords, hardcode n_vec = 2
        n_vec = 2;
        
        nm = EDB_Search(row_vec,(n_vec-1), newsockfd);

        search_stop_time = std::chrono::high_resolution_clock::now();
        search_time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(search_stop_time - search_start_time).count();

        // std::cout << "Search done!" << std::endl;

        result_temp.clear();
        query.clear();

		/*
		
		close newsockfd
		
		*/
		close(newsockfd); 
        // cout<<q_idx<<": newsockfd is closed"<<endl;

        q_idx++;
    }

	/*
	
	close sokcfd
	
	*/
	close(sockfd);
    cout<<"sockfd is closed"<<endl;


    //----------------------------------------------------------------------------------------------
    // Thread Release
    Sys_Clear();
    delete [] UIDX;
    //----------------------------------------------------------------------------------------------

    return 0;

    cout << "Program finished!" << endl;

    return 0;
}
