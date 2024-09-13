#include <bits/stdc++.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <cstdlib>

using namespace std;
int N,M;
int w,x,y;

const int numPrintStations = 4;
sem_t printSemaphores[numPrintStations];

const int numBindingStations = 2;
sem_t bindingSemaphores ;

sem_t entryBookMutex;
sem_t entryBookReaderMutex;
int numReaders = 0;
int numSubmission = 0;
chrono::high_resolution_clock::time_point start;


int *numGroup;
sem_t *semaphore;
sem_t *semaphore2;
int *status;
pthread_mutex_t *groupmutex;
pthread_mutex_t printmutex;
int getTime() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        return (int)duration.count();
    }

void test(int i)
{
    if(status[i] == 1)
    {
        int ps = (i%4)+ 1;
        bool v=false;
        for(int j=1; j<=N; j++)
        {
            if((j%4)+1==ps && status[j]==2)
            {
                v=true;
                break;
            }
        }
        if(!v)
        {
            status[i]=2;
            sem_post(&semaphore[i]);
        }


    }
}

void takePrinter(int i)
{
    sem_wait(&printSemaphores[(i%4)+1]);
    status[i] = 1;
    test(i);
    sem_post(&printSemaphores[(i%4)+1]);
    sem_wait(&semaphore[i]);

}
void leavePrinter(int i)
{
    int printStation = (i%4)+1 ;

    sem_wait(&printSemaphores[printStation]);
    status[i] = 0;
    int group_id = ceil(i/M);
    int lastmember = group_id*M;
    int firstmember = lastmember-M+1;

    for(int j=firstmember; j<=lastmember; j++)
    {
        if(printStation == ((j%4)+1))
            test(j);
    }

    for(int j=1; j<=N; j++)
    {
        if((j%4)+1 == printStation)
        {
            test(j);
        }
    }

    sem_post(&printSemaphores[printStation]);
}


void* student(void* arg)
{
    int id = *((int*) arg);
    int station = (id % numPrintStations)+1;

    //sleep(rand());

    takePrinter(id);

    string s = "Student "+to_string(id) + " has arrived at printer with printer id "+to_string(station)+ " at time "+to_string(getTime());
    pthread_mutex_lock(&printmutex);
    cout << s << endl;
    pthread_mutex_unlock(&printmutex);
    sleep(w);

    string s1 = "Student " +to_string(id)  +" has finished printing at printer with printer_id  " +to_string(station)+ " at time "+to_string(getTime());
    pthread_mutex_lock(&printmutex);
    cout << s1 << endl;
    pthread_mutex_unlock(&printmutex);
    leavePrinter(id);

    int group_id = ceil(id/M);
    pthread_mutex_lock(&groupmutex[group_id]);
    int lastmember = group_id*M;
    numGroup[group_id]++;
    if(numGroup[group_id] == M)
    {
        sem_post(&semaphore2[group_id]);
    }
    pthread_mutex_unlock(&groupmutex[group_id]);
    return NULL;
}

void* groupLeader(void* arg)
{
    int group_id = *((int*) arg);

    sem_wait(&semaphore2[group_id]);


    string s = "Group  " +to_string(group_id) + " is using binding station "+" at time "+to_string(getTime());
    pthread_mutex_lock(&printmutex);
    cout << s <<endl;
    pthread_mutex_unlock(&printmutex);

    // Simulate binding
    sleep(x);


    string s1 = "Group  " + to_string(group_id) + " finished using binding station "+" at time "+to_string(getTime());;
    pthread_mutex_lock(&printmutex);
    cout << s1<<endl;
    pthread_mutex_unlock(&printmutex);
    return NULL;



}

void* groupLeaderSubmission(void* arg)
{
    int id = *((int*) arg);

    sem_wait(&entryBookMutex);
    // Simulate writing in the entry book
    string s = "Group  " +to_string(id) + " is writing in the entry book"+ " at time "+to_string(getTime());;
    pthread_mutex_lock(&printmutex);
    cout << s << endl;
    pthread_mutex_unlock(&printmutex);
    sleep(y);
    string s1 = "Group  " + to_string(id) + " finished writing in the entry book"+ " at time "+to_string(getTime());;
    pthread_mutex_lock(&printmutex);
    cout << s1 << endl;
    pthread_mutex_unlock(&printmutex);

    sem_post(&entryBookMutex);

    return NULL;
}

