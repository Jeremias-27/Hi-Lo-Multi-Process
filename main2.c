#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

// flag variables
int lowsignl = 0;
int highsignl = 0;
int p1signl = 0;
int p2signl = 0;
int ctrlc = 0;

// function declarations
int checkError(int, const char*);
void signlHndlrP(int);
void signlHndlrC(int);
void child(int);
void parent(struct sigaction, pid_t, pid_t);

int main(int argc, char* argv[])
{
    struct sigaction saP; // parent sigaction
    
    saP.sa_handler = signlHndlrP; // setting our Parent signal handler
    saP.sa_flags = 0; // no flags
    sigemptyset(&saP.sa_mask); // initializing parent mask to be empty

    // setting the disposition of our signals
    checkError(sigaction(SIGCHLD, &saP, NULL), "sigaction");
    checkError(sigaction(SIGUSR1, &saP, NULL), "sigaction");
    checkError(sigaction(SIGUSR2, &saP, NULL), "sigaction");

    srand(time(NULL)); // seeding random number generator

    pid_t child1, child2; // processes

    child1 = checkError(fork(), "child"); // fork child process 1

    if(child1 == 0) // if we are in child process enter as player 1
    {
        child(1); // player function
    }

    child2 = checkError(fork(), "child"); // fork child process 2

    if(child2 == 0) // if we are in child process enter as player 2
    {
        child(2); // player function
    }

    parent(saP, child1, child2); // referee function

    return 0;
}

