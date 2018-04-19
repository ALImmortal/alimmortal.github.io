/***********************************************************************************
Name: Anne Liang
Course: CSCI 340
Home Project
Due: 12/10/17
Pre-conditions: From the user, we will be given three values: 
				Storage size, Frame size, number of disks
Post-condition: With the given values and commands given by user, it will 
				make modifications to the program. It will create a mock version of
				an operating system.
**********************************************************************************/
#include <iostream> //For input/output
#include <fstream> //For the files that the disks want to read
#include <vector> //To keep a vector of processes that are in the ready queue
#include <string> //For commands

using namespace std;

//Structs created for the OS
//Process: As PCB, I am only including its PID and priority
struct Process{
	int pid=0;
	int pr=-1;
	long int memAdd=-1;
	int diskN=-1;
	int programCounter=0; 
};

//CPU
struct CPU{
	vector <Process> cProc; //easier way to check if there is a process in the CPU or not
};

//Disk
struct Disk{
	vector<Process> dProc; //made it as a vector only to store one process
	string file; //In main, it will only accept .txt files
	int dNum; //Number of disk ranges from 0 to the number of disks - 1
};

bool CPUhasProc(CPU cp){
	if(cp.cProc.size()==0){
		return false;
	}
	else{
		return true;
	}
}

