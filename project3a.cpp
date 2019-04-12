//Angel Solis CS370
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <list>
#include <iomanip>

using namespace std;

class process
{
public:
	process();
	process(int pd,int nic,int ari,int cpub,int cpubr[],int iobr[],int prio,int times);
	bool operator < (const process& right)const{return(iobursts[ioburstdone]< right.getioleft());}
	int getpid()const {return pid;}
	int getts()const {return timeslice;}
	int getnice()const {return nice;}
	int getprio()const {return priority;}
	int getend()const {return endtime;}
	int getarrival()const {return arrival;}
	int getcpubnum()const {return cpuburstsnum;}
	int getcpuleft()const {return cpubursts[cpuburstdone];}
	int getburstdone()const {return cpuburstdone;}
	void inccpudone() {cpuburstdone++;}
	void inciodone() {ioburstdone++;}
	int getiodone()const {return ioburstdone;}
	int getioleft()const {return iobursts[ioburstdone];}
	int gettotalio()const {return totalio;}
	int gettotalcpu() const {return totalcpu;}
	void updatets(int bonus);
	void resetcpu() {cpuburstsnum=0;}
	void setend(int end){endtime=end;}
	void dects();
	void decio();
	
	
private:
	int pid, nice, starttime, arrival, endtime, original_priority,priority, timeslice, cpuburstsnum, cpubursts[100],iobursts[100], totalio, totalcpu, cpuburstdone, ioburstdone;

};
process::process()
{
	pid=0;
	nice=0;
	starttime=0;
	arrival=0;
	timeslice=0;
	cpuburstdone = 0;
	ioburstdone = 1; //done because first element will be skipped
	totalcpu = 0;
	totalio =0;
}
process::process(int pd,int nic,int ari,int cpub,int cpubr[],int iobr[],int prio,int times)
	{
		pid = pd;
		nice = nic;
		arrival = ari;
		cpuburstdone = 0;
		ioburstdone = 1;
		totalcpu = 0;
		totalio =0;
		cpuburstsnum = cpub;
	timeslice = times;
	priority = prio;
	original_priority=prio;
		for (int j = 0; j < cpub;j++)
			{
				if (j==0)//first just io
				{
					cpubursts[j] = cpubr[j];
				
				}			
				else //all others must be cpu and io bursts
				{
					
					iobursts[j] = iobr[j];
					cpubursts[j] = cpubr[j];
				}
			}
	}

void process::dects()
{// Decrements timeslice and amount of burst left
	timeslice--;
	cpubursts[cpuburstdone]--;
	totalcpu++;
} 

void process::decio() 
{//decrements io burst left
	iobursts[ioburstdone]--;
	totalio++;
}
 
void process::updatets(int bonus)
{
	// updates priority using bonus
	if (original_priority + bonus >150 )	//done because values do not always fall in the correct range
		priority = 150;
	else if(original_priority + bonus <100)
		priority =100;
	else
	priority =   original_priority + bonus;
	int timesl=((int)(((1-(priority/150.0))*445+.5)+5));
	if(timesl>450)	//ensure times are in range
		timeslice=450;
	else if(timesl<5)
		timeslice=5;
	else
		timeslice=timesl;
	
}