// val == -1 we will output what whent wrong and exit the program,
// but if errno == EINTR we will just return the value
int checkError(int val, const char* msg)
{
     if(val == -1)
    {
        if(errno == EINTR) {return val;}
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

// parent signal handler
void signlHndlrP(int sig1)
{

    if(sig1 == SIGCHLD)
    {
        pid_t p;
        while((p = waitpid(-1, NULL, WNOHANG)) > 0) // clean up after the child
        {
            char* msg = "Handling exit of child\n"; // inform that we are cleaning up after child
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        
        if(p == -1) // finished cleaning after child or waitpid failed
        {
            if(errno = ECHILD) // clean up is finished now exiting parent
            {
                char* msg2 = "No children remaining, exiting the parent...\n";
                write(STDOUT_FILENO, msg2, strlen(msg2));
                exit(EXIT_SUCCESS);
            }
            else // failed to clean up after child
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        }
    }

    // if the signal SIGINT is sent we will exit the children and exit the parent
    if(sig1 == SIGINT)
    {
        kill(0, SIGTERM); exit(EXIT_SUCCESS);
    }

    // If the signal SIGUSR1 is sent flag that it has been received
    if(sig1 == SIGUSR1)
    {
       p1signl = 1;
    }

    // If the signal SIGUSR2 is sent flag that it has been received
    if(sig1 == SIGUSR2)
    {
        p2signl = 1;
    }
}

void signlHndlrC(int sig2)
{
     // if the signal SIGTERM is sent we will exit out of the child
    if(sig2 == SIGTERM)
    {
        char* msg = "Child exiting..\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        exit(EXIT_SUCCESS);
    }

    // if the parent sends SIGUSR1 flag that it has been received
    if(sig2 == SIGUSR1)
    {
        lowsignl = 1;
    }

    // if the parent sends SIGUSR2 flag that it has been received
    if(sig2 == SIGUSR2)
    {
        highsignl = 1;
    }

    // if the parent sends SIGINT flag that it has been received
    if(sig2 == SIGINT)
    {
        ctrlc = 1;
    }
}

// player function to make guesses
void child(int id)
{
    // minimum and maximum variables for range to guess
    int min1, max1, min2, max2; 
    int fd; // file descriptor
    struct sigaction saC; // child sigaction

    saC.sa_handler = signlHndlrC; // setting our child signal handler
    saC.sa_flags = 0; // no flags
    sigemptyset(&saC.sa_mask); // initializing child mask to be empty

    // setting the disposition of our signals
    checkError(sigaction(SIGTERM, &saC, NULL), "sigaction");
    checkError(sigaction(SIGUSR1, &saC, NULL), "sigaction");
    checkError(sigaction(SIGUSR2, &saC, NULL), "sigaction");
    checkError(sigaction(SIGINT, &saC, NULL), "sigaction");

    pause(); // wait for parent to signal we can start

    while(1)
    {
        // initialize our min and max variables for child 1 and child 2
        min1 = 1;
        max1 = 100;

        min2 = 1;
        max2 = 100;

        if(id == 1) // if player 1
        {
            kill(getppid(), SIGUSR1); //signal to parent we are making our guess
            
            while(1)
            {
                // intialize/reinitialize signals from parent
                lowsignl = 0;
                highsignl = 0;
                ctrlc = 0;

                // player 1's strategy is the average of its low and high bounds
                int guess = (min1 + max1) / 2; // make our guess

                // write guess to player 1 file
                fd = checkError(open("p1.dat", O_RDONLY | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR), "opening");
                checkError(write(fd, &guess, sizeof(int)), "writing");
                close(fd); // close file

                sleep(1); // sleep for a second

                kill(getppid(), SIGUSR1); // signal to parent we have made our guess
                
                while(1) // wait for parent feedback
                {
                    if(lowsignl || highsignl || ctrlc) {break;}
                    pause();
                }

                if(lowsignl) // if parent signaled guess is too low set min of bounds to current guess
                {
                    min1 = guess;
                }
                else if(highsignl) // if parent signaled guess is too high set max of bounds to current guess
                {
                    max1 = guess;
                }
                else if(ctrlc) // if parent signaled to restart break out of guess loop
                {
                    break;
                }
            }
        }
        else // player 2
        {
            kill(getppid(), SIGUSR2); // signal to parent we are making our guess

            while(1)
            {
                // intialize/reinitialize signals from parent
                lowsignl = 0;
                highsignl = 0;
                ctrlc = 0;

                // Player 2's strategy is to guess a random number between its low and high bounds 
                int guess2 = rand() % (max2 + 1 - min2) + min2; // make our guess

                // write guess to player 2 file
                fd = checkError(open("p2.dat", O_RDONLY | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR), "opening");
                checkError(write(fd, &guess2, sizeof(int)), "writing");
                close(fd); // close file

                sleep(1); // sleep for a second

                kill(getppid(), SIGUSR2); // signal to parent we have made our guess
                
               while(1) // wait for parent feedback
                {
                    if(lowsignl || highsignl || ctrlc) {break;}
                    pause();
                }
                
                if(lowsignl) // if parent signaled guess was too low set min of our bounds to current guess
                {
                    min2 = guess2;
                }
                else if(highsignl) // if parent signaled guess was too high set max of our bounds to current guess
                {
                    max2 = guess2;
                }
                else if(ctrlc) // if parent signaled to restart break out of guess loop
                {
                    break;
                }
            }
        }  
    }
}

// referee function to coordinate game and check each players guesses
void parent(struct sigaction saP, pid_t chld1, pid_t chld2)
{
    int fd; // file descriptor
    int g1, g2; // variables to store player 1 and player 2 guess
    int target; // variable to store target number to guess
    int signl[2] = {0,0}; // array for which signal to send
    
    // counter for player 1 and player 2 wins and number of ties
    int p1count = 0;
    int p2count = 0;
    int tiecount = 0;

    // set disposition of our signal
    checkError(sigaction(SIGINT, &saP, NULL), "sigaction");

    printf("Game starting...\n"); // game starting

    sleep(5); // sleep for 5 seconds

    // initialize signals from player 1 and player 2 to 0
    p1signl = 0;
    p2signl = 0;

    // signal to player 1 and player 2 they can start
    kill(chld1, SIGUSR1);
    kill(chld2, SIGUSR2);

    for(int i = 0; i < 10; i++)
    {
        printf("\nGame %d\n", i+1); // Game number

        // wait for players to signal they are ready
        while(!p1signl || !p2signl)
        {
            pause();
        }

        // generating random number from 1 to 100 inclusive
        // generating a couple times so player 2 will not always generate the same number 
        for(int i = 0; i < 5; i++) {target = rand() % (100 + 1 - 1) + 1;}
        
        while(1)
        {
            // reset signals
            p1signl = 0;
            p2signl = 0;
            
            while(!p1signl || !p2signl) // wait for player 1 and player 2 to make a guess
            {
                pause();
            }
            
            // reset signals
            p1signl = 0;
            p2signl = 0;

            // read player 1's guess
            fd = checkError(open("p1.dat", O_RDONLY, S_IRUSR), "opening");
            checkError(read(fd, &g1, sizeof(int)), "reading");
            close(fd); // close file

            //read player 2's guess 
            fd = checkError(open("p2.dat", O_RDONLY, S_IRUSR), "opening");
            checkError(read(fd, &g2, sizeof(int)), "reading");
            close(fd); // close file
            
            printf("%d %d %d\n", g1, g2, target); // outputting current players guesses and the target guess
            
            // indicate whether player 1's guess was higher, lower, or equal to the target
            if(g1 < target) {signl[0] = -1;}
            else if(g1 > target) {signl[0] = 1;} 
            else {signl[0] = 0;}

            // indicate whether player 2's guess was higher, lower, or equal to the target
            if(g2 < target){signl[1] = -1;}
            else if(g2 > target) {signl[1] = 1;}
            else {signl[1] = 0;}

            if(!signl[0] && !signl[1]) // if both values are 0 both players guessed correctly at the same time resulting in a tie
            {
                tiecount++; 
                printf("It is a tie!\n"); 
                break;
            }
            else if(!signl[0]) // if only the value at player 1's index == 0 player 1 wins
            {
                p1count++; 
                printf("Player 1 won!\n"); 
                break;
            }
            else if(!signl[1]) // if only the value at player 2's index == 0 player 2 wins
            { 
                p2count++; 
                printf("Player 2 won!\n"); 
                break;
            }

            if(signl[0] == -1) // player 1's guess was too low so we send the low signal
            {
                printf("Player 1 guessed low\n"); 
                kill(chld1, SIGUSR1);
            }
            else if(signl[0] == 1) // player 1's guess was too high so we send the high signal
            {
                printf("Player 1 guessed high\n"); 
                kill(chld1, SIGUSR2);
            }


            if(signl[1] == -1) // player 2's guess was too low so we send the low signal
            {
                printf("Player 2 guessed low\n"); 
                kill(chld2, SIGUSR1);
            }
            else if(signl[1] == 1) // player 2's guess was too high so we send the high signal
            {
                printf("Player 2 guessed high\n"); 
                kill(chld2, SIGUSR2);
            }            

        }
        
        // signal player 1 and player 2 to restart
        kill(chld1, SIGINT);
        kill(chld2, SIGINT);
    }

    // output how many times each player won and how many ties there were
    printf("\nNumber of ties: %d", tiecount);
    
    printf("\nPlayer 1 won %d", p1count);
    printf(" times!\n");
    
    printf("Player 2 won %d", p2count);
    printf(" times!\n");

    // output the overall winner of the guessing game
    if(p1count > p2count) {printf("\nPlayer 1 is the winner!\n");}
    else if (p1count < p2count){printf("\nPlayer 2 is the winner!\n");}
    else {printf("\nThe game resulted in a tie!\n");}

    kill(getpid(), SIGINT); // exit out of parent
}