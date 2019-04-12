//Angel Solis CS370
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <list>
#include <iomanip>
#include "project3a.h" 

using namespace std;

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
