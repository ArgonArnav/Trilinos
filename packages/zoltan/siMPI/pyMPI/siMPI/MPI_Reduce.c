/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    $Author$
 *    $Date$
 *    $Revision$
 ****************************************************************************/
/* FILE  ******************       MPI_Reduce.c       ************************/
/****************************************************************************/
/* Author : Lisa Alano July 1 2002                                          */
/* Copyright (c) 2002 University of California Regents                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

/*==========================================================================*/
int MPI_Reduce ( void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm )
{
  _MPI_COVERAGE();
  return PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}

/*==========================================================================*/