//Main function
int main(){
	//General inputs needed from user to start the OS
	long int m;
	long int f;
	int d;
	string command;
	long int p;
	int dN;
	string fileN;
	char dNChar;

  //For the structs created
	CPU c;
	Process pro;

	vector <Process> readyqueue;
	vector <Process> PCB; //Stores PCB of processes created
	vector <Disk> disks;
	vector <vector<Process>> pages;

	long int page;
	long int address; //For the frame and pages	

	cout<<"Welcome to the Operating System."<<endl;
	cout<<"Before we begin, please provide some values:"<<endl;

	//Memory/RAM size
	cout<<"Enter memory/RAM size (0-4000000000): ";
	cin>>m;
	while(m<0 || m>4000000000 || !cin){
		cout<<"Invalid: Enter a valid memory/RAM size: ";
		cin.clear(); //To get rid of that invalid input and ignore to avoid infinite loop
		cin.ignore();
		cin>>m;
	}
	cout<<"Memory/RAM size is saved: "<<m<<" bytes"<<endl;

	//Disk Size
	cout<<"Enter number of disks (0-10): ";
	cin>>d;
	while(d<0 || d>10 || !cin){
		cout<<"Invalid: Enter a valid number of disks: ";
		cin.clear();
		cin.ignore();
		cin>>d;
	}
	cout<<"Number of disks is saved: "<<d<<endl;
	//Resize vector for disks and set numbers for disk
	disks.resize(d);
	for(int i=0;i<disks.size();i++){
		disks[i].dNum=i;
	}
	
	//Frame/page size
	cout<<"Enter Page/Frame size (0 to memory/RAM size): ";
	cin>>f;
	while(f<0 || f>m || !cin){
		cout<<"Invalid: Enter a valid size: ";
		cin.clear();
		cin.ignore();
		cin>>f;
	}
	cout<<"Page/Frame size is saved: "<<f<<" bytes"<<endl;
	page=f;
	cout<<endl; //Just to space it out
	
	//Resize pages/address matrix
	pages.resize(page);
	for(long int j=0;j<pages.size();j++){
		pages[j].resize(page);
	}

	cout<<"----------------Main Menu-------------------"<<endl;

	//Main menu of the OS
	cout<<"We got your values! Here is a list of commands you can do with the OS:"<<endl;
	cout<<"A [priority]: Create a process with the priority (1(LOW)-5(HIGH))--> Ex: A 3 creates a new process with the priority of 3"<<endl;
	cout<<"t: terminates the process currently in CPU"<<endl;
	cout<<"d [number] [filename]: Assigns the current process in the CPU to a disk, which wants to read/write file filename"<<endl;
	cout<<"D [number]: Hard disk has finished the work for one process"<<endl;
	cout<<"m [address]: Process currently using CPU requests memory operation for the address"<<endl;
	cout<<"S r: Show what process uses CPU and what processes are in ready queue"<<endl;
	cout<<"S i: Show what processes are using disks and processes that are waiting for disk"<<endl;
	cout<<"S m: Show state of memory"<<endl;
	cout<<"'E' or 'e': Exit the program"<<endl;

	bool cont=true;
	int count=1;
	long int add=0; 
	int startC=0;

	//Make user input keep working until user asks to exit: while loop
	while(cont!=false){
    if(startC==0){cin.ignore();}
    startC++;
    cout<<endl; //Add empty line
		cout<<"Enter your command: "<<endl;
		getline(cin, command);

		if(command[0]=='A' || command[0]=='t' || command[0]=='d' || command[0]=='D' || command[0]=='m' || command[0]=='S' || command[0]=='e' || command[0]=='E'){
			//'A' command
			if(command[0]=='A'){
				if(command.length()<3 || command[1]!=' '){
					cout<<"Invalid command"<<endl;
				}
				else{
					pro.pid=count;
					char pri=command[2];
					int prio=pri-'0';
					if (prio<1 ||prio>5) {cout<< "Invalid priority. We only accept priorities from 1 to 5."<<endl;}
					else{
					  cout<<"New process created!"<<endl;
  					cout<<"Priority: "<<prio<<endl; //Print out priority
  					pro.pr=prio;
  					
  					count++; //Need to update PID everytime a new proces is created
					while(pages[0][add].pid!=0){add++;}
		  				pages[0][add].pid=pro.pid;
		  				pages[0][add].pr=pro.pr;
		  				pages[0][add].memAdd=add;
		  				cout<<"Page 0 has been loaded at address "<<add<<endl;
		  				pro.memAdd=add;
		  				add++;

  					//PCB Info
  					PCB.push_back(pro);
  					cout<<"A process control block has been created for the process."<<endl;
  					
  					if(CPUhasProc(c)==false){
  						c.cProc.push_back(pro);
  						cout<<"The process has been added to CPU."<<endl;
  						cout<<endl; //Just to space it out
  					}
  					else{
  						readyqueue.push_back(pro);
  						if(readyqueue.size()>1){
    						for(int j=0;j<readyqueue.size()-1;j++){
    						  for(int k=j+1;k<readyqueue.size();k++){
    						    if(readyqueue[j].pr<readyqueue[k].pr){
    						      Process temp;
    						      temp.pid=readyqueue[j].pid;
    						      temp.pr=readyqueue[j].pr;
    						      readyqueue[j].pid=readyqueue[k].pid;
    						      readyqueue[j].pr=readyqueue[k].pr;
    						      readyqueue[k].pid=temp.pid;
    						      readyqueue[k].pr=temp.pr;
    						    }
    						  }
    						}
  						}
  						cout<<"Process has entered ready queue."<<endl;
  						cout<<endl; //Just to space it out
  					}
					}
				}
			}
			
			//'t' command
			if(command[0]=='t'){
				cout<<"Process "<<c.cProc[0].pid<<" has been terminated from the CPU."<<endl;
				
	  			pages[c.cProc[0].memAdd/f].erase(pages[c.cProc[0].memAdd/f].begin()+(c.cProc[0].memAdd%f));
	  			
	  			Process emptyProc;
				emptyProc.pid=0;
				emptyProc.pr=-1;
				emptyProc.memAdd=-1;
				emptyProc.diskN=-1;
	  			pages[c.cProc[0].memAdd/f].insert(pages[c.cProc[0].memAdd/f].begin()+(c.cProc[0].memAdd/f), emptyProc);
	  				
	  			disks[c.cProc[0].diskN].dProc.clear();
				
				c.cProc.erase(c.cProc.begin()+0);
				if(readyqueue.size()!=0){
					c.cProc.push_back(readyqueue[0]);
					cout<<"Process "<<readyqueue[0].pid<<" has been moved from ready queue to the CPU."<<endl;
					cout<<endl; //Just to space it out
					
				readyqueue.erase(readyqueue.begin()+0);
				}

				if(readyqueue.size()==0 && c.cProc.size()==1){
					c.cProc.resize(0);
					cout<<"There are currently no processes in the readyqueue."<<endl;
				}
				if(c.cProc.size()==0){
					cout<<"There are no more processes in the system."<<endl;
				}
				
			}

			//'d' command
			if(command[0]=='d'){
				if(command.length()<4 || command[1]!=' '){
					cout<<"Invalid command"<<endl;
				}
			  else{
					dNChar=command[2];
					dN=dNChar-'0';
					fileN=command.substr(4);
					if(fileN.substr(fileN.length()-4,4)!=".txt"){cout<<"Not a valid txt file. We only accept txt files."<<endl;}
          else{
  					if(disks[dN].dProc.size()==0){
  						disks[dN].dProc.push_back(c.cProc[0]);
  						cout<<"Process "<<c.cProc[0].pid<<" is now using Disk "<<dN<<endl;
  						cout<<"Process "<<c.cProc[0].pid<<" can read/write on "<<fileN<<endl;
  						c.cProc[0].diskN=dN;
						disks[dN].file=fileN;
  						cout<<endl; //Just to space it out
  					}
            else{cout<<"Disk is busy."<<endl;}
          }
				}
			}
			
			//'D' command
			if(command[0]=='D'){
				if(command.length()<3 || command[1]!=' '){
					cout<<"Invalid command"<<endl;
					cout<<endl; //Just to space it out
				}
				else{
					if(disks[dN].dProc.size()==0){cout<<"No process is using this disk."<<endl;}
					else{
						dNChar=command[2];
						dN=dNChar-'0';
						cout<<"Process "<<disks[dN].dProc[0].pid<<" is done reading/writing."<<endl;
					
						//Get rid of the process from the disk
						disks[dN].dProc.erase(disks[dN].dProc.begin()+0);
					}
				}
			}

			//'m' command
			if(command[0]=='m'){
				string addNumChar=command.substr(2);
				int addN=stoi(addNumChar);
				if(command.length()<3 || command[1]!=' '){
					cout<<"Invalid command"<<endl;
					cout<<endl; //Just to space it out
				}
				else if(addN>m){
					cout<<"Invalid address."<<endl;
				}
				else{
					for(long int r=0;r<pages.size();r++){
					  for(long int t=0;t<pages[r].size();t++){
					    if(pages[r][t].pid==c.cProc[0].pid){
						pages[r][t].pid=0;
						pages[r][t].pr=-1;
						pages[r][t].memAdd=(r*f)+t;
						pages[r][t].diskN=-1;
					    }
					  }
					}
					int pageN=addN/f;
					int mod=addN%f;
					c.cProc[0].memAdd=addN;
					
					if(pages[pageN][mod].pid==c.cProc[0].pid && pages[pageN][mod].pr==c.cProc[0].pr){
						int leastUsedPage=0;
						for(int z=0;z<pages.size();z++){
							int count=0;
							for(int z1=0;z1<pages[z].size();z1++){
								if(pages[z][z1].pid==0 && pages[z][z1].pr==-1){
									count++;
								}
							if(count==pages[z].size()){
								cout<<"Address "<<addN<<" was taken, so it has been moved to Page "<<z<<"(st/th) address of the page."<<endl;
								pageN=z;
								break;
							}
							}
						}
					}
					pages[pageN][mod].pid=c.cProc[0].pid;
					pages[pageN][mod].pr=c.cProc[0].pr;
					pages[pageN][mod].diskN=c.cProc[0].diskN;
					cout<<"You have entered address: "<<addN<<endl; 
					cout<<"Process "<<c.cProc[0].pid<<" has been stored on Page "<<pageN<<" on "<<mod<<"(st/th) address of the page."<<endl;
					cout<<endl; //Just to space it out 
				}
			}

			//'S' command
			if(command[0]=='S'){
				if(command.length()<3 || command[1]!=' '){
					cout<<"Invalid command"<<endl;
					cout<<endl; //Just to space it out
				}
				else{
					char command2=command[2];
					//'r' command
					if(command2=='r'){
						if(c.cProc.size()==0){cout<<"No more processes."<<endl;}
						else{
							cout<<"Process "<<c.cProc[0].pid<<" with priority "<<c.cProc[0].pr<<" is currently using the CPU."<<endl;
							for(int a=0;a<readyqueue.size();a++){
								cout<<"Process "<<readyqueue[a].pid<<" with priority "<<readyqueue[a].pr<<" is current in the ready queue."<<endl;
							}
						}	
					} 
					//'i' command
					if(command2=='i'){
						for(int b=0;b<disks.size();b++){
							if(disks[b].dProc.size()==0){cout<<"Disk "<<b<<": No disks are being used."<<endl;}
							if(disks[b].dProc.size()!=0){
								cout<<"Disk "<<b<<": Process "<<disks[b].dProc[0].pid<<": "<<disks[b].file<<endl;
							}
						}
						cout<<endl; //Just to space it out
					} 
					//'m' command
					if(command2=='m'){
						for(long int x=0;x<pages.size();x++){
							for(long int y=0;y<pages[0].size();y++){
								if(pages[x][y].pid!=0){
									cout<<pages[x][y].memAdd<<": Process "<<pages[x][y].pid<<" with priority "<<pages[x][y].pr<<" is stored on Page "<<x<<" on "<<y<<"(st/rd/th) part of the page."<<endl;
								}
							}
						}
					} 
				}
			}		

			//'e' command - I made this so users can choose when to end program instead of force shutting program
			if(command=="e"){
				cout<<"You are done using the OS."<<endl;
				cont=false;
			}
		}

		else{
			cout<<"Invalid command. Please re-enter a valid command."<<endl;
		}
	}
}




