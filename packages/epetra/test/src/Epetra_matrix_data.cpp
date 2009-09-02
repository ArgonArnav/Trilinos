
//@HEADER
// ************************************************************************
//
//               Epetra: Linear Algebra Services Package
//                 Copyright (2001) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#include <Epetra_matrix_data.h>
#include <Epetra_Map.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_Util.h>

namespace epetra_test {

matrix_data::matrix_data(int num_rows,
                         int* rowlengths,
                         int blocksize)
 : numrows_(num_rows),
   numcols_(0),
   rows_(0),
   rowlengths_(0),
   blocksize_(blocksize),
   colindices_(0),
   coefs_(0)
{
  if (numrows_ > 0) {
    rows_ = new int[numrows_];
    rowlengths_ = new int[numrows_];
    colindices_ = new int*[numrows_];
    coefs_ = new double*[numrows_];
    int dim = blocksize*blocksize;
    for(int i=0; i<numrows_; ++i) {
      rows_[i] = i;
      rowlengths_[i] = rowlengths[i];
      colindices_[i] = new int[rowlengths_[i]];
      coefs_[i] = new double[rowlengths_[i]*dim];

      for(int j=0; j<rowlengths_[i]; ++j) {
        colindices_[i][j] = 0;
        for(int k=0; k<dim; ++k) coefs_[i][j*dim+k] = 0.0;
      }
    }
  }
}

matrix_data::matrix_data(int num_rows,
                         int num_cols,
                         int num_off_diagonals,
                         int blocksize)
 : numrows_(num_rows),
   numcols_(num_cols),
   rows_(0),
   rowlengths_(0),
   blocksize_(blocksize),
   colindices_(0),
   coefs_(0)
{
  if (numrows_ > 0) {
    rows_       = new int[numrows_];
    rowlengths_ = new int[numrows_];
    colindices_ = new int*[numrows_];
    coefs_      = new double*[numrows_];

    int max_row_length = 1+num_off_diagonals*2;

    for(int i=0; i<numrows_; ++i) {
      rows_[i] = i;
      if (i >= num_off_diagonals && numrows_-i > num_off_diagonals) {
        rowlengths_[i] = max_row_length;
      }
      else {
        if (i<num_off_diagonals) {
          rowlengths_[i] = 1+max_row_length/2+i;
        }
        else {
          rowlengths_[i] = 1+max_row_length/2+numrows_-i-1;
        }
      }
      colindices_[i] = new int[rowlengths_[i]];
      int dim = blocksize*blocksize;
      coefs_[i] = new double[rowlengths_[i]*dim];

      int first_col = i - max_row_length/2;
      if (first_col < 0) first_col = 0;

      for(int j=0; j<rowlengths_[i]; ++j) {
        colindices_[i][j] = first_col+j;
        for(int k=0; k<dim; ++k) {
          coefs_[i][j*dim+k] = 1.0;
        }
      }
    }
  }
}

static const int nodes_per_elem = 4;

void get_node_ids(int elem_id, int* node_ids)
{
  int first_node = 2*elem_id;
  for(int i=0; i<nodes_per_elem; ++i) node_ids[i] = first_node+i;
}

matrix_data::matrix_data(int num_quad_elements,
                         int num_dof_per_node,
			 bool make_numerically_nonsymmetric)
 : numrows_(0),
   numcols_(0),
   rows_(0),
   rowlengths_(0),
   blocksize_(num_dof_per_node),
   colindices_(0),
   coefs_(0)
{
  //Set up matrix-data representing a simple finite-element
  //mesh containing 2-D quad elements
  //
  //   *-----*-----*-----*
  //  0|    2|    4|    6|
  //   | 0   | 1   | ne-1|
  //   |     |     |     |
  //   *-----*-----*-----*
  //  1     3     5     7
  //
  //In the above drawing, 'ne' means num-elems. node-numbers are to the
  //lower-left of each node (*).

  numrows_ = num_quad_elements*2+2;

  if (numrows_ > 0) {
    rows_       = new int[numrows_];
    rowlengths_ = new int[numrows_];
    colindices_ = new int*[numrows_];
    coefs_      = new double*[numrows_];

    int i, j, k;
    for(i=0; i<numrows_; ++i) {
      rows_[i] = i;
      rowlengths_[i] = 0;
    }

    int* nodes = new int[nodes_per_elem];
    for(i=0; i<num_quad_elements; ++i) {
      get_node_ids(i, nodes);

      for(j=0; j<nodes_per_elem; ++j) {
        int node_j = nodes[j];
        for(k=0; k<nodes_per_elem; ++k) {
          int insertPoint = -1;
          int alloclen = rowlengths_[node_j];
          int offset = Epetra_Util_binary_search(nodes[k], colindices_[node_j],
                                                 rowlengths_[node_j], insertPoint);
          if (offset < 0) {
            Epetra_Util_insert(nodes[k], insertPoint,
                               colindices_[node_j], rowlengths_[node_j],
                               alloclen);
          }
        }
      }
    }

    int dim = blocksize_*blocksize_;
    for(i=0; i<numrows_; ++i) {
      int len = rowlengths_[i]*dim;
      coefs_[i] = new double[len];
      for(j=0; j<len; ++j) {
	if (make_numerically_nonsymmetric) {
	  coefs_[i][j] = 1.0*(j+1);
	}
	else {
	  coefs_[i][j] = 1.0;
	}
      }
    }
  }
}

matrix_data::~matrix_data()
{
  for(int i=0; i<numrows_; ++i) {
    delete [] colindices_[i];
    delete [] coefs_[i];
  }

  delete [] colindices_; colindices_ = 0;
  delete [] coefs_; coefs_ = 0;
  delete [] rowlengths_; rowlengths_ = 0;
  delete [] rows_; rows_ = 0;
  numrows_ = 0;
}

double* matrix_data::coefs(int row, int col)
{
  int insertPoint = -1;
  int row_idx = Epetra_Util_binary_search(row, rows_, numrows_,
                                          insertPoint);
  if (row_idx < 0) {
    cerr << "ERROR, row " << row
         << " not found in matrix_data"<<endl;
    return 0;
  }

  int col_idx = Epetra_Util_binary_search(col, colindices_[row_idx],
                                          rowlengths_[row_idx], insertPoint);
  if (col_idx < 0) {
    cerr << "ERROR, col " << col
         << " not found in matrix_data"<<endl;
    return 0;
  }

  int dim = blocksize_*blocksize_;
  return( &(coefs_[row_idx][col_idx*dim]) );
}

void matrix_data::copy_local_data_to_matrix(Epetra_CrsMatrix& A)
{
  const Epetra_Map& rowmap = A.RowMap();

  for(int i=0; i<numrows_; ++i) {
    int row = rows_[i];
    if (rowmap.MyGID(row)) {
      int err = A.ReplaceGlobalValues(row, rowlengths_[i],
				      coefs_[i], colindices_[i]);
      if (err < 0) {
	err = A.InsertGlobalValues(row, rowlengths_[i],
				   coefs_[i], colindices_[i]);
      }
    }
  }
}

bool matrix_data::compare_local_data(const Epetra_CrsMatrix& A)
{
  const Epetra_Map& map = A.RowMap();
  int numMyRows = map.NumMyElements();
  int* myRows = map.MyGlobalElements();

  Epetra_Util util;

  for(int i=0; i<numMyRows; ++i) {
    int row = myRows[i];
    int rowLen = A.NumGlobalEntries(row);
    if (rowLen != rowlengths_[row]) {
      return(false);
    }

    int* indices = new int[rowLen];
    double* values = new double[rowLen];
    A.ExtractGlobalRowCopy(row, rowLen, rowLen, values, indices);

    util.Sort(true, rowLen, indices, 1, &values, 0, 0);

    bool same = true;
    int* this_indices = colindices_[row];
    double* this_coefs = coefs_[row];

    for(int j=0; j<rowLen; ++j) {
      if (indices[j] != this_indices[j]) {
        same = false; break;
      }
      if (values[j] != this_coefs[j]) {
        same = false; break;
      }
    }

    delete [] indices;
    delete [] values;

    if (!same) return(false);
  }

  return(true);
}

}//namespace epetra_test

