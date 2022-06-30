#include <iostream>
#include <fstream>
#include <cstring>
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
  int k;
  int l;
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
    result += test[i].k;
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
  ifstream in("inp-params.txt");
  ofstream out("EDF-Log.txt");
  ofstream out2("EDF-Stats.txt");

  int n;
  string n_temp;
  getline(in, n_temp);
  n = stoi(n_temp);
  Process process[n];//All the processes
  int end_time[n];//Time when the process has finished it's cycle
  int missed_deadlines[n];//cointing missed_deadlines

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
  { if(left.deadline!=right.deadline){return (left.deadline) >= (right.deadline);}
    else{return left.remaining_time>=right.remaining_time;} };
  priority_queue<Process, vector<Process>, decltype(cmp)> Queue(cmp);
  int flag = 0;

  int time = 0;
  int idle_time = 0;
  int temp = sum(process, n);

  /*The actual execution loop which is same as Rate monotonic one*/
  while (sum(process, n) != 0)
  {

    for (int i = 0; i < n; i++)
    {
      if (time % (process[i].period) == 0 && process[i].k > 0)//Adding the processes when they come back again
      {
        process[i].deadline = time + process[i].period;
        Queue.push(process[i]);
      }
    }

    if (Queue.empty())//CPU is idle during this
    {
      time++;
      idle_time++;
      flag = 1;
    }
    else
    {
      if (time != 0 and idle_time != 0)
      {
        out << "CPU is idle till time = " << (time - 1) << endl;
        idle_time = 0;
      }
      Process executing;//The process which is executing

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
      /*Printing the next process which is going to be executed if the previous one is terminated*/
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

      /*Actual execution of the process*/
      executing.remaining_time = executing.remaining_time - 1;
      time++;

      /*Checking whether the process we have executed is over or has reached it's deadline*/
      if (executing.remaining_time == 0 || time == executing.deadline)
      {

        if (executing.deadline == time && executing.remaining_time != 0)
        {
          out << "Process P" << executing.pid << " has reached it's deadline hence terminated at time = " << time << endl;
          missed_deadlines[find(process, executing.pid, n)]++;
          end_time[find(process, executing.pid, n)] += (time + executing.remaining_time);
        }
        else
        {
          out << "Process P" << executing.pid << " is completed at time = " << time << endl;
          end_time[find(process, executing.pid, n)] += time;
        }

        process[find(process, executing.pid, n)].k--;
        while (Queue.top().deadline <= time && !Queue.empty())
        {
          out << "Process P" << Queue.top().pid << " is terminated because it has reached it's deadline at time = " << Queue.top().deadline << endl;
          process[find(process, Queue.top().pid, n)].k--;
          end_time[find(process, Queue.top().pid, n)] += (Queue.top().deadline + Queue.top().remaining_time);
          missed_deadlines[find(process, Queue.top().pid, n)]++;
          Queue.pop();
        }
      }
      else//Pushing the process back into the queue if it has not been completed;
      {
        Queue.push(executing);
      }
    }
  }


  float waiting_times[n];//To calculate the waiting times

  /*Writing into the Stats file*/
  out2 << "No of processes entering the system is " << temp << endl;
  int temp2 = temp;
  for (int i = 0; i < n; i++)
  {
    temp2 = temp2 - missed_deadlines[i];
  }
  out2 << "No of process which executed succesfully " << temp2 << endl;
  out2<< "No of process which have missed their deadlines "<<temp-temp2<<endl;
  
  for (int i = 0; i < n; i++)
  {
    waiting_times[i] = (float)(end_time[i] - ((process[i].l * ((process[i].l - 1) * process[i].period)) / 2) - (process[i].l) * (process[i].time)) / process[i].l;
    out2 << "Process P" << process[i].pid << " has an average waiting time = " << waiting_times[i] << endl;
  }

  return 0;
}