struct byari //sort perameter for startqueue: arival time
{
	bool operator()(const process& left, const process& right) const
	{
		return (left.getarrival() >= right.getarrival());
	}
};
struct byprio // sort peramerter for ready queue: priority
{
	bool operator()(const process& left, const process& right) const
	{
		return (left.getprio() >= right.getprio());
	}
};
struct byend // sort perameter for finished queue: finish time
{
	bool operator()(const process& left, const process& right) const
	{
		return (left.getend() >= right.getend());
	}
};
void reader (priority_queue<process, vector<process>,byari> &startqueue, int &pids);
void decios (list<process> &);
void calcbonus(process& running);
void printcalcs(priority_queue<process, vector<process>,byend> &finished);
int main()
{
	int clk = 0,pids=0,bonus;
	priority_queue<process, vector<process>,byari> startqueue; 
	priority_queue<process, vector<process>,byprio> *tempptr, *active =new priority_queue<process, vector<process>,byprio>;
	priority_queue<process, vector<process>,byprio> *expired =new priority_queue<process, vector<process>,byprio>;
	priority_queue<process, vector<process>,byend> finished;
	process running,iotemp;
	running.resetcpu();
	list<process> iowait;
	reader(startqueue, pids);
	
	while(1)
	{
		while (!startqueue.empty() && startqueue.top().getarrival() == clk) //moves start to arival when clk
		{
			cout << "[" << clk << "] <" << startqueue.top().getpid()<<"> Enters ready queue (Priority: " << startqueue.top().getprio() << ", TimeSlice: " << startqueue.top().getts()<< ")\n";
			(*active).push(startqueue.top());
			startqueue.pop();
		}
		if(!(*active).empty() && running.getcpubnum()==0) // Moves new process in if cpu is empty and there is a process in the active queue
		{
			running = (*active).top();
			(*active).pop();
			cout << "[" << clk << "] <" <<running.getpid()<<"> Enters the cpu\n";
		}
		if(!(*active).empty() && (*active).top().getprio() < running.getprio()) //preempt currently running process if a newer process has a lower priority
		{
			cout << "[" << clk << "] <" <<(*active).top().getpid()<<"> Preempts Process " << running.getpid()<<endl;
			(*active).push(running);
			running = (*active).top();
			(*active).pop();
		}
		if (running.getcpubnum()!=0) //decrement running cpu time
			running.dects();
		if (!iowait.empty())	//decrement all io
			decios(iowait);
		if (running.getcpubnum()!=0) //if process is in running
		{
			if(running.getcpuleft()==0) // if cpuburst is over
			{
				running.inccpudone();	//incriment cpuburt counter to show that aburst has been finished
			if(running.getcpubnum()== running.getburstdone())	// if cpuburst counter = #of cpu bursts
				{
					cout << "[" << clk << "] <" <<running.getpid()<<"> Finishes and moves to the Finished Queue"<<endl;
					running.setend(clk);
					finished.push(running);
					running.resetcpu();    //show running as empty
				}
				else //otherwise its io
				{
					cout << "[" << clk << "] <" <<running.getpid()<<"> Moves to the IO Queue " <<endl;
					
					iowait.push_back(running);
					iowait.sort();
					running.resetcpu();
				}
			}
		if(running.getts() == 0) // if timeslice runs out and there is aprocess in the running state
			{
				calcbonus(running);//calculate the bonuses
				cout << "[" << clk << "] <" <<running.getpid()<<"> Finishes its time slice and moves to the Expired Queue (Priority: " << running.getprio() << ", TimeSlice: " <<running.getts()<<")"<<endl;
				(*expired).push(running);
				running.resetcpu();//show running as empty
			}
		}
		while(!iowait.empty() && iowait.front().getioleft() == 0) // if the top io is done check all others too
		{
			iotemp = iowait.front();
			iotemp.inciodone();	
			if (iotemp.getts() == 0) // if io is done and timeslice was done
			{
				calcbonus(iotemp);
				cout << "[" << clk << "] <" <<iotemp.getpid()<<"> Finishes IO and moves to the Expired Queue {Priority: " << iotemp.getprio() << ", TimeSlice: " <<iotemp.getts()<<">"<<endl;
				(*expired).push(iotemp);
				iowait.pop_front();
				iowait.sort();
			}
			else if(iotemp.getts() >0) //if io done but process still has timeslice
			{		//move to active queue
					(*active).push(iotemp);
					iowait.pop_front();
					iowait.sort();
					cout << "[" << clk << "] <" <<iotemp.getpid()<<"> Finishes IO moves to the Ready Queue " <<endl;
			}
		}
		if (running.getcpubnum()==0&& iowait.empty()&&(*active).empty()&&(*expired).empty() &&startqueue.empty()) //if everything is empty exit
			break;
		if((*active).empty() &&!(*expired).empty() && running.getcpubnum()==0)	//swap if active empty
		{
			cout << "[" << clk << "] " <<"*** Queue Swap" <<endl;
			tempptr=active;
			active = expired;
			expired = tempptr;
		}
		clk++;
	}
printcalcs(finished);
return 0;	
}

void reader(priority_queue<process, vector<process>,byari> &startqueue, int &pids)
{//reads in string and pushes values onto startqueue
	string line;
	int nice, starttime, cpuburnum,cpuburst[100],ioburst[100],prio,times;
	while (1)	
	{
		getline(cin,line);
		if (line == "***")
			break;
		istringstream iss (line);
		iss >> nice >> starttime >> cpuburnum;

		for (int j = 0; j < cpuburnum;j++)
		{
			if (j==0)
			{
				iss >> cpuburst[j];
				
			}			
			else 
			{
				iss >>ioburst[j];
				iss >> cpuburst[j];
				
			}
		}
	prio = ((int)((((nice+20) / 39.0) * 30 + 0.5)+105));
	times=((int)((1-prio/150.0)*445+.5)+5);
	
	startqueue.push( process(pids, nice, starttime, cpuburnum , cpuburst, ioburst, prio,times));
	pids++;
	}
}

void decios (list<process> &ios)
{//decrements all io processes
	for (list<process>::iterator i = ios.begin();;i++)
	{
	if (i == ios.end())
		break;
	(*i).decio();	
	}
}

void calcbonus(process& running)
{
	int bonus;
	if (running.gettotalcpu() < running.gettotalio())
		bonus = ((int)(((1- running.gettotalcpu()/((double)running.gettotalio()))*(-10))-.5));
	else
		bonus = ((int)(((1- running.gettotalio()/((double)running.gettotalcpu()))*10)+.5));
	running.updatets(bonus);
}

void printcalcs(priority_queue<process, vector<process>,byend> &finished)
{//prints out final calculations
	int tatsum=0,wtsum=0,pcount=0,indtat=0,indwt=0;
	double cutsum=0.0,indcut=0.0,avgtat=0.0,avgwt=0.0,avgcut=0.0;
	process test;
	while (!finished.empty())
	{
		test = finished.top();
		finished.pop();
		pcount++;
		indtat = test.getend()-test.getarrival();
		tatsum += indtat;

		indwt = indtat-test.gettotalcpu()-test.gettotalio();
		wtsum += indwt;

		indcut = ((double)test.gettotalcpu())/indtat;
		cutsum +=indcut;
		
		cout <<"Pid: " << test.getpid() << "\nTurn around time = " << indtat <<"\nTotal CPU time = "<< test.gettotalcpu()
		     << "\nWaiting Time = "<<indwt<< "\nPercentage of CPU utilization time = " <<fixed<<setprecision(1)<< 100*indcut<<"%\n\n";
	}
	avgtat = ((double)tatsum)/pcount;
	avgwt = ((double)wtsum)/pcount;
	avgcut = ((double)cutsum)/pcount;
	cout <<fixed<<setprecision(3)<<"Average Turn arround time: " << avgtat<< "\nAverage Waiting Time: " << avgwt << "\nAverage Percentage of CPU utilization time: "<<  100*indcut<<"%\n";
}



