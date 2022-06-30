#include <iostream>
#include <fstream>
#include <cstring>
#include<iomanip>
#include <sstream>
#include <set>
#include <queue>
#include <cstdlib>
using namespace std;
/*We create a struct named process which has the following terms*/
struct Process
{
  int pid;
  int time;
  int period;
  int l;//No of process remaining of this type
  int k;//Total no of process entering
  int remaining_time;
  int deadline;
};

typedef struct Process Process;

/*Function to keep track of total number of processes remaining*/
int sum(Process test[], int n)
{
  int result = 0;
  for (int i = 0; i < n; i++)
  {
    result += test[i].l;
  }
  return result;
}

/*Function to find the index of the process in the array
based on the process_id*/
int find(Process p[], int pid, int n)
{
  for (int i = 0; i < n; i++)
  {
    if (p[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
}

int main()
{
  ifstream in("inp-params.txt");//Opeining input file
  ofstream out("RM-Log.txt");//Opening output file
  ofstream out2("RM-Stats.txt");//Opening output file 2

  int n;
  string n_temp;
  getline(in, n_temp);
  n = stoi(n_temp);
  Process process[n];
  int end_time[n];
  int missed_deadlines[n];
  /*Taking the input from the opened file*/
  for (int i = 0; i < n; i++)
  {
    string temp;
    getline(in, temp);
    stringstream X(temp);
    string token;
    getline(X, token, ' ');
    process[i].pid = stoi(token);
    getline(X, token, ' ');
    process[i].time = stoi(token);
    getline(X, token, ' ');
    process[i].period = stoi(token);
    getline(X, token, ' ');
    process[i].k = stoi(token);
    process[i].l = process[i].k;
    process[i].remaining_time = process[i].time;
    end_time[i] = 0;
    missed_deadlines[i] = 0;
  }

  /*Printing the initial data into the Logs file */
  for (int i = 0; i < n; i++)
  {
    out << "Process P" << process[i].pid << ": processing time = " << process[i].time << "; deadline =  " << process[i].period;
    out << "; period = " << process[i].period << " joined the system at time = 0" << endl;
  }

  /*Creating a min queue to store all the process according to
  their priority*/

  /*Lambda Comparator*/
  auto cmp = [](Process left, Process right)
  { if(left.pid!=right.pid){return (left.period) >= (right.period);}
    else{return left.remaining_time>=right.remaining_time;} };
  priority_queue<Process, vector<Process>, decltype(cmp)> Queue(cmp);
  int flag = 0;

  int time = 0;
  int idle_time = 0;
  int proc_entering = sum(process, n);//Stores the number of process entering the system

  /*Now the actual execution loop*/
  while (sum(process, n) != 0)
  {
    Process executing;//Process which gets executed at this instant of time
    for (int i = 0; i < n; i++)
    {
      if (time % (process[i].period) == 0 && process[i].l > 0)//Pushing the process which have come again.
      {
        process[i].deadline = time + process[i].period;
        Queue.push(process[i]);
      }
    }


    if (Queue.empty())
    {
      time++;
      idle_time++;
      flag = 1;//Flag to print the CPU idle times
    }
    else
    {
      if (time != 0 and idle_time != 0)
      {
        out << "CPU is idle till time = " << (time - 1) << endl;
        idle_time = 0;
      }
      
      if (time == 0)
      {
        executing = Queue.top();
        out << "Process P" << Queue.top().pid << " starts executing at time = 0" << endl;
      }
      
      /*Premption checks*/
      if (executing.pid != Queue.top().pid && executing.remaining_time != 0 && executing.deadline != time)
      {
        out << "Process P" << executing.pid << " is prempted by "
            << "Process P" << Queue.top().pid << " at time = " << time << " remaining process time = " << executing.remaining_time << endl;
        if (Queue.top().time == Queue.top().remaining_time)
        {
          out << "Process P" << Queue.top().pid << " starts executing at time = " << time + 1 << endl;
        }
        else
        {
          out << "Process P" << Queue.top().pid << " resumes executing at time = " << time + 1 << endl;
        }
      }


      /* Previous Process has reached it's deadline while executing or is executed sucessfully*/
      if (executing.remaining_time == 0 || time == executing.deadline)
      {
        if (!Queue.empty() && idle_time == 0)
        {
          if (Queue.top().time == Queue.top().remaining_time)
          {
            if (flag == 0)
            {
              out << "Process P" << Queue.top().pid << " starts executing at time = " << time + 1 << endl;
            }
            else
            {
              out << "Process P" << Queue.top().pid << " starts executing at time = " << time << endl;
              flag = 0;
            }
          }
          else
          {

            out << "Process P" << Queue.top().pid << " resumes executing at time = " << time + 1 << endl;
          }
        }
      }

      executing = Queue.top();
      Queue.pop();
      /*Executing the process*/
      executing.remaining_time = executing.remaining_time - 1;
      time++;

      /*The process is executed sucessfully or has reached it's deadline*/

      if (executing.remaining_time == 0 || time == executing.deadline)
      {

        if (executing.deadline == time && executing.remaining_time != 0)//Reached deadline
        {
          out << "Process P" << executing.pid << " has reached it's deadline hence terminated at time = " << time << endl;
          missed_deadlines[find(process, executing.pid, n)]++;//Missed deadline
          end_time[find(process, executing.pid, n)] += (time+executing.remaining_time);//End times to calculate waiting time
        }
        else//Executed sucessfully
        {
          out << "Process P" << executing.pid << " is completed at time = " << time << endl;
          end_time[find(process, executing.pid, n)] += time;
        }

        process[find(process, executing.pid, n)].l--;//Subtracting the completed process

        /*Now we need to remove processes which have excedded their deadline but were not removed from
        the queue*/
        while (Queue.top().deadline <= time && !Queue.empty())
        {
          out << "Process P" << Queue.top().pid << " now comes to the top and is terminated because it has reached it's deadline at time = " << Queue.top().deadline << endl;
          process[find(process, Queue.top().pid, n)].l--;
          end_time[find(process, Queue.top().pid, n)] += (Queue.top().deadline+Queue.top().remaining_time);
          missed_deadlines[find(process, Queue.top().pid, n)]++;
          Queue.pop();
        }
      }
      else//If the process has not completed and not reached it's deadline we have to push it back
      {
        Queue.push(executing);
      }
    }
  }

  /*Printing into the stats file*/

  float waiting_times[n];
  out2 << "No of processes entering the system is " << proc_entering << endl;

  int temp2 = proc_entering;
  for (int i = 0; i < n; i++)
  {
    temp2 = temp2 - missed_deadlines[i];//Counting the total number of process which have missed their deadlines
  }
  out2 << "No of process which executed succesfully " << temp2 << endl;
  out2<<"No of process which missed their deadlines "<< proc_entering-temp2<<endl;

  for (int i = 0; i < n; i++)
  {
    waiting_times[i] = (float)(end_time[i] - ((process[i].k * ((process[i].k - 1) * process[i].period)) / 2) - (process[i].k) * (process[i].time)) / (float)process[i].k;
    out2 << "Process P" << process[i].pid << " has an average waiting time = " <<waiting_times[i] << endl;
  }

  return 0;
}