void* staffMember(void* arg)
{
    int id = *((int*) arg);

    while(true)
    {
        sem_wait(&entryBookReaderMutex);

        ++numReaders;
        if (numReaders == 1)
        {
            sem_wait(&entryBookMutex);
        }

        sem_post(&entryBookReaderMutex);

        // Simulate reading the entry book
        string s = "Staff  " +to_string(id) + " is reading the entry book" +" at time "+to_string(getTime());
        pthread_mutex_lock(&printmutex);
        cout << s << endl;
        pthread_mutex_unlock(&printmutex);
        sleep(10);
        string s1 = "Staff  "+to_string(id) + " finished reading the entry book"+" at time "+to_string(getTime());
        pthread_mutex_lock(&printmutex);
        cout << s1 << endl;
        pthread_mutex_unlock(&printmutex);

        sem_wait(&entryBookReaderMutex);

        --numReaders;
        if (numReaders == 0)
        {
            sem_post(&entryBookMutex);
        }

        sem_post(&entryBookReaderMutex);

        if(numSubmission==N/M)
            break;
    }

    return NULL;
}

int main()
{
    start = std::chrono::high_resolution_clock::now();
    cin>>N>>M;
    cin>>w>>x>>y;

    const int numStudents =  N;
    const int numGroupLeaders = N/M;
    const int numStaffMembers = 2;

    numGroup = new int[numGroupLeaders];
    groupmutex = new pthread_mutex_t [numGroupLeaders+1];
    semaphore = new sem_t [N+1];
    semaphore2 = new sem_t [numGroupLeaders+1];

    status = new int [N+1];
    for(int i = 1; i<=N; i++)
    {

        status[i] = 0;
    }

    pthread_mutex_init(&printmutex, NULL);

    vector<pthread_t> students;
    vector<pthread_t> leaders;
    vector<pthread_t> staff;

    sem_init(&bindingSemaphores,0,2);
    for (int i = 1; i <= N; i++)
    {
        sem_init(&semaphore[i], 0, 0);
    }

    for (int i = 1; i <= numPrintStations; i++)
    {
        sem_init(&printSemaphores[i], 0, 1);
    }

    /* for (int i = 1; i <= numBindingStations; i++)
     {
         sem_init(&bindingSemaphores[i - 1], 0, 1);
     }*/

    sem_init(&entryBookMutex, 0, 1);
    sem_init(&entryBookReaderMutex, 0, 1);

    for (int i = 1; i <= numGroupLeaders; ++i)
    {
        sem_init(&semaphore2[i], 0, 0);
        pthread_mutex_init(&groupmutex[i],NULL);
    }

    for (int i = 1; i <= numStudents; ++i)
    {
        int* studentId = new int(i);
        pthread_t studentThread;
        pthread_create(&studentThread, NULL, student, studentId);
        students.push_back(studentThread);
    }

    for (int i = 1; i <= numGroupLeaders; ++i)
    {
        int* leaderId = new int(i);
        pthread_t leaderThread;
        pthread_create(&leaderThread, NULL, groupLeader, leaderId);
        leaders.push_back(leaderThread);

        int* submissionId = new int(i);
        pthread_t submissionThread;
        pthread_create(&submissionThread, NULL, groupLeaderSubmission, submissionId);
        leaders.push_back(submissionThread);
    }

    for (int i = 1; i <= numStaffMembers; ++i)
    {
        int* staffId = new int(i);
        pthread_t staffThread;
        pthread_create(&staffThread, NULL, staffMember, staffId);
        staff.push_back(staffThread);
    }
    for (pthread_t &studentThread : students)
    {
        pthread_join(studentThread, NULL);
    }

    for (pthread_t &leaderThread : leaders)
    {
        pthread_join(leaderThread, NULL);
    }

    for (pthread_t &staffThread : staff)
    {
        pthread_join(staffThread, NULL);
    }

    for (int i = 1; i <= numPrintStations; ++i)
    {
        sem_destroy(&printSemaphores[i]);
    }

    /* for (int i = 1; i <= numBindingStations; ++i)
     {
         sem_destroy(&bindingSemaphores[i - 1]);
     }*/

    sem_destroy(&entryBookMutex);
    sem_destroy(&entryBookReaderMutex);

    return 0;
}
