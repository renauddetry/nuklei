// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <boost/algorithm/string.hpp>

#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_linalg.h>

#include <nuklei/LinearAlgebra.h>

namespace nuklei
{
  
  namespace la
  {
    void eigenDecomposition(Matrix3 &eVectors, Vector3& eValues, const Matrix3& sym)
    {
       coord_t data[9];
       for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
        data[i*3+j] = sym(i,j);
     
       gsl_matrix_view m 
         = gsl_matrix_view_array (data, 3, 3);
     
       gsl_vector *eval = gsl_vector_alloc (3);
       gsl_matrix *evec = gsl_matrix_alloc (3, 3);
     
       gsl_eigen_symmv_workspace * w = 
         gsl_eigen_symmv_alloc (3);
       
       gsl_eigen_symmv (&m.matrix, eval, evec, w);
     
       gsl_eigen_symmv_free (w);
     
       gsl_eigen_symmv_sort (eval, evec, 
                             GSL_EIGEN_SORT_ABS_DESC);
       
       for (int i = 0; i < 3; i++)
       {
         eValues[i]
            = gsl_vector_get (eval, i);
         //NUKLEI_ASSERT(eValues[i] >= 0);
         gsl_vector_view evec_i 
            = gsl_matrix_column (evec, i);
         for (int j = 0; j < 3; ++j)
           eVectors(j, i) = gsl_vector_get(&evec_i.vector, j);
       }
       
       // There should be a more efficient way to do this...
       if (Vector3(eVectors.GetColumn(0)).Cross(eVectors.GetColumn(1)).Dot(eVectors.GetColumn(2)) < 0)
         eVectors.SetColumn(2, -Vector3(eVectors.GetColumn(2)));

       gsl_vector_free (eval);
       gsl_matrix_free (evec);
    }
        
    // The following function
    //     Vector3 project(const Plane3& plane, const Vector3& point)
    // is currently defined in KernelCollectionPCA.cpp
    
    
    double determinant(const GMatrix& m)
    {
      NUKLEI_FAST_ASSERT(m.GetRows() == m.GetColumns());
      const int dim = m.GetRows();
      
      gsl_matrix *lu;
      gsl_permutation *perm;
      int signum = 0;
      
      lu = gsl_matrix_alloc(dim, dim);
      perm = gsl_permutation_alloc(dim);
      
      for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j)
        {
          gsl_matrix_set(lu, i, j, m(i,j));        
        }
      
      gsl_linalg_LU_decomp(lu, perm, &signum);
      double det = gsl_linalg_LU_det(lu, signum);
      
      gsl_matrix_free(lu);
      gsl_permutation_free(perm);
      
      return det;
    }
    
    GMatrix inverse(const GMatrix& m)
    {
      NUKLEI_FAST_ASSERT(m.GetRows() == m.GetColumns());
      const int dim = m.GetRows();
      
      gsl_matrix *lu, *inverse;
      gsl_permutation *perm;
      int signum = 0;
      
      lu = gsl_matrix_alloc(dim, dim);
      inverse = gsl_matrix_alloc(dim, dim);
      perm = gsl_permutation_alloc(dim);
      
      for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j)
        {
          gsl_matrix_set(lu, i, j, m(i,j));        
        }
      
      gsl_linalg_LU_decomp(lu, perm, &signum);
      gsl_linalg_LU_invert(lu, perm, inverse);
      
      GMatrix inv(dim, dim);
      for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j)
        {
          inv(i,j) = gsl_matrix_get(inverse, i, j);        
        }
      
      gsl_matrix_free(lu);
      gsl_matrix_free(inverse);
      gsl_permutation_free(perm);
      
      return inv;
    }
  
  }
  
  std::ostream& operator<<(std::ostream &out, const GMatrix &m)
  {
    for (int i = 0; i < m.GetRows(); ++i)
    {
      for (int j = 0; j < m.GetColumns(); ++j)
        NUKLEI_ASSERT(out << m(i,j) << " ");
      NUKLEI_ASSERT(out << "\n");
    }
    out << std::flush;
    return out;
  }

  std::istream& operator>>(std::istream &in, GMatrix &m)
  {
    std::vector< std::vector<double> > rows;
    std::string line;
    int ntok = -1;
    while (std::getline(in, line))
    {
      cleanLine(line);
      std::vector<std::string> tokens;
      boost::split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
      if (tokens.front() == "")
        tokens.erase(tokens.begin());
      if (tokens.back() == "")
        tokens.pop_back();
      
      if (ntok == -1)
        ntok = tokens.size();
      else
        NUKLEI_ASSERT(ntok == tokens.size());
      
      if (tokens.size() == 0)
        break;
      
      rows.push_back(std::vector<double>());
      for (std::vector<std::string>::const_iterator i = tokens.begin();
           i != tokens.end(); ++i)
        rows.back().push_back(numify<double>(*i));
    }
    
    m.SetSize(rows.size(), ntok);
    for (int i = 0; i < m.GetRows(); ++i)
      for (int j = 0; j < m.GetColumns(); ++j)
        m(i,j) = rows.at(i).at(j);
    
    return in;
  }


}
