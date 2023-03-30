#include<iostream>
#include"server.h"
#include<vector>
#include<fstream>
#include<mutex>
#include<iomanip>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <memory.h>

class Job {
//function objects that will be put into the queue of the thread_pool
private:
    //the next order no, catalog server's ip and port, and the file handler of order log file are designed to be shared and accessed by every Job object
    static unsigned long long next_order_no;
	static std::mutex mutex; //used to synchroniza log operation
    static struct sockaddr_in catalog_serv_addr;
    static std::fstream file;
    
    int socket_to_front;//member variable of each Job object
public:
	static void InitJob(std::string catalog_serv_ip, unsigned catalog_serv_port, std::string order_log_file){
        catalog_serv_addr.sin_family = AF_INET;
        catalog_serv_addr.sin_port = htons(catalog_serv_port);
        catalog_serv_addr.sin_addr.s_addr = inet_addr(catalog_serv_ip.c_str());
		//the log file has historical order no stored in the first line, and then stores all the transaction data line by line, with space as delimitnator 
        file.open(order_log_file);
		if(!file.is_open()){
			std::cout<<"read file error";
			throw;
		}
		file>>next_order_no; //read the historical order no because we need to increment based on that number
		file.seekp(0, std::ios_base::end);//set the write position to the end of file.
    }
	static void DeInit(){
		file.seekp(std::ios_base::beg);
		file<<std::setfill('0') << std::setw(20)<<next_order_no; //make sure the beginning order no takes fix space, otherwise data could be collapsed.
		file.close();
	}
	Job(){}
	Job(int socket)
		:socket_to_front(socket){}
	void operator()(){
		//all operations including receving cmds from clients, parsing the cmds, doing actual processing, builing the reply msgs and sending the reply back
		//are done inside the worker thread, thus makes the main thread able to respond to other request quickly 
		char buffer[128];
		read(socket_to_front, buffer, 128);
		
		std::string cmd(buffer);
		std::vector<std::string> arg_list;
		size_t pos = 0;
		std::string delimiter = " ";
		while ((pos = cmd.find(delimiter)) != std::string::npos) {
			arg_list.push_back(cmd.substr(0, pos));
			cmd.erase(0, pos + delimiter.length());
		}
		arg_list.push_back(cmd);

		//the reply is a text string built from converting each fields of data to string and concatenate them with space as seperater 
		//the first field is always a state code, with -100 representing invaild command,-200 means cannot connect to the catalog server,other return code are
		//subjected to specific operation
		char reply[128]; 
		memset((void*)reply,'\0',sizeof(reply));
		int socket_to_catalog_server;
		if(arg_list[0] == "trade"){
			if ((socket_to_catalog_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("catalog server server creation failed");
				exit(EXIT_FAILURE);
			}
			if ((connect(socket_to_catalog_server, (struct sockaddr*)&catalog_serv_addr, sizeof(catalog_serv_addr)))< 0) {
				perror("cannot connect to catalog server");
				memcpy((void*)reply,(const void*)"-200",5);
			}else{
				send(socket_to_catalog_server, buffer, strlen(buffer), 0);//forward trade cmd to the catalog server and get reply
				read(socket_to_catalog_server, reply, 128);
				if(reply[0]=='1'){
					//if the trade operation success
					{
						std::unique_lock<std::mutex> lock(mutex);
						file<<next_order_no++<<" "<<arg_list[1]<<" "<<arg_list[2]<<std::endl; //log transaction to the file
					}
				}
			}

			close(socket_to_catalog_server);
		}else{
			memcpy((void*)reply,(const void*)"-100",5);
		}

		send(socket_to_front, reply, strlen(reply), 0);
		close(socket_to_front);
		
	}
};
unsigned long long Job::next_order_no;
std::mutex Job::mutex; //used to provide synchronized access to "next_order_info"
struct sockaddr_in Job::catalog_serv_addr;
std::fstream Job::file;

class OrderServer :public Server<Job>{
private:
	Job BuildJob(int socket){//implements the callback to provide the job to insert into the thread pool
		return Job(socket);
	}
public:
	OrderServer(const std::string catalog_serv_ip, const unsigned catalog_serv_port,const std::string log_file){
		//the parent class constructor will be called implicitly
		Job::InitJob(catalog_serv_ip,catalog_serv_port,log_file);
	}
	void Stop(){
		Server<Job>::Stop();//must be called first because we need to stop the thread pool and make sure no more new transaction will be made before we write the order no
		Job::DeInit();
	}

};

int main(){
	OrderServer server("192.168.85.128",8080,"order_log.txt");
	server.Start("192.168.85.128",8081,10);
}