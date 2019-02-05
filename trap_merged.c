/* trap.c -- Parallel Trapezoidal Rule, first version
 *
 * Input: None.
 * Output:  Estimate of the integral from a to b of f(x)
 *    using the trapezoidal rule and n trapezoids.
 *
 * Algorithm:
 *    1.  Each process calculates "its" interval of
 *        integration.
 *    2.  Each process estimates the integral of f(x)
 *        over its interval using the trapezoidal rule.
 *    3a. Each process != 0 sends its integral to 0.
 *    3b. Process 0 sums the calculations received from
 *        the individual processes and prints the result.
 *
 * Notes:  
 *    1.  f(x), a, b, and n are all hardwired.
 *    2.  The number of processes (p) should evenly divide
 *        the number of trapezoids (n = 1024)
 *
 * See Chap. 4, pp. 56 & ff. in PPMPI.
 */
#include <stdio.h>
#include <math.h>

/* We'll be using MPI routines, definitions, etc. */
#include "mpi.h"

/* Define bool type */
typedef int bool;
enum { false, true };

main(int argc, char** argv) {
    int         my_rank;   /* My process rank           */
    int         p;         /* The number of processes   */
    float       a = 0.0;   /* Left endpoint             */
    float       b = 1.0;   /* Right endpoint            */
    int         n = 1024;  /* Number of trapezoids      */
    float       h;         /* Trapezoid base length     */
    float       local_a;   /* Left endpoint my process  */
    float       local_b;   /* Right endpoint my process */
    int         local_n;   /* Number of trapezoids for  */
                           /* my calculation            */
//mengxi
    int         residual;  /* to be described           */
//mengxi
    float       integral;  /* Integral over my interval */
    float       total;     /* Total integral            */
    int         source;    /* Process sending integral  */
    int         dest = 0;  /* All messages go to 0      */
    int         tag = 0;
    MPI_Status  status;

    /* Change to False for main runs, True will give more information. */
    bool verbose = false; 

    float Trap(float local_a, float local_b, int local_n,
              float h);    /* Calculate local integral  */

    /* Let the system do what it needs to start up MPI */
    MPI_Init(&argc, &argv);

    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /*Process command line arguments */
    if(my_rank==0) {
        if(argc>1)
        {
            if(verbose) printf("Command Line Arguments:\n");
            a = atof(argv[1]);
            b = atof(argv[2]);
            n = atoi(argv[3]);
        }
        else /* Otherwise we will use the standard arguments*/
        {
            if(verbose) printf("Default Arguments:\n");
        }

        if(verbose)
        {
            printf("a is %5.3f\n", a);
            printf("b is %5.3f\n", b);
            printf("n is %d\n", n);
        }
        if(n<=0) {
        printf("Error: n <= 0 or n is not a number.\n");
        MPI_Abort(MPI_COMM_WORLD,1); /* Here I prescribe error code 1 for inputs. */
        }
    }
   
    MPI_Bcast(&a,1,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&b,1,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
//mengxi

    h = (b-a)/n;    /* h is the same for all processes */
    local_n = n/p;  /* So is the number of trapezoids */

//mengxi
    residual =  n%p;
//mengxi

    /* Length of each process' interval of
     * integration = local_n*h.  So my interval
     * starts at: */
    local_a = a + my_rank*local_n*h;
//mengxi
    if (my_rank < residual) {
        local_n ++;
        local_a += my_rank*h;
    } else {
        local_a += residual*h;
    }
    local_b = local_a + local_n*h;
//mengxi
    integral = Trap(local_a, local_b, local_n, h);

//mengxi
    printf("Process %d: %d subintervals from %f to %f\n",\
           my_rank,local_n,local_a,local_b);
//mengxi

    /* Add up the integrals calculated by each process */
/*
    if (my_rank == 0) {
        total = integral;
        for (source = 1; source < p; source++) {
            MPI_Recv(&integral, 1, MPI_FLOAT, MPI_ANY_SOURCE, tag,
                MPI_COMM_WORLD, &status);
            total = total + integral;
        }
    } else {  
        MPI_Send(&integral, 1, MPI_FLOAT, dest,
            tag, MPI_COMM_WORLD);
    }
*/
    MPI_Reduce(&integral,&total,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
    /* Print the result */

     if (my_rank == 0) {
        printf("With n = %d trapezoids, our estimate of the integral\n", n);
        printf("from %f to %f = %f\n", a, b, total);
        /*Other things to print:
         * True Value
         * True Error
         * h^2
         * h
         * n - number of intervals
         * p - number of processes */

        printf("True Value:                 %f\n", (pow(b,3)-pow(a,3))/3.0);
    }
    /* Shut down MPI */
    MPI_Finalize();
} /*  main  */


float Trap(
          float  local_a   /* in */,
          float  local_b   /* in */,
          int    local_n   /* in */,
          float  h         /* in */) {

    float integral;   /* Store result in integral  */
    float x;
    int i;

    float f(float x); /* function we're integrating */

    integral = (f(local_a) + f(local_b))/2.0;
    x = local_a;
    for (i = 1; i <= local_n-1; i++) {
        x = x + h;
        integral = integral + f(x);
    }
    integral = integral*h;
    return integral;
} /*  Trap  */


float f(float x) {
    float return_val;
    /* Calculate f(x). */
    /* Store calculation in return_val. */
    return_val = x*x;
    return return_val;
} /* f */

