/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    $Author$
 *    $Date$
 *    $Revision$
 ****************************************************************************/
/****************************************************************************/
/* FILE  ******************   MPI_Keyval_create.c    ************************/
/****************************************************************************/
/* Author : Lisa Alano July 23 2002                                         */
/* Copyright (c) 2002 University of California Regents                      */
/****************************************************************************/

#include "mpi.h"

int MPI_Keyval_create ( 
        MPI_Copy_function *copy_fn, 
        MPI_Delete_function *delete_fn, 
        int *keyval, 
        void *extra_state )
{
  _MPI_COVERAGE();
  return PMPI_Keyval_create (copy_fn, delete_fn, keyval, extra_state);
}

